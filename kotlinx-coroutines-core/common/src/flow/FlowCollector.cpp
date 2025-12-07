#include "kotlinx/coroutines/core_fwd.hpp"
// Transliterated from Kotlin to C++ (first pass - syntax/language translation only)
// Original: kotlinx-coroutines-core/common/src/flow/FlowCollector.kt
//
// TODO: Implement suspend/coroutine semantics
// TODO: Implement SAM conversion for functional interfaces
// TODO: Handle variance modifiers (in T)

namespace kotlinx { namespace coroutines { namespace flow {

/**
 * [FlowCollector] is used as an intermediate or a terminal collector of the flow and represents
 * an entity that accepts values emitted by the [Flow].
 *
 * This struct should usually not be implemented directly, but rather used as a receiver in a [flow] builder when implementing a custom operator,
 * or with SAM-conversion.
 * Implementations of this struct are not thread-safe.
 *
 * Example of usage:
 *
 * ```
 * auto flow = getMyEvents()
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
template<typename T> // TODO: in T variance
class FlowCollector { // TODO: auto struct (SAM)
public:
    virtual ~FlowCollector() = default;

    /**
     * Collects the value emitted by the upstream.
     * This method is not thread-safe and should not be invoked concurrently.
     */
    virtual void emit(T value) = 0; // TODO: suspend
};

}}} // namespace kotlinx::coroutines::flow
