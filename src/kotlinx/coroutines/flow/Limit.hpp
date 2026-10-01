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
#include <utility>

namespace kotlinx {
namespace coroutines {
namespace flow {

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
 * @throws std::invalid_argument if count is negative
 */
template<typename T>
std::shared_ptr<Flow<T>> drop(std::shared_ptr<Flow<T>> upstream, int count) {
    if (count < 0) throw std::invalid_argument("Drop count should be non-negative");

    return flow<T>([upstream, count](FlowCollector<T>* collector, Continuation<void*>* cont) -> void* {
        class DropCollector : public FlowCollector<T> {
        public:
            FlowCollector<T>* collector_;
            int count_;
            int skipped_{0};
            DropCollector(FlowCollector<T>* c, int cnt) : collector_(c), count_(cnt) {}
            void* emit(T value, Continuation<void*>* c) override {
                if (skipped_ >= count_) {
                    return collector_->emit(std::move(value), c);
                } else {
                    ++skipped_;
                    return nullptr;
                }
            }
        };
        DropCollector drop_collector(collector, count);
        return upstream->collect(&drop_collector, cont);
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
 */
template<typename T, typename Predicate>
std::shared_ptr<Flow<T>> drop_while(std::shared_ptr<Flow<T>> upstream, Predicate predicate) {
    return flow<T>([upstream, predicate](FlowCollector<T>* collector, Continuation<void*>* cont) -> void* {
        class DropWhileCollector : public FlowCollector<T> {
        public:
            FlowCollector<T>* collector_;
            Predicate predicate_;
            bool matched_{false};
            DropWhileCollector(FlowCollector<T>* c, Predicate p) : collector_(c), predicate_(p) {}
            void* emit(T value, Continuation<void*>* c) override {
                if (matched_) {
                    return collector_->emit(std::move(value), c);
                } else if (!predicate_(value)) {
                    matched_ = true;
                    return collector_->emit(std::move(value), c);
                }
                return nullptr;
            }
        };
        DropWhileCollector dw_collector(collector, predicate);
        return upstream->collect(&dw_collector, cont);
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
 * @throws std::invalid_argument if count is not positive
 */
template<typename T>
std::shared_ptr<Flow<T>> take(std::shared_ptr<Flow<T>> upstream, int count) {
    if (count <= 0) throw std::invalid_argument("Requested element count should be positive");

    return flow<T>([upstream, count](FlowCollector<T>* collector, Continuation<void*>* cont) -> void* {
        class TakeCollector : public FlowCollector<T> {
            FlowCollector<T>* down_;
            int limit_;
            int consumed_{0};
        public:
            TakeCollector(FlowCollector<T>* d, int l) : down_(d), limit_(l) {}
            void* emit(T value, Continuation<void*>* c) override {
                consumed_++;
                if (consumed_ < limit_) {
                    return down_->emit(std::move(value), c);
                } else {
                    down_->emit(std::move(value), c);
                    throw internal::AbortFlowException(this);
                }
            }
        };

        TakeCollector tc(collector, count);
        try {
            return upstream->collect(&tc, cont);
        } catch (internal::AbortFlowException& e) {
            e.check_ownership(&tc);
            return nullptr;
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
 */
template<typename T, typename Predicate>
void* collect_while(std::shared_ptr<Flow<T>> upstream, Predicate predicate, Continuation<void*>* cont = nullptr) {
    struct PredicateCollector : public FlowCollector<T> {
        Predicate pred;
        PredicateCollector(Predicate p) : pred(p) {}
        void* emit(T value, Continuation<void*>*) override {
            if (!pred(value)) {
                throw internal::AbortFlowException(this);
            }
            return nullptr;
        }
    };

    PredicateCollector collector(predicate);
    try {
        return upstream->collect(&collector, cont);
    } catch (internal::AbortFlowException& e) {
        e.check_ownership(&collector);
        return nullptr;
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
 */
template<typename T, typename Predicate>
std::shared_ptr<Flow<T>> take_while(std::shared_ptr<Flow<T>> upstream, Predicate predicate) {
    return flow<T>([upstream, predicate](FlowCollector<T>* collector, Continuation<void*>* cont) -> void* {
        class TakeWhileCollector : public FlowCollector<T> {
            FlowCollector<T>* down_;
            Predicate pred_;
        public:
            TakeWhileCollector(FlowCollector<T>* d, Predicate p) : down_(d), pred_(p) {}
            void* emit(T value, Continuation<void*>* c) override {
                if (pred_(value)) {
                    return down_->emit(std::move(value), c);
                } else {
                    throw internal::AbortFlowException(this);
                }
            }
        };

        TakeWhileCollector twc(collector, predicate);
        try {
            return upstream->collect(&twc, cont);
        } catch (internal::AbortFlowException& e) {
            e.check_ownership(&twc);
            return nullptr;
        }
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
 */
template<typename T, typename R, typename Transform>
std::shared_ptr<Flow<R>> transform_while(std::shared_ptr<Flow<T>> upstream, Transform transform_fn) {
    return flow<R>([upstream, transform_fn](FlowCollector<R>* collector, Continuation<void*>* cont) -> void* {
        class TransformWhileCollector : public FlowCollector<T> {
            FlowCollector<R>* down_;
            Transform fn_;
        public:
            TransformWhileCollector(FlowCollector<R>* d, Transform fn) : down_(d), fn_(fn) {}
            void* emit(T value, Continuation<void*>*) override {
                if (fn_(down_, std::move(value))) {
                    return nullptr;
                } else {
                    throw internal::AbortFlowException(this);
                }
            }
        };

        TransformWhileCollector twc(collector, transform_fn);
        try {
            return upstream->collect(&twc, cont);
        } catch (internal::AbortFlowException& e) {
            e.check_ownership(&twc);
            return nullptr;
        }
    });
}

} // namespace flow
} // namespace coroutines
} // namespace kotlinx
