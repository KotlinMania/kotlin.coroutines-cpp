#pragma once
#include "kotlinx/coroutines/flow/Flow.hpp"
#include "kotlinx/coroutines/flow/FlowCollector.hpp"
#include <functional>
#include <exception>

namespace kotlinx {
namespace coroutines {
namespace flow {

/**
 * Applies [transform] function to each value of the given flow.
 */
#include "kotlinx/coroutines/flow/internal/FlowImpl.hpp"
#include <memory>

using namespace kotlinx::coroutines::flow::internal;

/**
 * Applies [transform] function to each value of the given flow.
 */
template <typename T, typename R>
Flow<R>* transform(Flow<T>* flow, std::function<void(FlowCollector<R>*, T)> transform) {
    return new FlowImpl<R>([flow, transform](FlowCollector<R>* collector) {
        flow->collect(new FlowCollectorImpl<T>([collector, transform](T value) {
            transform(collector, value);
        }));
    });
}

/**
 * Returns a flow that invokes the given [action] BEFORE this flow starts to be collected.
 */
template <typename T>
Flow<T>* onStart(Flow<T>* flow, std::function<void(FlowCollector<T>*)> action) {
    return new FlowImpl<T>([flow, action](FlowCollector<T>* collector) {
        // SafeCollector logic would go here
        action(collector);
        flow->collect(collector);
    });
}

/**
 * Returns a flow that invokes the given [action] AFTER the flow is completed or cancelled.
 */
template <typename T>
Flow<T>* onCompletion(Flow<T>* flow, std::function<void(FlowCollector<T>*, std::exception_ptr)> action) {
    return new FlowImpl<T>([flow, action](FlowCollector<T>* collector) {
        try {
            flow->collect(collector);
            action(collector, nullptr);
        } catch (...) {
            auto ex = std::current_exception();
            // Invoke safely
            try {
                action(collector, ex);
            } catch (...) {
                // If action throws, we might need to suppress or rethrow. 
                // Simple rethrow of original exception for now if action fails?
                // Kotlin behavior: throw e
            }
            throw; // Rethrow original
        }
    });
}

/**
 * Invokes the given [action] when this flow completes without emitting any elements.
 */
template <typename T>
Flow<T>* onEmpty(Flow<T>* flow, std::function<void(FlowCollector<T>*)> action) {
    return new FlowImpl<T>([flow, action](FlowCollector<T>* collector) {
        auto isEmpty = std::make_shared<bool>(true);
        flow->collect(new FlowCollectorImpl<T>([collector, isEmpty](T value) {
            *isEmpty = false;
            collector->emit(value);
        }));
        if (*isEmpty) {
            action(collector);
        }
    });
}

} // namespace flow
} // namespace coroutines
} // namespace kotlinx
