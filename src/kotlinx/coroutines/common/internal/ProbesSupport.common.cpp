/**
 * Transliterated from: kotlinx-coroutines-core/common/src/internal/ProbesSupport.common.kt
 *
 * Kotlin file header (translated):
 *   package kotlinx.coroutines.internal
 *
 * Upstream:
 *   internal expect inline fun <T> probeCoroutineCreated(completion: Continuation<T>): Continuation<T>
 *   internal expect inline fun <T> probeCoroutineResumed(completion: Continuation<T>)
 *
 * The JVM target wires these into kotlinx-coroutines-core/jvm/src/internal/ProbesSupport.kt to
 * surface coroutines to the JVM agent's `DebugProbes`. K/N has no agent, so both are no-ops.
 * The C++ port matches the K/N behavior — debug probes are a JVM-only feature.
 */

namespace kotlinx::coroutines::internal {

template <typename T>
class Continuation;

template <typename T>
inline Continuation<T>* probe_coroutine_created(Continuation<T>* completion) {
    return completion;
}

template <typename T>
inline void probe_coroutine_resumed(Continuation<T>* /*completion*/) {}

} // namespace kotlinx::coroutines::internal
