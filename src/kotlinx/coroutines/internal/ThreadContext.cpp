/**
 * Transliterated from: kotlinx-coroutines-core/native/src/internal/ThreadContext.kt
 *
 * Kotlin file header (translated):
 *   package kotlinx.coroutines.internal
 */

#include "kotlinx/coroutines/CoroutineContext.hpp"

#include <cstdint>

namespace kotlinx::coroutines::internal {

/**
 * Upstream:
 *   internal actual fun threadContextElements(context: CoroutineContext): Any = 0
 *
 * Native always returns a constant 0 sentinel — Kotlin/Native does not maintain a counted
 * thread-context element list the way the JVM target does.
 */
std::intptr_t thread_context_elements(const CoroutineContext& /*context*/) {
    return 0;
}

} // namespace kotlinx::coroutines::internal
