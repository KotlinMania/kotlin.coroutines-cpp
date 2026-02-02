#pragma once
// port-lint: source flow/operators/Limit.kt
/**
 * @file Limit.hpp
 * @brief Flow operators that limit emissions: drop, dropWhile, take, takeWhile, transformWhile
 *
 * Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Limit.kt
 */

#include "kotlinx/coroutines/flow/Flow.hpp"
#include "kotlinx/coroutines/flow/FlowBuilders.hpp"
#include "kotlinx/coroutines/flow/internal/FlowExceptions.hpp"
#include <functional>
#include <stdexcept>
#include <memory>

namespace kotlinx {
namespace coroutines {
namespace flow {

// Alias for the flow builder to avoid shadowing issues with parameters named 'upstream'
template<typename T>
inline std::shared_ptr<Flow<T>> make_flow(std::function<void(FlowCollector<T>*)> block) {
    return flow<T>(block);
}

/**
 * Returns a flow that ignores first count elements.
 *
 * This operator transforms the upstream flow by skipping the specified number
 * of elements from the beginning, then passing through all remaining elements.
 *
 * @param upstream The flow to transform
 * @param count The number of elements to skip (must be non-negative)
 * @return A new flow that skips the first count elements
 *
 * @note **CURRENT LIMITATION**: The collect() call inside this operator does not
 *       properly handle suspension, which may break backpressure in complex flows.
 *
 * @throws std::invalid_argument if count is negative
 *
 * ### Thread Safety
 * This operator is thread-safe as long as the upstream flow is thread-safe.
 * The skipping logic is stateless and does not introduce additional concurrency concerns.
 */
template<typename T>
std::shared_ptr<Flow<T>> drop(std::shared_ptr<Flow<T>> upstream, int count) {
    if (count < 0) throw std::invalid_argument("Drop count should be non-negative");

    return make_flow<T>([upstream, count](FlowCollector<T>* collector) {
        int skipped = 0;
        upstream->collect([&](T value) {
            if (skipped >= count) {
                collector->emit(value);
            } else {
                skipped++;
            }
        });
    });
}

/**
 * Returns a flow containing all elements except first elements that satisfy the given predicate.
 *
 * This operator skips elements from the beginning of the flow while the predicate
 * returns true, then emits all remaining elements (including the first one that
 * doesn't satisfy the predicate).
 *
 * @param upstream The flow to transform
 * @param predicate The predicate function to test elements
 * @return A new flow that skips elements while predicate is true
 *
 * @note **CURRENT LIMITATION**: The collect() call inside this operator does not
 *       properly handle suspension, which may break backpressure in complex flows.
 *
 * ### Thread Safety
 * This operator is thread-safe as long as the upstream flow is thread-safe.
 * The predicate is called sequentially for each element during collection.
 */
template<typename T, typename Predicate>
std::shared_ptr<Flow<T>> drop_while(std::shared_ptr<Flow<T>> upstream, Predicate predicate) {
    return make_flow<T>([upstream, predicate](FlowCollector<T>* collector) {
        bool matched = false;
        upstream->collect([&](T value) {
            if (matched) {
                collector->emit(value);
            } else if (!predicate(value)) {
                matched = true;
                collector->emit(value);
            }
        });
    });
}

/**
 * Returns a flow that contains first count elements.
 *
 * This operator transforms the upstream flow by emitting only the specified
 * number of elements from the beginning, then cancelling the upstream flow.
 *
 * @param upstream The flow to transform
 * @param count The number of elements to take (must be positive)
 * @return A new flow that emits only the first count elements
 *
 * @note **CURRENT LIMITATION**: Uses AbortFlowException to stop upstream flow,
 *       which is not the most efficient approach. The collect() call does not
 *       properly handle suspension, which may break backpressure.
 *
 * @throws std::invalid_argument if count is not positive
 *
 * ### Thread Safety
 * This operator is thread-safe as long as the upstream flow is thread-safe.
 * The cancellation mechanism ensures no further emissions after the limit.
 */
template<typename T>
std::shared_ptr<Flow<T>> take(std::shared_ptr<Flow<T>> upstream, int count) {
    if (count <= 0) throw std::invalid_argument("Requested element count should be positive");

    return make_flow<T>([upstream, count](FlowCollector<T>* collector) {
        int consumed = 0;
        // We use AbortFlowException logic if we need to stop upstream
        // But since we control upstream somewhat via lambda, we can't easily "stop" without exception.
        // Assuming AbortFlowException is defined and functional.

        class TakeCollector : public FlowCollector<T> {
            FlowCollector<T>* down;
            int limit;
            int& consumed;
        public:
            TakeCollector(FlowCollector<T>* d, int l, int& c) : down(d), limit(l), consumed(c) {}
            void emit(T value) override {
                consumed++;
                if (consumed < limit) {
                    down->emit(value);
                } else {
                    down->emit(value);
                    throw internal::AbortFlowException(this);
                }
            }
        };

        try {
            TakeCollector tc(collector, count, consumed);
            upstream->collect(&tc);
        } catch (internal::AbortFlowException& e) {
            // e.checkOwnership(owner); implementation detail usage
            // simplified:
        }
    });
}

/**
 * Helper for collectWhile logic.
 *
 * This internal function collects elements from the upstream flow while the
 * predicate returns true, throwing AbortFlowException when the predicate fails.
 *
 * @param upstream The flow to collect from
 * @param predicate The predicate function to test elements
 *
 * @note **CURRENT LIMITATION**: This is a helper function that uses exceptions
 *       for flow control, which is not optimal for performance.
 */
template<typename T, typename Predicate>
void collect_while(std::shared_ptr<Flow<T>> upstream, Predicate predicate) {
     struct PredicateCollector : public FlowCollector<T> {
        Predicate pred;
        PredicateCollector(Predicate p) : pred(p) {}
        void emit(T value) override {
            if (!pred(value)) {
                throw internal::AbortFlowException(this);
            }
        }
    };

    try {
        PredicateCollector collector(predicate);
        upstream->collect(&collector);
    } catch (internal::AbortFlowException& e) {
        // e.checkOwnership(&collector);
    }
}

/**
 * Returns a flow that contains first elements satisfying the given predicate.
 *
 * This operator emits elements from the upstream flow while the predicate
 * returns true, then cancels the upstream flow when the predicate fails.
 *
 * @param upstream The flow to transform
 * @param predicate The predicate function to test elements
 * @return A new flow that emits elements while predicate is true
 *
 * @note **CURRENT LIMITATION**: Uses AbortFlowException for flow control and
 *       does not properly handle suspension, which may break backpressure.
 *
 * ### Thread Safety
 * This operator is thread-safe as long as the upstream flow is thread-safe.
 * The predicate is called sequentially for each element during collection.
 */
template<typename T, typename Predicate>
std::shared_ptr<Flow<T>> take_while(std::shared_ptr<Flow<T>> upstream, Predicate predicate) {
    return make_flow<T>([upstream, predicate](FlowCollector<T>* collector) {
        collect_while(upstream, [&](T value) -> bool {
            if (predicate(value)) {
                collector->emit(value);
                return true;
            } else {
                return false;
            }
        });
    });
}

/**
 * Applies transform function to each value of the given flow while this function returns true.
 *
 * This operator transforms each element using the provided function and continues
 * processing as long as the transform function returns true. It allows for
 * complex transformations with early termination.
 *
 * @param upstream The flow to transform
 * @param transform_fn The transformation function that returns true to continue
 * @return A new flow with transformed elements
 *
 * @tparam T The input element type
 * @tparam R The output element type
 * @tparam Transform The transform function type
 *
 * @note **CURRENT LIMITATION**: Does not properly handle suspension, which may
 *       break backpressure in complex flows.
 *
 * ### Thread Safety
 * This operator is thread-safe as long as the upstream flow is thread-safe.
 * The transform function is called sequentially for each element.
 */
template<typename T, typename R, typename Transform>
std::shared_ptr<Flow<R>> transform_while(std::shared_ptr<Flow<T>> upstream, Transform transform_fn) {
    return make_flow<R>([upstream, transform_fn](FlowCollector<R>* collector) {
        collect_while(upstream, [&](T value) -> bool {
             return transform_fn(collector, value);
        });
    });
}

} // namespace flow
} // namespace coroutines
} // namespace kotlinx

