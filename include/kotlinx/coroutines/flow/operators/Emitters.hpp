#pragma once
#include "kotlinx/coroutines/flow/Flow.hpp"
#include "kotlinx/coroutines/flow/FlowBuilders.hpp"
#include <functional>
#include <exception>
#include <memory>

namespace kotlinx {
namespace coroutines {
namespace flow {

/**
 * Applies [transform] function to each value of the given flow.
 */
template <typename T, typename R>
std::shared_ptr<Flow<R>> transform(std::shared_ptr<Flow<T>> flow, std::function<void(FlowCollector<R>*, T)> transform) {
    return flow<R>([flow, transform](FlowCollector<R>* collector) {
        flow->collect([&](T value) {
            transform(collector, value);
        });
    });
}

/**
 * Returns a flow that invokes the given [action] BEFORE this flow starts to be collected.
 */
template <typename T>
std::shared_ptr<Flow<T>> onStart(std::shared_ptr<Flow<T>> src, std::function<void(FlowCollector<T>*)> action) {
    return flow<T>([src, action](FlowCollector<T>* collector) {
        // SafeCollector logic would go here
        action(collector);
        src->collect(collector);
    });
}

/**
 * Returns a flow that invokes the given [action] AFTER the flow is completed or cancelled.
 */
template <typename T>
std::shared_ptr<Flow<T>> onCompletion(std::shared_ptr<Flow<T>> src, std::function<void(FlowCollector<T>*, std::exception_ptr)> action) {
    return flow<T>([src, action](FlowCollector<T>* collector) {
        try {
            src->collect(collector);
            action(collector, nullptr);
        } catch (...) {
            auto ex = std::current_exception();
            // Invoke safely
            try {
                action(collector, ex);
            } catch (...) {
                // If action throws, we might need to suppress or rethrow. 
            }
            throw; // Rethrow original
        }
    });
}

/**
 * Invokes the given [action] when this flow completes without emitting any elements.
 */
template <typename T>
std::shared_ptr<Flow<T>> onEmpty(std::shared_ptr<Flow<T>> src, std::function<void(FlowCollector<T>*)> action) {
    return flow<T>([src, action](FlowCollector<T>* collector) {
        bool isEmpty = true;
        src->collect([&](T value) {
            isEmpty = false;
            collector->emit(value);
        });
        if (isEmpty) {
            action(collector);
        }
    });
}

} // namespace flow
} // namespace coroutines
} // namespace kotlinx

