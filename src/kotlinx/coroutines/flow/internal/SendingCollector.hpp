/**
 * Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/SendingCollector.kt
 */
#pragma once
// port-lint: source flow/internal/SendingCollector.kt
#include "kotlinx/coroutines/flow/FlowCollector.hpp"
#include "kotlinx/coroutines/channels/Channel.hpp"

namespace kotlinx {
namespace coroutines {
namespace flow {
namespace internal {

// Collection that sends to channel.
// This is an internal API and should not be used from general code.
// Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/SendingCollector.kt:7-16
template <typename T>
class SendingCollector : public FlowCollector<T> {
public:
    // Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/SendingCollector.kt:12-14
    explicit SendingCollector(channels::SendChannel<T>* channel) : channel_(channel) {}

    // Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/SendingCollector.kt:15-15
    void* emit(T value, Continuation<void*>* continuation) override {
        return channel_->send(std::move(value), continuation);
    }

private:
    channels::SendChannel<T>* channel_;
};

} // namespace internal
} // namespace flow
} // namespace coroutines
} // namespace kotlinx
