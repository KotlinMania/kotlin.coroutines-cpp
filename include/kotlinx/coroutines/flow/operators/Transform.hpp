#pragma once
/**
 * @file Transform.hpp
 * @brief Flow transformation operators: filter, map, mapNotNull
 *
 * Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Transform.kt
 */

#include "kotlinx/coroutines/flow/Flow.hpp"
#include "kotlinx/coroutines/flow/FlowBuilders.hpp"
#include <functional>
#include <memory>
#include <optional>

namespace kotlinx {
namespace coroutines {
namespace flow {

// Alias for the flow builder to avoid shadowing issues with parameters
template<typename T>
inline std::shared_ptr<Flow<T>> make_flow(std::function<void(FlowCollector<T>*)> block) {
    return flow<T>(block);
}

/**
 * Returns a flow containing only values of the original flow that match the given predicate.
 */
template <typename T>
std::shared_ptr<Flow<T>> filter(std::shared_ptr<Flow<T>> upstream, std::function<bool(T)> predicate) {
    return make_flow<T>([upstream, predicate](FlowCollector<T>* collector) {
        upstream->collect([&](T value) {
            if (predicate(value)) {
                collector->emit(value);
            }
        });
    });
}

/**
 * Returns a flow containing the results of applying the given transform function
 * to each value of the original flow.
 */
template <typename T, typename R>
std::shared_ptr<Flow<R>> map(std::shared_ptr<Flow<T>> upstream, std::function<R(T)> transform_fn) {
    return make_flow<R>([upstream, transform_fn](FlowCollector<R>* collector) {
        upstream->collect([&](T value) {
            collector->emit(transform_fn(value));
        });
    });
}

/**
 * Returns a flow containing only the non-null results of applying the given transform function
 * to each value of the original flow.
 *
 * This version uses raw pointers for nullable return (nullptr = null).
 */
template <typename T, typename R>
std::shared_ptr<Flow<R>> map_not_null(std::shared_ptr<Flow<T>> upstream, std::function<R*(T)> transform_fn) {
    return make_flow<R>([upstream, transform_fn](FlowCollector<R>* collector) {
        upstream->collect([&](T value) {
            auto result = transform_fn(value);
            if (result != nullptr) {
                collector->emit(*result);
            }
        });
    });
}

/**
 * Returns a flow containing only the non-null results of applying the given transform function
 * to each value of the original flow.
 *
 * This version uses std::optional for nullable return.
 */
template <typename T, typename R>
std::shared_ptr<Flow<R>> map_not_null_opt(std::shared_ptr<Flow<T>> upstream, std::function<std::optional<R>(T)> transform_fn) {
    return make_flow<R>([upstream, transform_fn](FlowCollector<R>* collector) {
        upstream->collect([&](T value) {
            auto result = transform_fn(value);
            if (result.has_value()) {
                collector->emit(result.value());
            }
        });
    });
}

/**
 * Returns a flow that contains only non-null results of applying the given transform function
 * to each value of the original flow.
 */
template <typename T>
std::shared_ptr<Flow<T>> filter_not_null(std::shared_ptr<Flow<T*>> upstream) {
    return make_flow<T>([upstream](FlowCollector<T>* collector) {
        upstream->collect([&](T* value) {
            if (value != nullptr) {
                collector->emit(*value);
            }
        });
    });
}

} // namespace flow
} // namespace coroutines
} // namespace kotlinx

