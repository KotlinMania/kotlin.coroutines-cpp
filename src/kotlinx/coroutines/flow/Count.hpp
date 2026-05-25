#pragma once
/**
 * Transliterated from: kotlinx-coroutines-core/common/src/flow/terminal/Count.kt
 *
 * Kotlin file header (translated):
 *   @file:JvmMultifileClass
 *   @file:JvmName("FlowKt")
 *   package kotlinx.coroutines.flow
 */

#include "kotlinx/coroutines/Continuation.hpp"
#include "kotlinx/coroutines/dsl/Suspend.hpp"
#include "kotlinx/coroutines/flow/Flow.hpp"
#include "kotlinx/coroutines/flow/FlowCollector.hpp"
#include "kotlinx/coroutines/intrinsics/Intrinsics.hpp"

#include <functional>
#include <memory>
#include <utility>

namespace kotlinx::coroutines::flow {

namespace detail {

/** Sink for the bare `count()` overload: increments a counter on every emit. */
template <typename T>
class CountCollector : public FlowCollector<T> {
public:
    explicit CountCollector(int* counter) : counter_(counter) {}

    void* emit(T /*value*/, Continuation<void*>* /*continuation*/) override {
        ++(*counter_);
        return nullptr;
    }

private:
    int* counter_;
};

/** Sink for the `count(predicate)` overload. */
template <typename T>
class CountPredicateCollector : public FlowCollector<T> {
public:
    CountPredicateCollector(
        int* counter,
        std::function<void*(T, Continuation<void*>*)> predicate)
        : counter_(counter), predicate_(std::move(predicate)) {}

    void* emit(T value, Continuation<void*>* continuation) override {
        // Upstream predicate is `suspend (T) -> Boolean`. The C++ predicate signature mirrors
        // the suspend ABI: a non-null result is a boxed boolean payload, and
        // `is_coroutine_suspended` indicates suspension at the predicate site.
        void* predicate_result =
            dsl::suspend(predicate_(std::move(value), continuation));
        if (intrinsics::is_coroutine_suspended(predicate_result)) {
            return intrinsics::get_COROUTINE_SUSPENDED();
        }
        bool matched =
            predicate_result && *static_cast<bool*>(predicate_result);
        if (matched) ++(*counter_);
        return nullptr;
    }

private:
    int* counter_;
    std::function<void*(T, Continuation<void*>*)> predicate_;
};

} // namespace detail

/**
 * Returns the number of elements in this flow.
 *
 * Upstream:
 *   public suspend fun <T> Flow<T>.count(): Int  {
 *       var i = 0
 *       collect { ++i }
 *       return i
 *   }
 *
 * Suspend ABI: returns `void*` per the project convention — a heap-boxed `int` on completion,
 * or `COROUTINE_SUSPENDED` when the underlying `collect` suspends. Callers unbox the boxed
 * int and own the returned pointer.
 */
template <typename T>
[[suspend]]
inline void* count(
    Flow<T>* flow,
    std::shared_ptr<Continuation<void*>> completion) {
    int i = 0;
    detail::CountCollector<T> collector(&i);
    void* collect_result =
        dsl::suspend(flow->collect(&collector, completion.get()));
    if (intrinsics::is_coroutine_suspended(collect_result)) {
        return intrinsics::get_COROUTINE_SUSPENDED();
    }
    return new int(i);
}

/**
 * Returns the number of elements matching the given predicate.
 *
 * Upstream:
 *   public suspend fun <T> Flow<T>.count(predicate: suspend (T) -> Boolean): Int {
 *       var i = 0
 *       collect { value -> if (predicate(value)) ++i }
 *       return i
 *   }
 */
template <typename T>
[[suspend]]
inline void* count(
    Flow<T>* flow,
    std::function<void*(T, Continuation<void*>*)> predicate,
    std::shared_ptr<Continuation<void*>> completion) {
    int i = 0;
    detail::CountPredicateCollector<T> collector(&i, std::move(predicate));
    void* collect_result =
        dsl::suspend(flow->collect(&collector, completion.get()));
    if (intrinsics::is_coroutine_suspended(collect_result)) {
        return intrinsics::get_COROUTINE_SUSPENDED();
    }
    return new int(i);
}

} // namespace kotlinx::coroutines::flow
