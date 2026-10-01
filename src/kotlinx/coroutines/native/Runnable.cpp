// port-lint: source Runnable.common.kt
/**
 * Transliterated from: kotlinx-coroutines-core/native/src/Runnable.kt
 *
 * Kotlin file header (translated):
 *   package kotlinx.coroutines
 *
 * Upstream:
 *   public actual fun interface Runnable { fun run() }
 *   @Deprecated(...) public inline fun Runnable(crossinline block: () -> Unit): Runnable =
 *       object : Runnable { override fun run() { block() } }
 */

#include "kotlinx/coroutines/Runnable.hpp"

namespace kotlinx::coroutines {
    // Runnable interface and make_runnable factory are defined in Runnable.hpp
} // namespace kotlinx::coroutines
