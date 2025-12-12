/**
 * @file ProbesSupport.cpp
 * @brief Native platform implementation of probes support
 *
 * Transliterated from: kotlinx-coroutines-core/native/src/internal/ProbesSupport.kt
 *
 * Platform-specific (native) implementation of coroutine probes for debugging.
 * These are typically no-ops on native platforms (probes are mainly for JVM debugging).
 *
 * TODO:
 * - Implement probeCoroutineCreated hook
 * - Implement probeCoroutineResumed hook
 */

#include "kotlinx/coroutines/core_fwd.hpp"

namespace kotlinx {
    namespace coroutines {
        namespace internal {
            // On native platforms, probe functions are typically no-ops
            // Probe support is mainly used for JVM debugging tooling
        } // namespace internal
    } // namespace coroutines
} // namespace kotlinx