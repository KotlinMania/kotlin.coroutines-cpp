#pragma once
#include "kotlinx/coroutines/channels/Channel.hpp"
#include "kotlinx/coroutines/channels/Channels.hpp"
#include "kotlinx/coroutines/channels/ProducerScope.hpp"
#include "kotlinx/coroutines/channels/ChannelCoroutine.hpp"
#include "kotlinx/coroutines/CoroutineStart.hpp"
#include "kotlinx/coroutines/channels/BufferOverflow.hpp"
#include "kotlinx/coroutines/context_impl.hpp"
#include <functional>
#include <memory>

namespace kotlinx {
namespace coroutines {
namespace channels {

template <typename E>
class ProducerCoroutine : public ChannelCoroutine<E>, public ProducerScope<E> {
public:
    ProducerCoroutine(
        std::shared_ptr<CoroutineContext> parentContext,
        std::shared_ptr<Channel<E>> channel
    ) : ChannelCoroutine<E>(parentContext, channel, true, true) {}

    virtual ~ProducerCoroutine() = default;

    bool is_active() const override { return ChannelCoroutine<E>::is_active(); }

    // AbstractCoroutine overrides
    void on_completed(Unit value) override {
        (void)value;
        ChannelCoroutine<E>::_channel->close(nullptr);
    }

    void on_cancelled(std::exception_ptr cause, bool handled) override {
        bool processed = ChannelCoroutine<E>::_channel->close(cause);
        if (!processed && !handled) {
            // handleCoroutineException(context, cause); // TODO: Implement exception handling
        }
    }

    // ProducerScope overrides (delegating to ChannelCoroutine -> Channel)
    bool is_closed_for_send() const override { return ChannelCoroutine<E>::is_closed_for_send(); }

    void* send(E element, Continuation<void*>* continuation) override {
        return ChannelCoroutine<E>::send(std::move(element), continuation);
    }

    ChannelResult<void> try_send(E element) override { return ChannelCoroutine<E>::try_send(std::move(element)); }

    bool close(std::exception_ptr cause = nullptr) override { return ChannelCoroutine<E>::close(cause); }

    void invoke_on_close(std::function<void(std::exception_ptr)> handler) override {
        ChannelCoroutine<E>::invoke_on_close(handler);
    }

    SendChannel<E>* get_channel() override { return ChannelCoroutine<E>::_channel.get(); }
};

template <typename E>
std::shared_ptr<ReceiveChannel<E>> produce(
    CoroutineScope* scope,
    std::shared_ptr<CoroutineContext> context, // defaulted in Kotlin
    int capacity = 0,
    BufferOverflow on_buffer_overflow = BufferOverflow::SUSPEND,
    CoroutineStart start = CoroutineStart::DEFAULT,
    std::function<void(ProducerScope<E>*)> block = nullptr // should be suspend
) {
    // 1. Create Channel
    auto channel = create_channel<E>(capacity, on_buffer_overflow);

    // 2. Create Context
    // Kotlin: val newContext = scope.newCoroutineContext(context)
    if (!context) context = EmptyCoroutineContext::instance();
    auto newContext = scope->get_coroutine_context()->operator+(context);

    // 3. Create Coroutine
    auto coroutine = std::make_shared<ProducerCoroutine<E>>(newContext, channel);

    // 4. Start
    if (block) {
        // AbstractCoroutine::start expects an explicit std::function<T(R)> (not a generic lambda).
        std::function<Unit(std::shared_ptr<ProducerCoroutine<E>>)> wrapped_block =
            [block](std::shared_ptr<ProducerCoroutine<E>> receiver) {
                block(receiver.get());
                return Unit();
            };
        coroutine->start(start, coroutine, std::move(wrapped_block));
    }

    return coroutine;
}

} // namespace channels
} // namespace coroutines
} // namespace kotlinx
