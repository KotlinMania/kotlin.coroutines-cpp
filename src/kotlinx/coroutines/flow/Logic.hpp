#pragma once
/**
 * Transliterated from: kotlinx-coroutines-core/common/src/flow/terminal/Logic.kt
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
#include "kotlinx/coroutines/flow/internal/SafeCollector.hpp"
#include "kotlinx/coroutines/intrinsics/Intrinsics.hpp"

#include <functional>
#include <memory>
#include <utility>

namespace kotlinx::coroutines::flow {

namespace detail {

/**
 * Predicate-controlled collector used by `any` / `all`. Stops the outer collection by throwing
 * the upstream-internal `AbortFlowException` when the supplied predicate returns `false`.
 *
 * Mirrors the upstream `collectWhile { ... }` internal helper used by Logic.kt.
 */
template <typename T>
class CollectWhileCollector : public FlowCollector<T> {
public:
    explicit CollectWhileCollector(std::function<bool(T, Continuation<void*>*)> predicate)
        : predicate_(std::move(predicate)) {}

    void* emit(T value, Continuation<void*>* continuation) override {
        bool keep_going = predicate_(std::move(value), continuation);
        if (!keep_going) {
            // Upstream uses `AbortFlowException` to short-circuit; the C++ port mirrors that
            // by throwing the same intent.
            throw internal::AbortFlowException(this);
        }
        return nullptr;
    }

private:
    std::function<bool(T, Continuation<void*>*)> predicate_;
};

} // namespace detail

/**
 * A terminal operator that returns `true` and immediately cancels the flow if at least one
 * element matches the given [predicate]. If the flow does not emit any elements or no element
 * matches the predicate, the function returns `false`.
 *
 * Upstream:
 *   public suspend fun <T> Flow<T>.any(predicate: suspend (T) -> Boolean): Boolean {
 *       var found = false
 *       collectWhile {
 *           val satisfies = predicate(it)
 *           if (satisfies) found = true
 *           !satisfies
 *       }
 *       return found
 *   }
 */
template <typename T>
[[suspend]]
inline bool any(
    Flow<T>* flow,
    std::function<void*(T, Continuation<void*>*)> predicate,
    std::shared_ptr<Continuation<void*>> completion) {
    bool found = false;
    auto sink = detail::CollectWhileCollector<T>(
        [&found, predicate = std::move(predicate)](T value, Continuation<void*>* cont) {
            void* result = dsl::suspend(predicate(std::move(value), cont));
            if (intrinsics::is_coroutine_suspended(result)) return true; // keep going on suspend
            bool satisfies = result && *static_cast<bool*>(result);
            if (satisfies) found = true;
            return !satisfies;
        });
    try {
        dsl::suspend(flow->collect(&sink, completion.get()));
    } catch (const internal::AbortFlowException&) {
        // Upstream short-circuit completed.
    }
    return found;
}

/**
 * A terminal operator that returns `true` if all elements match the given [predicate], or
 * returns `false` and cancels the flow as soon as the first element not matching the
 * predicate is encountered.
 *
 * Upstream:
 *   public suspend fun <T> Flow<T>.all(predicate: suspend (T) -> Boolean): Boolean {
 *       var foundCounterExample = false
 *       collectWhile {
 *           val satisfies = predicate(it)
 *           if (!satisfies) foundCounterExample = true
 *           satisfies
 *       }
 *       return !foundCounterExample
 *   }
 */
template <typename T>
[[suspend]]
inline bool all(
    Flow<T>* flow,
    std::function<void*(T, Continuation<void*>*)> predicate,
    std::shared_ptr<Continuation<void*>> completion) {
    bool found_counter_example = false;
    auto sink = detail::CollectWhileCollector<T>(
        [&found_counter_example, predicate = std::move(predicate)](
            T value, Continuation<void*>* cont) {
            void* result = dsl::suspend(predicate(std::move(value), cont));
            if (intrinsics::is_coroutine_suspended(result)) return true; // keep going on suspend
            bool satisfies = result && *static_cast<bool*>(result);
            if (!satisfies) found_counter_example = true;
            return satisfies;
        });
    try {
        dsl::suspend(flow->collect(&sink, completion.get()));
    } catch (const internal::AbortFlowException&) {
        // Upstream short-circuit completed.
    }
    return !found_counter_example;
}

/**
 * A terminal operator that returns `true` if no elements match the given [predicate].
 *
 * Upstream:
 *   public suspend fun <T> Flow<T>.none(predicate: suspend (T) -> Boolean): Boolean =
 *       !any(predicate)
 */
template <typename T>
[[suspend]]
inline bool none(
    Flow<T>* flow,
    std::function<void*(T, Continuation<void*>*)> predicate,
    std::shared_ptr<Continuation<void*>> completion) {
    return !any<T>(flow, std::move(predicate), std::move(completion));
}

} // namespace kotlinx::coroutines::flow
