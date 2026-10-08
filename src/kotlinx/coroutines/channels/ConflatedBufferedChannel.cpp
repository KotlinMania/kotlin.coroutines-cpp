// port-lint: source channels/ConflatedBufferedChannel.kt
/**
 * Transliterated from: kotlinx-coroutines-core/common/src/channels/ConflatedBufferedChannel.kt
 * Generic source algorithms live in the co-located header; this file instantiates common element types.
 */

#include "kotlinx/coroutines/channels/ConflatedBufferedChannel.hpp"

namespace kotlinx {
    namespace coroutines {
        namespace channels {
            // Template implementation is in the header.

            // Explicit instantiation for common types to ensure compilation validity and linkage.
            template class ConflatedBufferedChannel<int>;
            template class ConflatedBufferedChannel<std::string>;
        } // namespace channels
    } // namespace coroutines
} // namespace kotlinx