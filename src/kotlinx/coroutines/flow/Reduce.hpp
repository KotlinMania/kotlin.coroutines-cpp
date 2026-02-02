#pragma once
// port-lint: source flow/terminal/Reduce.kt
/**
 * @file Reduce.hpp
 * @brief Terminal flow operators for reduction: reduce, fold, first, last, single
 *
 * Transliterated from: kotlinx-coroutines-core/common/src/flow/terminal/Reduce.kt
 */

#include "kotlinx/coroutines/flow/Flow.hpp"
#include "kotlinx/coroutines/flow/FlowCollector.hpp"
#include "kotlinx/coroutines/flow/internal/NullSurrogate.hpp"
#include "kotlinx/coroutines/Continuation.hpp"
#include <functional>
#include <stdexcept>
#include <memory>

namespace kotlinx {
namespace coroutines {
namespace flow {

/**
 * Accumulates value starting with the first element and applying [operation] to current accumulator value and each element.
 * Throws std::out_of_range if flow was empty.
 *
 * Transliterated from:
 * public suspend fun <S, T : S> Flow<T>.reduce(operation: suspend (accumulator: S, value: T) -> S): S
 */
template<typename T, typename S = T>
void* reduce(
    std::shared_ptr<Flow<T>> flow,
    std::function<S(S, T)> operation,
    Continuation<void*>* continuation
) {
    void* accumulator = &internal::NULL_VALUE();

    class ReduceCollector : public FlowCollector<T> {
        void** accumulator_;
        std::function<S(S, T)> operation_;
    public:
        ReduceCollector(void** acc, std::function<S(S, T)> op)
            : accumulator_(acc), operation_(std::move(op)) {}

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
        throw std::out_of_range("Empty flow can't be reduced");
    }
    return accumulator;
}

/**
 * Accumulates value starting with [initial] value and applying [operation] current accumulator value and each element.
 *
 * Transliterated from:
 * public suspend inline fun <T, R> Flow<T>.fold(
 *     initial: R,
 *     crossinline operation: suspend (acc: R, value: T) -> R
 * ): R
 */
template<typename T, typename R>
void* fold(
    std::shared_ptr<Flow<T>> flow,
    R initial,
    std::function<R(R, T)> operation,
    Continuation<void*>* continuation
) {
    R* accumulator = new R(std::move(initial));

    class FoldCollector : public FlowCollector<T> {
        R* accumulator_;
        std::function<R(R, T)> operation_;
    public:
        FoldCollector(R* acc, std::function<R(R, T)> op)
            : accumulator_(acc), operation_(std::move(op)) {}

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
 * The terminal operator that awaits for one and only one value to be emitted.
 * Throws std::out_of_range for empty flow and std::invalid_argument for flow
 * that contains more than one element.
 *
 * Transliterated from:
 * public suspend fun <T> Flow<T>.single(): T
 */
template<typename T>
void* single(std::shared_ptr<Flow<T>> flow, Continuation<void*>* continuation) {
    void* result = &internal::NULL_VALUE();

    class SingleCollector : public FlowCollector<T> {
        void** result_;
    public:
        explicit SingleCollector(void** res) : result_(res) {}

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
        throw std::out_of_range("Flow is empty");
    }
    return result;
}

/**
 * The terminal operator that awaits for one and only one value to be emitted.
 * Returns the single value or nullptr, if the flow was empty or emitted more than one value.
 *
 * Transliterated from:
 * public suspend fun <T> Flow<T>.singleOrNull(): T?
 */
template<typename T>
void* single_or_null(std::shared_ptr<Flow<T>> flow, Continuation<void*>* continuation) {
    void* result = &internal::NULL_VALUE();
    bool has_multiple = false;

    class SingleOrNullCollector : public FlowCollector<T> {
        void** result_;
        bool* has_multiple_;
    public:
        SingleOrNullCollector(void** res, bool* mult)
            : result_(res), has_multiple_(mult) {}

        void* emit(T value, Continuation<void*>*) override {
            if (*result_ == &internal::NULL_VALUE()) {
                *result_ = new T(std::move(value));
            } else {
                *has_multiple_ = true;
            }
            return nullptr;
        }
    };

    SingleOrNullCollector collector(&result, &has_multiple);
    flow->collect(&collector, continuation);

    if (has_multiple || result == &internal::NULL_VALUE()) {
        return nullptr;
    }
    return result;
}

/**
 * The terminal operator that returns the first element emitted by the flow and then cancels flow's collection.
 * Throws std::out_of_range if the flow was empty.
 *
 * Transliterated from:
 * public suspend fun <T> Flow<T>.first(): T
 */
template<typename T>
void* first(std::shared_ptr<Flow<T>> flow, Continuation<void*>* continuation) {
    void* result = &internal::NULL_VALUE();

    class FirstCollector : public FlowCollector<T> {
        void** result_;
    public:
        explicit FirstCollector(void** res) : result_(res) {}

        void* emit(T value, Continuation<void*>*) override {
            *result_ = new T(std::move(value));
            throw AbortFlowException();  // Cancel collection after first element
        }
    };

    try {
        FirstCollector collector(&result);
        flow->collect(&collector, continuation);
    } catch (const AbortFlowException&) {
        // Expected - flow was aborted after first element
    }

    if (result == &internal::NULL_VALUE()) {
        throw std::out_of_range("Expected at least one element");
    }
    return result;
}

/**
 * The terminal operator that returns the first element emitted by the flow and then cancels flow's collection.
 * Returns nullptr if the flow was empty.
 *
 * Transliterated from:
 * public suspend fun <T> Flow<T>.firstOrNull(): T?
 */
template<typename T>
void* first_or_null(std::shared_ptr<Flow<T>> flow, Continuation<void*>* continuation) {
    T* result = nullptr;

    class FirstOrNullCollector : public FlowCollector<T> {
        T** result_;
    public:
        explicit FirstOrNullCollector(T** res) : result_(res) {}

        void* emit(T value, Continuation<void*>*) override {
            *result_ = new T(std::move(value));
            throw AbortFlowException();
        }
    };

    try {
        FirstOrNullCollector collector(&result);
        flow->collect(&collector, continuation);
    } catch (const AbortFlowException&) {
        // Expected
    }

    return result;
}

/**
 * The terminal operator that returns the last element emitted by the flow.
 * Throws std::out_of_range if the flow was empty.
 *
 * Transliterated from:
 * public suspend fun <T> Flow<T>.last(): T
 */
template<typename T>
void* last(std::shared_ptr<Flow<T>> flow, Continuation<void*>* continuation) {
    void* result = &internal::NULL_VALUE();

    class LastCollector : public FlowCollector<T> {
        void** result_;
    public:
        explicit LastCollector(void** res) : result_(res) {}

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
        throw std::out_of_range("Expected at least one element");
    }
    return result;
}

/**
 * The terminal operator that returns the last element emitted by the flow or nullptr if the flow was empty.
 *
 * Transliterated from:
 * public suspend fun <T> Flow<T>.lastOrNull(): T?
 */
template<typename T>
void* last_or_null(std::shared_ptr<Flow<T>> flow, Continuation<void*>* continuation) {
    T* result = nullptr;

    class LastOrNullCollector : public FlowCollector<T> {
        T** result_;
    public:
        explicit LastOrNullCollector(T** res) : result_(res) {}

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

// Exception used to abort flow collection early
class AbortFlowException : public std::exception {
public:
    const char* what() const noexcept override { return "AbortFlowException"; }
};

} // namespace flow
} // namespace coroutines
} // namespace kotlinx
