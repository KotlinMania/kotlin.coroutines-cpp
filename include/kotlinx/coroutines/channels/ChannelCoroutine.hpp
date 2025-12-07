#pragma once
#include "kotlinx/coroutines/core_fwd.hpp"
#include "kotlinx/coroutines/AbstractCoroutine.hpp"
#include "kotlinx/coroutines/channels/Channel.hpp"

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
    ) : AbstractCoroutine<Unit>(*parent_context, init_parent_job, active), _channel(channel) {}

    virtual ~ChannelCoroutine() = default;

    // Channel delegation
    bool is_closed_for_send() const override { return _channel->is_closed_for_send(); }
    void send(E element) override { _channel->send(element); }
    ChannelResult<void> try_send(E element) override { return _channel->try_send(element); }
    bool close(std::exception_ptr cause = nullptr) override { return _channel->close(cause); }
    void invoke_on_close(std::function<void(std::exception_ptr)> handler) override { _channel->invoke_on_close(handler); }

    bool is_closed_for_receive() const override { return _channel->is_closed_for_receive(); }
    bool is_empty() const override { return _channel->is_empty(); }
    E receive() override { return _channel->receive(); }
    ChannelResult<E> receive_catching() override { return _channel->receive_catching(); }
    ChannelResult<E> try_receive() override { return _channel->try_receive(); }
    std::shared_ptr<ChannelIterator<E>> iterator() override { return _channel->iterator(); }
    void cancel(std::exception_ptr cause = nullptr) override { 
        _channel->cancel(cause);
        AbstractCoroutine<Unit>::cancel(cause);
    }

    std::shared_ptr<Channel<E>> get_channel() { return _channel; }
};

} // namespace channels
} // namespace coroutines
} // namespace kotlinx
