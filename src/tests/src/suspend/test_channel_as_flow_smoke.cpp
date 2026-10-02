#include "kotlinx/coroutines/Continuation.hpp"
#include "kotlinx/coroutines/context_impl.hpp"
#include "kotlinx/coroutines/channels/Channels.hpp"
#include "kotlinx/coroutines/flow/Channels.hpp"
#include "kotlinx/coroutines/flow/FlowCollector.hpp"
#include <algorithm>
#include <memory>
#include <vector>

namespace {

class NoopContinuation : public kotlinx::coroutines::Continuation<void*> {
public:
    NoopContinuation() : ctx_(kotlinx::coroutines::EmptyCoroutineContext::instance()) {}

    std::shared_ptr<kotlinx::coroutines::CoroutineContext> get_context() const override { return ctx_; }
    void resume_with(kotlinx::coroutines::Result<void*> result) override {
        // No-op continuation - result indicates completion status
        // Could check result.is_success() to verify expected behavior
    }

private:
    std::shared_ptr<kotlinx::coroutines::CoroutineContext> ctx_;
};

template <typename T>
class VectorCollector : public kotlinx::coroutines::flow::FlowCollector<T> {
public:
    explicit VectorCollector(std::vector<T>* out) : out_(out) {}

    void* emit(T value, kotlinx::coroutines::Continuation<void*>* continuation) override {
        (void)continuation;
        out_->push_back(std::move(value));
        return nullptr;
    }

private:
    std::vector<T>* out_;
};

class RecordingContinuation : public kotlinx::coroutines::Continuation<void*> {
public:
    bool completed = false;
    std::shared_ptr<kotlinx::coroutines::CoroutineContext> ctx_ = kotlinx::coroutines::EmptyCoroutineContext::instance();

    std::shared_ptr<kotlinx::coroutines::CoroutineContext> get_context() const override { return ctx_; }
    void resume_with(kotlinx::coroutines::Result<void*> result) override {
        (void)result;
        completed = true;
    }
};

class MockSuspendingIterator : public kotlinx::coroutines::channels::ChannelIterator<int> {
public:
    int step = 0;
    kotlinx::coroutines::Continuation<void*>* saved_cont = nullptr;

    void* has_next(kotlinx::coroutines::Continuation<void*>* cont) override {
        if (step == 0) {
            step = 1;
            saved_cont = cont;
            return kotlinx::coroutines::intrinsics::get_COROUTINE_SUSPENDED();
        }
        return new bool(false);
    }

    int next() override {
        return 42;
    }
};

class MockSuspendingChannel : public kotlinx::coroutines::channels::ReceiveChannel<int> {
public:
    MockSuspendingIterator* it_ptr = nullptr;
    bool cancelled = false;

    std::unique_ptr<kotlinx::coroutines::channels::ChannelIterator<int>> iterator() override {
        auto it = std::make_unique<MockSuspendingIterator>();
        it_ptr = it.get();
        return it;
    }

    void cancel(std::exception_ptr cause) override {
        (void)cause;
        cancelled = true;
    }

    bool is_closed_for_receive() const override { return cancelled; }
    bool is_empty() const override { return false; }
    kotlinx::coroutines::channels::ChannelResult<int> try_receive() override {
        return kotlinx::coroutines::channels::ChannelResult<int>::failure();
    }
    void* receive(kotlinx::coroutines::Continuation<void*>* cont) override { (void)cont; return nullptr; }
    void* receive_catching(kotlinx::coroutines::Continuation<void*>* cont) override { (void)cont; return nullptr; }
    kotlinx::coroutines::selects::SelectClause1<int>& on_receive() override { throw std::logic_error("not implemented"); }
    kotlinx::coroutines::selects::SelectClause1<kotlinx::coroutines::channels::ChannelResult<int>>& on_receive_catching() override {
        throw std::logic_error("not implemented");
    }
};

class SuspendingCollector : public kotlinx::coroutines::flow::FlowCollector<int> {
public:
    std::vector<int>* out_;
    kotlinx::coroutines::Continuation<void*>* saved_emit_cont = nullptr;

    explicit SuspendingCollector(std::vector<int>* out) : out_(out) {}

    void* emit(int value, kotlinx::coroutines::Continuation<void*>* continuation) override {
        out_->push_back(value);
        saved_emit_cont = continuation;
        return kotlinx::coroutines::intrinsics::get_COROUTINE_SUSPENDED();
    }
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

    // emit_all: normal completion and channel consumption
    {
        auto ch1 = create_channel<int>(Channel<int>::BUFFERED);
        ch1->try_send(10);
        ch1->try_send(20);
        ch1->close(nullptr);

        std::vector<int> out;
        VectorCollector<int> collector(&out);
        NoopContinuation cont;
        void* r = kotlinx::coroutines::flow::emit_all(&collector, ch1.get(), &cont);
        if (r != nullptr) return 1;
        if (out != std::vector<int>{10, 20}) return 1;
        if (!ch1->is_closed_for_receive()) return 1;
    }

    // emit_all: exception during emit cancels channel and rethrows
    {
        auto ch2 = create_channel<int>(Channel<int>::BUFFERED);
        ch2->try_send(100);
        ch2->try_send(200);

        class ThrowingCollector : public kotlinx::coroutines::flow::FlowCollector<int> {
        public:
            void* emit(int value, kotlinx::coroutines::Continuation<void*>* continuation) override {
                (void)value;
                (void)continuation;
                throw std::runtime_error("emission failure");
            }
        };

        ThrowingCollector collector;
        NoopContinuation cont;
        bool caught = false;
        try {
            kotlinx::coroutines::flow::emit_all(&collector, ch2.get(), &cont);
        } catch (const std::runtime_error&) {
            caught = true;
        }
        if (!caught) return 1;
        if (!ch2->is_closed_for_receive()) return 1;
    }

    // emit_all: async suspension and resumption at has_next
    {
        MockSuspendingChannel mock_ch;
        std::vector<int> out;
        VectorCollector<int> collector(&out);
        RecordingContinuation completion;

        void* r = kotlinx::coroutines::flow::emit_all(&collector, &mock_ch, &completion);
        if (r != kotlinx::coroutines::intrinsics::get_COROUTINE_SUSPENDED()) return 1;
        if (completion.completed) return 1;
        if (mock_ch.it_ptr == nullptr || mock_ch.it_ptr->saved_cont == nullptr) return 1;

        // Resume with true -> should emit 42 and complete
        mock_ch.it_ptr->saved_cont->resume_with(
            kotlinx::coroutines::Result<void*>::success(new bool(true)));

        if (!completion.completed) return 1;
        if (out != std::vector<int>{42}) return 1;
        if (!mock_ch.cancelled) return 1;
    }

    // emit_all: async suspension and resumption at collector emit
    {
        auto ch3 = create_channel<int>(Channel<int>::BUFFERED);
        ch3->try_send(999);
        ch3->close(nullptr);

        std::vector<int> out;
        SuspendingCollector collector(&out);
        RecordingContinuation completion;

        void* r = kotlinx::coroutines::flow::emit_all(&collector, ch3.get(), &completion);
        if (r != kotlinx::coroutines::intrinsics::get_COROUTINE_SUSPENDED()) return 1;
        if (completion.completed) return 1;
        if (collector.saved_emit_cont == nullptr) return 1;
        if (out != std::vector<int>{999}) return 1;

        // Resume emit continuation
        collector.saved_emit_cont->resume_with(kotlinx::coroutines::Result<void*>::success(nullptr));

        if (!completion.completed) return 1;
    }

    return 0;
}
