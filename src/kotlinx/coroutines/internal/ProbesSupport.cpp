/**
 * Transliterated from: kotlinx-coroutines-core/native/src/internal/ProbesSupport.kt
 *
 * Kotlin file header (translated):
 *   package kotlinx.coroutines.internal
 *
 * Upstream is two no-op actual declarations — Kotlin/Native has no debug-probe
 * machinery, the JVM hooks are stubbed away.
 *
 *   internal actual inline fun <T> probeCoroutineCreated(completion: Continuation<T>): Continuation<T> = completion
 *   internal actual inline fun <T> probeCoroutineResumed(completion: Continuation<T>) { }
 *
 * Both end up inlined and the symbols don't normally need a definition; this file is the
 * inventory companion only so the build system has the matching translation unit.
 */

#include "kotlinx/coroutines/Continuation.hpp"
