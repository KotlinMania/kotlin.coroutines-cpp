// port-lint: source kotlinx-coroutines-core/common/src/channels/Channels.common.kt
/**
 * Transliterated from: kotlinx-coroutines-core/common/src/channels/Channels.common.kt
 * Public element-type templates are instantiated in Channels.hpp.
 */
#include "kotlinx/coroutines/channels/Channels.hpp"

namespace kotlinx::coroutines::channels {

// Transliterated from: kotlinx-coroutines-core/common/src/channels/Channels.common.kt:198-202
std::exception_ptr internal::consumed_cancellation_cause(std::exception_ptr cause) {
    if (!cause || is_cancellation_exception(cause)) return cause;
    return std::make_exception_ptr(CancellationException(
        "Channel was consumed, consumer had failed", cause));
}


} // namespace kotlinx::coroutines::channels
