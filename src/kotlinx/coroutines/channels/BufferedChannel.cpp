// port-lint: source channels/BufferedChannel.kt
/**
 * @file BufferedChannel.cpp
 * @brief Implementation of BufferedChannel.
 *
 * Transliterated from: kotlinx-coroutines-core/common/src/channels/BufferedChannel.kt
 *
 * Generic algorithms remain in the header. This file instantiates common element types.
 */

#include "kotlinx/coroutines/channels/BufferedChannel.hpp"

namespace kotlinx {
    namespace coroutines {
        namespace channels {
            // Explicit instantiations for common types
            template class BufferedChannel<int>;
            template class BufferedChannel<std::string>;
        } // namespace channels
    } // namespace coroutines
} // namespace kotlinx
