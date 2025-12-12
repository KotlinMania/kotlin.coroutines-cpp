#pragma once
/**
 * @file Emitters.hpp
 * @brief Flow operators that emit values: transform, onStart, onCompletion, onEmpty
 *
 * Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Emitters.kt
 */

#include "kotlinx/coroutines/flow/Flow.hpp"
#include "kotlinx/coroutines/flow/FlowBuilders.hpp"
#include <functional>
#include <exception>
#include <memory>

namespace kotlinx {
namespace coroutines {
namespace flow {

// Alias for the flow builder to avoid shadowing issues with parameters named 'flow'
template<typename T>
inline std::shared_ptr<Flow<T>> make_flow(std::function<void(FlowCollector<T>*)> block) {
    return flow<T>(block);
}

/**
 * Applies transform function to each value of the given flow.
 *
 * The receiver of the transform is FlowCollector and thus transform is a
 * flexible function that may transform emitted element, skip it or emit it multiple times.
 */
template <typename T, typename R>
std::shared_ptr<Flow<R>> transform(std::shared_ptr<Flow<T>> upstream, std::function<void(FlowCollector<R>*, T)> transform_fn) {
    return make_flow<R>([upstream, transform_fn](FlowCollector<R>* collector) {
        upstream->collect([&](T value) {
            transform_fn(collector, value);
        });
    });
}

/**
 * Returns a flow that invokes the given action BEFORE this flow starts to be collected.
 *
 * The action is called before the upstream flow is collected. Action may emit values using
 * the collector or just perform some side effect.
 */
template <typename T>
std::shared_ptr<Flow<T>> on_start(std::shared_ptr<Flow<T>> upstream, std::function<void(FlowCollector<T>*)> action) {
    return make_flow<T>([upstream, action](FlowCollector<T>* collector) {
        // TODO: Use SafeCollector for proper context validation
        action(collector);
        upstream->collect(collector);
    });
}

/**
 * Returns a flow that invokes the given action AFTER the flow is completed or cancelled,
 * passing the cancellation exception or null if it completed successfully.
 *
 * Conceptually, onCompletion is similar to wrapping the flow collection into a finally block.
 */
template <typename T>
std::shared_ptr<Flow<T>> on_completion(std::shared_ptr<Flow<T>> upstream, std::function<void(FlowCollector<T>*, std::exception_ptr)> action) {
    return make_flow<T>([upstream, action](FlowCollector<T>* collector) {
        try {
            upstream->collect(collector);
            action(collector, nullptr);
        } catch (...) {
            auto ex = std::current_exception();
            try {
                action(collector, ex);
            } catch (...) {
                // If action throws, suppress it and rethrow original
            }
            throw; // Rethrow original exception
        }
    });
}

/**
 * Invokes the given action when this flow completes without emitting any elements.
 *
 * The receiver of action is FlowCollector so action can emit additional elements.
 */
template <typename T>
std::shared_ptr<Flow<T>> on_empty(std::shared_ptr<Flow<T>> upstream, std::function<void(FlowCollector<T>*)> action) {
    return make_flow<T>([upstream, action](FlowCollector<T>* collector) {
        bool isEmpty = true;
        upstream->collect([&](T value) {
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
