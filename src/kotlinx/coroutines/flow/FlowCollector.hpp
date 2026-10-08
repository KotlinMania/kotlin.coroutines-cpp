#pragma once
/**
 * Transliterated from: kotlinx-coroutines-core/common/src/flow/FlowCollector.kt
 *
 * Kotlin file header (translated):
 *   package kotlinx.coroutines.flow
 */

#include "kotlinx/coroutines/Continuation.hpp"

namespace kotlinx::coroutines::flow {

/**
 * [FlowCollector] is used as an intermediate or a terminal collector of the flow and represents
 * an entity that accepts values emitted by the [Flow].
 *
 * This interface should usually not be implemented directly, but rather used as a receiver in a [flow] builder when implementing a custom operator,
 * or with SAM-conversion.
 * Implementations of this interface are not thread-safe.
 *
 * Example of usage:
 *
 * ```kotlin
 * val flow = getMyEvents()
 * try {
 *     flow.collect { value ->
 *         println("Received $value")
 *     }
 *     println("My events are consumed successfully")
 * } catch (e: Throwable) {
 *     println("Exception from the flow: $e")
 * }
 * ```
 */
// Transliterated from: kotlinx-coroutines-core/common/src/flow/FlowCollector.kt:25-32
template <typename T>
struct FlowCollector {
    virtual ~FlowCollector() = default;

    /**
     * Collects the value emitted by the upstream. This method is not thread-safe and should
     * not be invoked concurrently.
     *
     * The Kotlin signature `suspend fun emit(value: T)` translates here to the project's
     * suspend ABI: a `Continuation<void*>*` is threaded through as the last parameter and the
     * return is `void*` with `COROUTINE_SUSPENDED` indicating suspension.
     */
    // Transliterated from: kotlinx-coroutines-core/common/src/flow/FlowCollector.kt:31-31
    virtual void* emit(T value, Continuation<void*>* continuation) = 0;
};

} // namespace kotlinx::coroutines::flow
