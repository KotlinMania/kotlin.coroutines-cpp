#include "kotlinx/coroutines/Continuation.hpp"
#include "kotlinx/coroutines/context_impl.hpp"
#include "kotlinx/coroutines/channels/Channels.hpp"
#include "kotlinx/coroutines/flow/ChannelAsFlow.hpp"
#include "kotlinx/coroutines/flow/FlowCollector.hpp"
#include <algorithm>
#include <memory>
#include <vector>

namespace {

class NoopContinuation : public kotlinx::coroutines::Continuation<void*> {
public:
    NoopContinuation() : ctx_(kotlinx::coroutines::EmptyCoroutineContext::instance()) {}

    std::shared_ptr<kotlinx::coroutines::CoroutineContext> get_context() const override { return ctx_; }
    void resume_with(kotlinx::coroutines::Result<void*> /*result*/) override {}

private:
    std::shared_ptr<kotlinx::coroutines::CoroutineContext> ctx_;
};

template <typename T>
class VectorCollector : public kotlinx::coroutines::flow::FlowCollector<T> {
public:
    explicit VectorCollector(std::vector<T>* out) : out_(out) {}

    void* emit(T value, kotlinx::coroutines::Continuation<void*>* /*continuation*/) override {
        out_->push_back(std::move(value));
        return nullptr;
    }

private:
    std::vector<T>* out_;
};

} // namespace

int main() {
    using kotlinx::coroutines::channels::Channel;
    using kotlinx::coroutines::channels::create_channel;
    using kotlinx::coroutines::flow::consume_as_flow;
    using kotlinx::coroutines::flow::receive_as_flow;

    // Build a channel with some buffered values.
    auto ch = create_channel<int>(Channel<int>::BUFFERED);
    if (!ch->try_send(1).is_success()) return 1;
    if (!ch->try_send(2).is_success()) return 1;
    ch->close(nullptr);

    // Channel<E> inherits from ReceiveChannel<E>
    std::shared_ptr<kotlinx::coroutines::channels::ReceiveChannel<int>> recv = ch;
    if (!recv) return 1;

    // receive_as_flow: multiple collections are allowed (channel fan-out).
    {
        auto f = receive_as_flow<int>(recv);
        std::vector<int> out;
        VectorCollector<int> collector(&out);
        NoopContinuation cont;
        void* r = f->collect(&collector, &cont);
        if (r != nullptr) return 1;
        std::sort(out.begin(), out.end());
        if (out != std::vector<int>{1, 2}) return 1;
    }

    // consume_as_flow: only a single collection is allowed.
    {
        auto f = consume_as_flow<int>(recv);
        std::vector<int> out;
        VectorCollector<int> collector(&out);
        NoopContinuation cont;
        (void)f->collect(&collector, &cont);

        bool threw = false;
        try {
            std::vector<int> out2;
            VectorCollector<int> collector2(&out2);
            (void)f->collect(&collector2, &cont);
        } catch (const std::logic_error&) {
            threw = true;
        }
        if (!threw) return 1;
    }

    return 0;
}
