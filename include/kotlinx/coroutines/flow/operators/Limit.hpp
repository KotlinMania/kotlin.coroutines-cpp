#pragma once
#include "kotlinx/coroutines/flow/Flow.hpp"
#include "kotlinx/coroutines/flow/FlowCollector.hpp"
#include <functional>
#include <stdexcept>

namespace kotlinx {
namespace coroutines {
#include <memory>

namespace kotlinx {
namespace coroutines {
namespace flow {

namespace internal {
    template<typename T>
    struct FlowCollectorImpl : public FlowCollector<T> {
        std::function<void(T)> emit_impl;
        FlowCollectorImpl(std::function<void(T)> e) : emit_impl(e) {}
        void emit(T value) override {
            emit_impl(value);
        }
    };

    template<typename T>
    struct FlowImpl : public Flow<T> {
        std::function<void(FlowCollector<T>*)> collect_impl;
        FlowImpl(std::function<void(FlowCollector<T>*)> c) : collect_impl(c) {}
        void collect(FlowCollector<T>* collector) override {
            collect_impl(collector);
        }
    };
}

#include "kotlinx/coroutines/flow/internal/FlowExceptions.hpp"
#include "kotlinx/coroutines/flow/internal/SafeCollector.hpp" // For safeFlow if needed, or just standard Flow impl

using namespace kotlinx::coroutines::flow::internal;

/**
 * Returns a flow that ignores first [count] elements.
 */
template<typename T>
Flow<T>* drop(Flow<T>* flow, int count) {
    if (count < 0) throw std::invalid_argument("Drop count should be non-negative, but had " + std::to_string(count));
    
    return new FlowImpl<T>([flow, count](FlowCollector<T>* collector) {
        auto skipped = std::make_shared<int>(0); // Shared state for capture
        flow->collect(new FlowCollectorImpl<T>([collector, count, skipped](T value) {
            if (*skipped >= count) {
                collector->emit(value);
            } else {
                (*skipped)++;
            }
        }));
    });
}

/**
 * Returns a flow containing all elements except first elements that satisfy the given predicate.
 */
template<typename T, typename Predicate>
Flow<T>* dropWhile(Flow<T>* flow, Predicate predicate) {
    return new FlowImpl<T>([flow, predicate](FlowCollector<T>* collector) {
        auto matched = std::make_shared<bool>(false);
        flow->collect(new FlowCollectorImpl<T>([collector, predicate, matched](T value) {
            if (*matched) {
                collector->emit(value);
            } else if (!predicate(value)) {
                *matched = true;
                collector->emit(value);
            }
        }));
    });
}

/**
 * Returns a flow that contains first [count] elements.
 */
template<typename T>
Flow<T>* take(Flow<T>* flow, int count) {
    if (count <= 0) throw std::invalid_argument("Requested element count should be positive");
    
    return new FlowImpl<T>([flow, count](FlowCollector<T>* collector) {
        auto consumed = std::make_shared<int>(0);
        void* ownership_marker = consumed.get(); // Unique address for this collection
        
        try {
            flow->collect(new FlowCollectorImpl<T>([collector, count, consumed, ownership_marker](T value) {
                (*consumed)++;
                if (*consumed < count) {
                    collector->emit(value);
                } else {
                    collector->emit(value);
                    throw AbortFlowException(ownership_marker);
                }
            }));
        } catch (AbortFlowException& e) {
            e.checkOwnership(ownership_marker);
        }
    });
}

/**
 * Helper for collectWhile logic
 */
template<typename T, typename Predicate>
void collectWhile(Flow<T>* flow, Predicate predicate) {
     struct PredicateCollector : public FlowCollector<T> {
        Predicate pred;
        PredicateCollector(Predicate p) : pred(p) {}
        void emit(T value) override {
            if (!pred(value)) {
                throw AbortFlowException(this);
            }
        }
    };
    
    PredicateCollector collector(predicate);
    try {
        flow->collect(&collector);
    } catch (AbortFlowException& e) {
        e.checkOwnership(&collector);
    }
}

/**
 * Returns a flow that contains first elements satisfying the given [predicate].
 */
template<typename T, typename Predicate>
Flow<T>* takeWhile(Flow<T>* flow, Predicate predicate) {
    return new FlowImpl<T>([flow, predicate](FlowCollector<T>* collector) {
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
Flow<R>* transformWhile(Flow<T>* flow, Transform transform) {
    return new FlowImpl<R>([flow, transform](FlowCollector<R>* collector) {
        collectWhile(flow, [&](T value) -> bool {
             return transform(collector, value);
        });
    });
}

} // namespace flow
} // namespace coroutines
} // namespace kotlinx
