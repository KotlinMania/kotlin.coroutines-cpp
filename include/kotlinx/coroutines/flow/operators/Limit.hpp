#pragma once
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
 * Returns a flow that ignores first [count] elements.
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
 * Returns a flow that contains first [count] elements.
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
 * Helper for collectWhile logic
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
 * Returns a flow that contains first elements satisfying the given [predicate].
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
 * Applies [transform] function to each value of the given flow while this function returns `true`.
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

