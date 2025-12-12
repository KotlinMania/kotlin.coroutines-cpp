#pragma once
#include "kotlinx/coroutines/flow/FlowCollector.hpp"
#include "kotlinx/coroutines/channels/Channel.hpp"

namespace kotlinx {
namespace coroutines {
namespace flow {
namespace internal {

// Collection that sends values to a channel
// Corresponds to kotlinx.coroutines.flow.internal.SendingCollector
template <typename T>
class SendingCollector : public FlowCollector<T> {
public:
    explicit SendingCollector(channels::SendChannel<T>* channel) : channel_(channel) {}

    void emit(T value) override {
        if (channel_->is_closed_for_send()) {
             throw channels::ClosedSendChannelException("Channel was closed");
        }
        channel_->send(value);
    }

private:
    channels::SendChannel<T>* channel_;
};

} // namespace internal
} // namespace flow
} // namespace coroutines
} // namespace kotlinx
