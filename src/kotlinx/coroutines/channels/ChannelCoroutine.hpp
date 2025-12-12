#pragma once
// Kotlin source: kotlinx-coroutines-core/common/src/channels/ChannelCoroutine.kt
#include "kotlinx/coroutines/Unit.hpp"
#include "kotlinx/coroutines/AbstractCoroutine.hpp"
#include "kotlinx/coroutines/channels/Channel.hpp"
#include "kotlinx/coroutines/CoroutineContext.hpp"
#include "kotlinx/coroutines/Exceptions.hpp"

namespace kotlinx {
namespace coroutines {
namespace channels {

template <typename E>
class ChannelCoroutine : public AbstractCoroutine<Unit>, public Channel<E> {
protected:
    std::shared_ptr<Channel<E>> _channel;

public:
    ChannelCoroutine(
        std::shared_ptr<CoroutineContext> parent_context,
        std::shared_ptr<Channel<E>> channel,
        bool init_parent_job,
        bool active
    ) : AbstractCoroutine<Unit>(parent_context, init_parent_job, active), _channel(channel) {}

    virtual ~ChannelCoroutine() = default;

    // SendChannel delegation
    bool is_closed_for_send() const override { return _channel->is_closed_for_send(); }
    
    ChannelAwaiter<void> send(E element) override { 
        // Delegate suspend function
        return _channel->send(std::move(element)); 
    }
    
    ChannelResult<void> try_send(E element) override { return _channel->try_send(std::move(element)); }
    
    bool close(std::exception_ptr cause = nullptr) override { return _channel->close(cause); }
    
    void invoke_on_close(std::function<void(std::exception_ptr)> handler) override { 
        _channel->invoke_on_close(handler); 
    }

    // ReceiveChannel delegation
    bool is_closed_for_receive() const override { return _channel->is_closed_for_receive(); }
    bool is_empty() const override { return _channel->is_empty(); }
    
    ChannelAwaiter<E> receive() override { 
        return _channel->receive(); 
    }
    
    ChannelAwaiter<ChannelResult<E>> receive_catching() override { 
        return _channel->receive_catching(); 
    }
    
    ChannelResult<E> try_receive() override { return _channel->try_receive(); }
    
    ChannelAwaiter<ChannelResult<E>> receive_with_timeout(long timeout_millis) override { 
        return _channel->receive_with_timeout(timeout_millis); 
    }

    std::shared_ptr<selects::SelectClause2<E, SendChannel<E>>> on_send() override {
        return _channel->on_send();
    }

    std::shared_ptr<selects::SelectClause1<E>> on_receive() override {
        return _channel->on_receive();
    }
    
    std::shared_ptr<ChannelIterator<E>> iterator() override { return _channel->iterator(); }

    // Cancellation logic matching ChannelCoroutine.kt
    void cancel(std::exception_ptr cause = nullptr) override { 
        // cancelInternal logic:
        // 1. Cancel the channel
        _channel->cancel(cause);
        // 2. Cancel the coroutine (Job)
        AbstractCoroutine<Unit>::cancel(cause);
    }

    std::shared_ptr<Channel<E>> get_channel() { return _channel; }
};

} // namespace channels
} // namespace coroutines
} // namespace kotlinx
