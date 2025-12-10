#pragma once
#include "kotlinx/coroutines/flow/Flow.hpp"
#include "kotlinx/coroutines/flow/FlowCollector.hpp"
#include <functional>
#include <exception>

namespace kotlinx {
namespace coroutines {
namespace flow {

/**
 * Catches exceptions in the flow completion and calls a specified [action] with the caught exception.
 */
template<typename T>
Flow<T>* catch_(Flow<T>* flow, std::function<void(FlowCollector<T>*, std::exception_ptr)> action) {
    // TODO: Implement catch operator
    return flow;
}

/**
 * Retries collection of the given flow up to [retries] times.
 */
template<typename T>
Flow<T>* retry(Flow<T>* flow, long retries = -1, std::function<bool(std::exception_ptr)> predicate = [](std::exception_ptr){ return true; }) {
    // TODO: Implement retry logic
    return flow;
}

/**
 * Retries collection of the given flow when an exception occurs in the upstream flow and the [predicate] returns true.
 */
template<typename T>
Flow<T>* retryWhen(Flow<T>* flow, std::function<bool(FlowCollector<T>*, std::exception_ptr, long)> predicate) {
    // TODO: Implement retryWhen
    return flow;
}

} // namespace flow
} // namespace coroutines
} // namespace kotlinx
