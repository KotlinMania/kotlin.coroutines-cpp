#pragma once
#include "kotlinx/coroutines/core_fwd.hpp"
#include "kotlinx/coroutines/channels/Channel.hpp"
#include "kotlinx/coroutines/channels/ProducerScope.hpp"
#include "kotlinx/coroutines/channels/ChannelCoroutine.hpp"
#include "kotlinx/coroutines/CoroutineStart.hpp"
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
        ChannelCoroutine<E>::_channel->close(nullptr);
    }
    
    void on_cancelled(std::exception_ptr cause, bool handled) override {
        // Filter out CancellationException specifically if needed
        ChannelCoroutine<E>::_channel->close(cause);
    }
    
    // ProducerScope overrides (delegating to ChannelCoroutine -> Channel)
    bool is_closed_for_send() const override { return ChannelCoroutine<E>::_channel->is_closed_for_send(); }
    void send(E element) override { ChannelCoroutine<E>::_channel->send(element); }
    ChannelResult<void> try_send(E element) override { return ChannelCoroutine<E>::_channel->try_send(element); }
    bool close(std::exception_ptr cause = nullptr) override { return ChannelCoroutine<E>::_channel->close(cause); }
    void invoke_on_close(std::function<void(std::exception_ptr)> handler) override { ChannelCoroutine<E>::_channel->invoke_on_close(handler); }
    
    SendChannel<E>* get_channel() override { return ChannelCoroutine<E>::_channel.get(); }
};

template <typename E>
std::shared_ptr<ReceiveChannel<E>> produce(
    CoroutineScope* scope,
    std::shared_ptr<CoroutineContext> context,
    int capacity,
    CoroutineStart start,
    std::function<void(ProducerScope<E>*)> block // suspend block
) {
    auto newContext = scope->get_coroutine_context().plus(context);
    auto channel = Channel<E>::create(capacity);
    auto coroutine = std::make_shared<ProducerCoroutine<E>>(std::make_shared<CoroutineContext>(newContext), channel);
    coroutine->start(start, coroutine, block);
    return coroutine;
}

} // namespace channels
} // namespace coroutines
} // namespace kotlinx
