#pragma once
#include "kotlinx/coroutines/flow/Flow.hpp"
#include "kotlinx/coroutines/flow/FlowBuilders.hpp"
#include "kotlinx/coroutines/flow/internal/FlowExceptions.hpp"
#include <functional>
#include <stdexcept>
#include <memory>

namespace kotlinx {
namespace coroutines {
namespace flow {

/**
 * Returns a flow that ignores first [count] elements.
 */
template<typename T>
std::shared_ptr<Flow<T>> drop(std::shared_ptr<Flow<T>> flow, int count) {
    if (count < 0) throw std::invalid_argument("Drop count should be non-negative");
    
    return flow<T>([flow, count](FlowCollector<T>* collector) {
        int skipped = 0;
        flow->collect([&](T value) {
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
std::shared_ptr<Flow<T>> dropWhile(std::shared_ptr<Flow<T>> flow, Predicate predicate) {
    return flow<T>([flow, predicate](FlowCollector<T>* collector) {
        bool matched = false;
        flow->collect([&](T value) {
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
std::shared_ptr<Flow<T>> take(std::shared_ptr<Flow<T>> flow, int count) {
    if (count <= 0) throw std::invalid_argument("Requested element count should be positive");
    
    return flow<T>([flow, count](FlowCollector<T>* collector) {
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
            flow->collect(&tc);
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
void collectWhile(std::shared_ptr<Flow<T>> flow, Predicate predicate) {
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
        flow->collect(&collector);
    } catch (internal::AbortFlowException& e) {
        // e.checkOwnership(&collector);
    }
}

/**
 * Returns a flow that contains first elements satisfying the given [predicate].
 */
template<typename T, typename Predicate>
std::shared_ptr<Flow<T>> takeWhile(std::shared_ptr<Flow<T>> flow, Predicate predicate) {
    return flow<T>([flow, predicate](FlowCollector<T>* collector) {
        collectWhile(flow, [&](T value) -> bool {
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
std::shared_ptr<Flow<R>> transformWhile(std::shared_ptr<Flow<T>> flow, Transform transform) {
    return flow<R>([flow, transform](FlowCollector<R>* collector) {
        collectWhile(flow, [&](T value) -> bool {
             return transform(collector, value);
        });
    });
}

} // namespace flow
} // namespace coroutines
} // namespace kotlinx

