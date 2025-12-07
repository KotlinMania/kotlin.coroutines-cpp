#pragma once
#include "kotlinx/coroutines/core_fwd.hpp"
#include "kotlinx/coroutines/flow/FlowCollector.hpp"
#include "kotlinx/coroutines/channels/Channel.hpp"

namespace kotlinx {
namespace coroutines {
namespace flow {
namespace internal {

// Collection that sends values to a channel
template <typename T>
class SendingCollector : public FlowCollector<T> {
public:
    explicit SendingCollector(SendChannel<T>* channel) : channel_(channel) {}

    void emit(T value) override {
        // TODO: implement channel_.send(value) once channel semantics are fully ported
        // channel_->send(value);
    }

private:
    SendChannel<T>* channel_;
};

} // namespace internal
} // namespace flow
} // namespace coroutines
} // namespace kotlinx
