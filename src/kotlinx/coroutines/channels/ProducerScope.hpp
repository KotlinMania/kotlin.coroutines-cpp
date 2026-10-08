#pragma once
// port-lint: source channels/Produce.kt
/** Transliterated from: kotlinx-coroutines-core/common/src/channels/Produce.kt */
#include "kotlinx/coroutines/channels/Channel.hpp"
#include "kotlinx/coroutines/CoroutineScope.hpp"

namespace kotlinx {
namespace coroutines {
namespace channels {

// Transliterated from: kotlinx-coroutines-core/common/src/channels/Produce.kt:10-19
template <typename E>
struct ProducerScope : public virtual CoroutineScope, public virtual SendChannel<E> {
    virtual ~ProducerScope() = default;

    // A reference to the channel this coroutine sends elements to.
    // All SendChannel functions delegate to this channel.
    // Transliterated from: kotlinx-coroutines-core/common/src/channels/Produce.kt:11-18
    virtual SendChannel<E>* get_channel() = 0;
};

} // namespace channels
} // namespace coroutines
} // namespace kotlinx
