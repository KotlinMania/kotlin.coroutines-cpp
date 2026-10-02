// port-lint: source kotlinx-coroutines-core/common/src/flow/terminal/Reduce.kt
#pragma once
/**
 * @file Reduce.hpp
 * @brief Terminal flow operators for reduction: reduce, fold, first, last, single
 *
 * Transliterated from: kotlinx-coroutines-core/common/src/flow/terminal/Reduce.kt
 */

#include "kotlinx/coroutines/Continuation.hpp"
#include "kotlinx/coroutines/Exceptions.hpp"
#include "kotlinx/coroutines/flow/Flow.hpp"
#include "kotlinx/coroutines/flow/FlowCollector.hpp"
#include "kotlinx/coroutines/flow/internal/FlowExceptions.hpp"
#include "kotlinx/coroutines/flow/internal/NullSurrogate.hpp"
#include "kotlinx/coroutines/intrinsics/Intrinsics.hpp"

#include <functional>
#include <memory>
#include <optional>
#include <stdexcept>
#include <utility>

namespace kotlinx::coroutines::flow {

/**
 * Accumulates value starting with the first element and applying [operation] to current accumulator value and each element.
 * Throws std::out_of_range if flow was empty.
 *
 * Transliterated from: kotlinx-coroutines-core/common/src/flow/terminal/Reduce.kt:15-30
 */
template<typename T, typename S = T>
inline void* reduce(
    std::shared_ptr<Flow<T>> flow,
    std::function<S(S, T)> operation,
    Continuation<void*>* continuation) {
    void* accumulator = &internal::NULL_VALUE();

    class ReduceCollector : public FlowCollector<T> {
        void** accumulator_;
        std::function<S(S, T)> operation_;
    public:
        ReduceCollector(void** acc, std::function<S(S, T)> op)
            : accumulator_(acc), operation_(std::move(op)) {
            (void)acc;
        }

        void* emit(T value, Continuation<void*>*) override {
            if (*accumulator_ == &internal::NULL_VALUE()) {
                *accumulator_ = new S(std::move(value));
            } else {
                S* acc = static_cast<S*>(*accumulator_);
                *acc = operation_(*acc, std::move(value));
            }
            return nullptr;
        }
    };

    ReduceCollector collector(&accumulator, operation);
    flow->collect(&collector, continuation);

    if (accumulator == &internal::NULL_VALUE()) {
        throw NoSuchElementException("Empty flow can't be reduced");
    }
    return accumulator;
}

/**
 * Suspending overload of [reduce].
 *
 * Transliterated from: kotlinx-coroutines-core/common/src/flow/terminal/Reduce.kt:15-30
 */
template<typename T, typename S = T>
inline void* reduce(
    std::shared_ptr<Flow<T>> flow,
    std::function<void*(S, T, Continuation<void*>*)> operation,
    Continuation<void*>* continuation) {
    void* accumulator = &internal::NULL_VALUE();

    class SuspendingReduceCollector : public FlowCollector<T> {
        void** accumulator_;
        std::function<void*(S, T, Continuation<void*>*)> operation_;
    public:
        SuspendingReduceCollector(void** acc, std::function<void*(S, T, Continuation<void*>*)> op)
            : accumulator_(acc), operation_(std::move(op)) {
            (void)acc;
        }

        void* emit(T value, Continuation<void*>* cont) override {
            if (*accumulator_ == &internal::NULL_VALUE()) {
                *accumulator_ = new S(std::move(value));
            } else {
                S* acc = static_cast<S*>(*accumulator_);
                void* res = operation_(*acc, std::move(value), cont);
                if (intrinsics::is_coroutine_suspended(res)) {
                    return intrinsics::get_COROUTINE_SUSPENDED();
                }
                if (res) {
                    auto* s_ptr = static_cast<S*>(res);
                    *acc = std::move(*s_ptr);
                    delete s_ptr;
                }
            }
            return nullptr;
        }
    };

    SuspendingReduceCollector collector(&accumulator, operation);
    void* r = flow->collect(&collector, continuation);
    if (intrinsics::is_coroutine_suspended(r)) {
        return intrinsics::get_COROUTINE_SUSPENDED();
    }

    if (accumulator == &internal::NULL_VALUE()) {
        throw NoSuchElementException("Empty flow can't be reduced");
    }
    return accumulator;
}

/**
 * Single-type overload of [reduce] where accumulator type equals element type.
 *
 * Transliterated from: kotlinx-coroutines-core/common/src/flow/terminal/Reduce.kt:15-30
 */
template<typename T>
inline void* reduce(
    std::shared_ptr<Flow<T>> flow,
    std::function<T(T, T)> operation,
    Continuation<void*>* continuation) {
    return reduce<T, T>(std::move(flow), std::move(operation), continuation);
}

/**
 * Single-type suspending overload of [reduce] where accumulator type equals element type.
 *
 * Transliterated from: kotlinx-coroutines-core/common/src/flow/terminal/Reduce.kt:15-30
 */
template<typename T>
inline void* reduce(
    std::shared_ptr<Flow<T>> flow,
    std::function<void*(T, T, Continuation<void*>*)> operation,
    Continuation<void*>* continuation) {
    return reduce<T, T>(std::move(flow), std::move(operation), continuation);
}

/**
 * Accumulates value starting with [initial] value and applying [operation] to current accumulator value and each element.
 *
 * Transliterated from: kotlinx-coroutines-core/common/src/flow/terminal/Reduce.kt:35-44
 */
template<typename T, typename R>
inline void* fold(
    std::shared_ptr<Flow<T>> flow,
    R initial,
    std::function<R(R, T)> operation,
    Continuation<void*>* continuation) {
    R* accumulator = new R(std::move(initial));

    class FoldCollector : public FlowCollector<T> {
        R* accumulator_;
        std::function<R(R, T)> operation_;
    public:
        FoldCollector(R* acc, std::function<R(R, T)> op)
            : accumulator_(acc), operation_(std::move(op)) {
            (void)acc;
        }

        void* emit(T value, Continuation<void*>*) override {
            *accumulator_ = operation_(*accumulator_, std::move(value));
            return nullptr;
        }
    };

    FoldCollector collector(accumulator, operation);
    flow->collect(&collector, continuation);
    return accumulator;
}

/**
 * Suspending overload of [fold].
 *
 * Transliterated from: kotlinx-coroutines-core/common/src/flow/terminal/Reduce.kt:35-44
 */
template<typename T, typename R>
inline void* fold(
    std::shared_ptr<Flow<T>> flow,
    R initial,
    std::function<void*(R, T, Continuation<void*>*)> operation,
    Continuation<void*>* continuation) {
    R* accumulator = new R(std::move(initial));

    class SuspendingFoldCollector : public FlowCollector<T> {
        R* accumulator_;
        std::function<void*(R, T, Continuation<void*>*)> operation_;
    public:
        SuspendingFoldCollector(R* acc, std::function<void*(R, T, Continuation<void*>*)> op)
            : accumulator_(acc), operation_(std::move(op)) {
            (void)acc;
        }

        void* emit(T value, Continuation<void*>* cont) override {
            void* res = operation_(*accumulator_, std::move(value), cont);
            if (intrinsics::is_coroutine_suspended(res)) {
                return intrinsics::get_COROUTINE_SUSPENDED();
            }
            if (res) {
                auto* r_ptr = static_cast<R*>(res);
                *accumulator_ = std::move(*r_ptr);
                delete r_ptr;
            }
            return nullptr;
        }
    };

    SuspendingFoldCollector collector(accumulator, operation);
    void* r = flow->collect(&collector, continuation);
    if (intrinsics::is_coroutine_suspended(r)) {
        return intrinsics::get_COROUTINE_SUSPENDED();
    }
    return accumulator;
}

/**
 * The terminal operator that awaits for one and only one value to be emitted.
 * Throws std::out_of_range for empty flow and std::invalid_argument for flow
 * that contains more than one element.
 *
 * Transliterated from: kotlinx-coroutines-core/common/src/flow/terminal/Reduce.kt:51-60
 */
template<typename T>
inline void* single(std::shared_ptr<Flow<T>> flow, Continuation<void*>* continuation) {
    void* result = &internal::NULL_VALUE();

    class SingleCollector : public FlowCollector<T> {
        void** result_;
    public:
        explicit SingleCollector(void** res) : result_(res) {
            (void)res;
        }

        void* emit(T value, Continuation<void*>*) override {
            if (*result_ != &internal::NULL_VALUE()) {
                throw std::invalid_argument("Flow has more than one element");
            }
            *result_ = new T(std::move(value));
            return nullptr;
        }
    };

    SingleCollector collector(&result);
    flow->collect(&collector, continuation);

    if (result == &internal::NULL_VALUE()) {
        throw NoSuchElementException("Flow is empty");
    }
    return result;
}

/**
 * The terminal operator that awaits for one and only one value to be emitted.
 * Returns the single value or nullptr, if the flow was empty or emitted more than one value.
 *
 * Transliterated from: kotlinx-coroutines-core/common/src/flow/terminal/Reduce.kt:66-80
 */
template<typename T>
inline void* single_or_null(std::shared_ptr<Flow<T>> flow, Continuation<void*>* continuation) {
    void* result = &internal::NULL_VALUE();
    bool has_multiple = false;

    class SingleOrNullCollector : public FlowCollector<T> {
        void** result_;
        bool* has_multiple_;
    public:
        SingleOrNullCollector(void** res, bool* mult)
            : result_(res), has_multiple_(mult) {
            (void)res;
            (void)mult;
        }

        void* emit(T value, Continuation<void*>*) override {
            if (*result_ == &internal::NULL_VALUE()) {
                *result_ = new T(std::move(value));
            } else {
                *has_multiple_ = true;
                delete static_cast<T*>(*result_);
                *result_ = &internal::NULL_VALUE();
                throw internal::AbortFlowException(this);
            }
            return nullptr;
        }
    };

    SingleOrNullCollector collector(&result, &has_multiple);
    try {
        flow->collect(&collector, continuation);
    } catch (internal::AbortFlowException& e) {
        e.check_ownership(&collector);
    }

    if (has_multiple || result == &internal::NULL_VALUE()) {
        return nullptr;
    }
    return result;
}

/**
 * The terminal operator that returns the first element emitted by the flow and then cancels flow's collection.
 * Throws NoSuchElementException if the flow was empty.
 *
 * Transliterated from: kotlinx-coroutines-core/common/src/flow/terminal/Reduce.kt:86-94
 */
template<typename T>
inline void* first(std::shared_ptr<Flow<T>> flow, Continuation<void*>* continuation) {
    void* result = &internal::NULL_VALUE();

    class FirstCollector : public FlowCollector<T> {
        void** result_;
    public:
        explicit FirstCollector(void** res) : result_(res) {
            (void)res;
        }

        void* emit(T value, Continuation<void*>*) override {
            *result_ = new T(std::move(value));
            throw internal::AbortFlowException(this);
        }
    };

    FirstCollector collector(&result);
    try {
        flow->collect(&collector, continuation);
    } catch (internal::AbortFlowException& e) {
        e.check_ownership(&collector);
    }

    if (result == &internal::NULL_VALUE()) {
        throw NoSuchElementException("Expected at least one element");
    }
    return result;
}

/**
 * The terminal operator that returns the first element emitted by the flow matching the given [predicate]
 * and then cancels flow's collection. Throws NoSuchElementException if the flow has not contained matching elements.
 *
 * Transliterated from: kotlinx-coroutines-core/common/src/flow/terminal/Reduce.kt:100-112
 */
template<typename T>
inline void* first(
    std::shared_ptr<Flow<T>> flow,
    std::function<bool(const T&)> predicate,
    Continuation<void*>* continuation) {
    void* result = &internal::NULL_VALUE();

    class FirstPredicateCollector : public FlowCollector<T> {
        void** result_;
        const std::function<bool(const T&)>& predicate_;
    public:
        FirstPredicateCollector(void** res, const std::function<bool(const T&)>& pred)
            : result_(res), predicate_(pred) {
            (void)res;
            (void)pred;
        }

        void* emit(T value, Continuation<void*>*) override {
            if (predicate_(value)) {
                *result_ = new T(std::move(value));
                throw internal::AbortFlowException(this);
            }
            return nullptr;
        }
    };

    FirstPredicateCollector collector(&result, predicate);
    try {
        flow->collect(&collector, continuation);
    } catch (internal::AbortFlowException& e) {
        e.check_ownership(&collector);
    }

    if (result == &internal::NULL_VALUE()) {
        throw NoSuchElementException("Expected at least one element matching the predicate");
    }
    return result;
}

/**
 * The terminal operator that returns the first element emitted by the flow and then cancels flow's collection.
 * Returns nullptr if the flow was empty.
 *
 * Transliterated from: kotlinx-coroutines-core/common/src/flow/terminal/Reduce.kt:118-125
 */
template<typename T>
inline void* first_or_null(std::shared_ptr<Flow<T>> flow, Continuation<void*>* continuation) {
    T* result = nullptr;

    class FirstOrNullCollector : public FlowCollector<T> {
        T** result_;
    public:
        explicit FirstOrNullCollector(T** res) : result_(res) {
            (void)res;
        }

        void* emit(T value, Continuation<void*>*) override {
            *result_ = new T(std::move(value));
            throw internal::AbortFlowException(this);
        }
    };

    FirstOrNullCollector collector(&result);
    try {
        flow->collect(&collector, continuation);
    } catch (internal::AbortFlowException& e) {
        e.check_ownership(&collector);
    }

    return result;
}

/**
 * The terminal operator that returns the first element emitted by the flow matching the given [predicate]
 * and then cancels flow's collection. Returns nullptr if the flow did not contain a matching element.
 *
 * Transliterated from: kotlinx-coroutines-core/common/src/flow/terminal/Reduce.kt:131-142
 */
template<typename T>
inline void* first_or_null(
    std::shared_ptr<Flow<T>> flow,
    std::function<bool(const T&)> predicate,
    Continuation<void*>* continuation) {
    T* result = nullptr;

    class FirstOrNullPredicateCollector : public FlowCollector<T> {
        T** result_;
        const std::function<bool(const T&)>& predicate_;
    public:
        FirstOrNullPredicateCollector(T** res, const std::function<bool(const T&)>& pred)
            : result_(res), predicate_(pred) {
            (void)res;
            (void)pred;
        }

        void* emit(T value, Continuation<void*>*) override {
            if (predicate_(value)) {
                *result_ = new T(std::move(value));
                throw internal::AbortFlowException(this);
            }
            return nullptr;
        }
    };

    FirstOrNullPredicateCollector collector(&result, predicate);
    try {
        flow->collect(&collector, continuation);
    } catch (internal::AbortFlowException& e) {
        e.check_ownership(&collector);
    }

    return result;
}

/**
 * The terminal operator that returns the last element emitted by the flow.
 * Throws NoSuchElementException if the flow was empty.
 *
 * Transliterated from: kotlinx-coroutines-core/common/src/flow/terminal/Reduce.kt:149-156
 */
template<typename T>
inline void* last(std::shared_ptr<Flow<T>> flow, Continuation<void*>* continuation) {
    void* result = &internal::NULL_VALUE();

    class LastCollector : public FlowCollector<T> {
        void** result_;
    public:
        explicit LastCollector(void** res) : result_(res) {
            (void)res;
        }

        void* emit(T value, Continuation<void*>*) override {
            if (*result_ != &internal::NULL_VALUE()) {
                delete static_cast<T*>(*result_);
            }
            *result_ = new T(std::move(value));
            return nullptr;
        }
    };

    LastCollector collector(&result);
    flow->collect(&collector, continuation);

    if (result == &internal::NULL_VALUE()) {
        throw NoSuchElementException("Expected at least one element");
    }
    return result;
}

/**
 * The terminal operator that returns the last element emitted by the flow or nullptr if the flow was empty.
 *
 * Transliterated from: kotlinx-coroutines-core/common/src/flow/terminal/Reduce.kt:161-167
 */
template<typename T>
inline void* last_or_null(std::shared_ptr<Flow<T>> flow, Continuation<void*>* continuation) {
    T* result = nullptr;

    class LastOrNullCollector : public FlowCollector<T> {
        T** result_;
    public:
        explicit LastOrNullCollector(T** res) : result_(res) {
            (void)res;
        }

        void* emit(T value, Continuation<void*>*) override {
            delete *result_;
            *result_ = new T(std::move(value));
            return nullptr;
        }
    };

    LastOrNullCollector collector(&result);
    flow->collect(&collector, continuation);
    return result;
}

} // namespace kotlinx::coroutines::flow
