#pragma once
#include "kotlinx/coroutines/flow/Flow.hpp"
#include <chrono>

namespace kotlinx {
namespace coroutines {
namespace flow {

/**
 * Returns a flow that mirrors the original flow, but filters out values
 * that are followed by the newer values within the given timeout.
 */
template<typename T>
Flow<T>* debounce(Flow<T>* flow, long timeout_millis) {
    if (timeout_millis < 0) throw std::invalid_argument("Debounce timeout should not be negative");
    if (timeout_millis == 0) return flow;
    // TODO: Implement debounce logic
    return flow; 
}

/**
 * Returns a flow that mirrors the original flow, but filters out values
 * that are followed by the newer values within the given timeout (dynamic).
 */
template<typename T, typename Fn>
Flow<T>* debounce(Flow<T>* flow, Fn timeout_millis_selector) {
    // TODO: Implement dynamic debounce
    return flow;
}

/**
 * Returns a flow that emits only the latest value emitted by the original flow during the given sampling period.
 */
template<typename T>
Flow<T>* sample(Flow<T>* flow, long period_millis) {
    if (period_millis <= 0) throw std::invalid_argument("Sample period should be positive");
    // TODO: Implement sample logic
    return flow;
}

/**
 * Returns a flow that will emit a TimeoutCancellationException if the upstream doesn't emit an item within the given time.
 */
template<typename T>
Flow<T>* timeout(Flow<T>* flow, long timeout_millis) {
     // TODO: Implement timeout logic
     // Requires 'TimeoutCancellationException'
    return flow;
}

} // namespace flow
} // namespace coroutines
} // namespace kotlinx
