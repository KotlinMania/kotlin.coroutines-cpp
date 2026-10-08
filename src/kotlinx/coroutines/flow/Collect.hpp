// port-lint: source kotlinx-coroutines-core/common/src/flow/terminal/Collect.kt
#pragma once
/**
 * @file Collect.hpp
 * @brief Terminal flow operators: collect, launch_in, collect_indexed, collect_latest, emit_all
 *
 * Transliterated from: kotlinx-coroutines-core/common/src/flow/terminal/Collect.kt
 * NOTE(port): KDoc examples retain upstream Kotlin notation.
 */

#include "kotlinx/coroutines/Builders.hpp"
#include "kotlinx/coroutines/Continuation.hpp"
#include "kotlinx/coroutines/CoroutineScope.hpp"
#include "kotlinx/coroutines/Job.hpp"
#include "kotlinx/coroutines/Unit.hpp"
#include "kotlinx/coroutines/dsl/Suspend.hpp"
#include "kotlinx/coroutines/flow/Context.hpp"
#include "kotlinx/coroutines/flow/Flow.hpp"
#include "kotlinx/coroutines/flow/FlowCollector.hpp"
#include "kotlinx/coroutines/flow/Merge.hpp"
#include "kotlinx/coroutines/flow/internal/FlowExceptions.hpp"
#include "kotlinx/coroutines/flow/internal/NopCollector.hpp"
#include "kotlinx/coroutines/flow/internal/ThrowingCollector.hpp"

#include <functional>
#include <memory>
#include <limits>
#include <utility>

namespace kotlinx::coroutines::flow {

namespace internal {

// Transliterated from: kotlinx-coroutines-core/common/src/flow/terminal/Collect.kt:26,55-59,103-113
// NOTE(port): Kotlin GC retains the source receiver and anonymous collectors in
// the suspended caller. Supplied C++ owners remain suspend arguments until completion;
// the Clang frontend constructs the frame and preserves their destruction rules.
// A raw collector supplied by emit_all remains borrowed from its caller.
template <typename T>
[[clang::annotate("suspend")]]
void* collect_with_retained_arguments(std::shared_ptr<Flow<T>> upstream,
    FlowCollector<T>* collector, std::shared_ptr<FlowCollector<T>> collector_owner,
    std::shared_ptr<Continuation<void*>> completion) {
    if (collector_owner) collector = collector_owner.get();
    dsl::suspend(upstream->collect(collector, completion.get()));
    return nullptr;
}

} // namespace internal

/**
 * Terminal flow operator that collects the given flow but ignores all emitted values.
 * If any exception occurs during collect or in the provided flow, this exception is rethrown from this method.
 *
 * It is a shorthand for `collect {}`.
 *
 * This operator is usually used with [onEach], [onCompletion] and [catch] operators to process all emitted values and
 * handle an exception that might occur in the upstream flow or during processing, for example:
 *
 * ```kotlin
 * flow
 *     .onEach { value -> process(value) }
 *     .catch { e -> handleException(e) }
 *     .collect() // trigger collection of the flow
 * ```
 */
// Transliterated from: kotlinx-coroutines-core/common/src/flow/terminal/Collect.kt:26-26
template <typename T>
inline void* collect(std::shared_ptr<Flow<T>> flow, Continuation<void*>* continuation) {
    // NOTE(port): The contravariant Kotlin object is one stable instance per C++ value type.
    static auto nop = std::make_shared<internal::NopCollector<T>>();
    return internal::collect_with_retained_arguments<T>(std::move(flow), nop.get(), nop, kotlinx::coroutines::internal::retain_continuation(continuation));
}

// Transliterated from: kotlinx-coroutines-core/common/src/flow/terminal/Collect.kt:26-26
// NOTE(port): Preserve actual owning completion handles at C++ coroutine-builder boundaries.
template <typename T>
inline void* collect(std::shared_ptr<Flow<T>> flow, std::shared_ptr<Continuation<void*>> continuation) {
    static auto nop = std::make_shared<internal::NopCollector<T>>();
    return internal::collect_with_retained_arguments<T>(std::move(flow), nop.get(), nop, std::move(continuation));
}

/**
 * Terminal flow operator that collects the given flow with a provided [action].
 *
 * Transliterated from: kotlinx-coroutines-core/common/src/flow/terminal/Collect.kt:110-113
 */
template <typename T>
inline void* collect(
    std::shared_ptr<Flow<T>> flow,
    std::function<void*(T, Continuation<void*>*)> action,
    Continuation<void*>* continuation) {
    // Transliterated from: kotlinx-coroutines-core/common/src/flow/terminal/Collect.kt:110-113
    class ActionCollector : public FlowCollector<T> {
        std::function<void*(T, Continuation<void*>*)> action_;
    public:
        // Transliterated from: kotlinx-coroutines-core/common/src/flow/terminal/Collect.kt:110-113
        ActionCollector(std::function<void*(T, Continuation<void*>*)> a) : action_(std::move(a)) {}
        // Transliterated from: kotlinx-coroutines-core/common/src/flow/terminal/Collect.kt:112-112
        void* emit(T value, Continuation<void*>* cont) override {
            return action_(std::move(value), cont);
        }
    };
    auto collector = std::make_shared<ActionCollector>(std::move(action));
    return internal::collect_with_retained_arguments<T>(std::move(flow), collector.get(), collector, kotlinx::coroutines::internal::retain_continuation(continuation));
}

/**
 * Non-suspending action overload of [collect].
 *
 * Transliterated from: kotlinx-coroutines-core/common/src/flow/terminal/Collect.kt:110-113
 */
template <typename T>
inline void* collect(
    std::shared_ptr<Flow<T>> flow,
    std::function<void(T)> action,
    Continuation<void*>* continuation) {
    return collect<T>(
        std::move(flow),
        [action = std::move(action)](T val, Continuation<void*>*) -> void* {
            action(std::move(val));
            return nullptr;
        },
        continuation);
}

/**
 * Terminal flow operator that [launches][launch] the [collection][collect] of the given flow in the [scope].
 * It is a shorthand for `scope.launch { flow.collect() }`.
 *
 * This operator is usually used with [onEach], [onCompletion] and [catch] operators to process all emitted values
 * handle an exception that might occur in the upstream flow or during processing, for example:
 *
 * ```kotlin
 * flow
 *     .onEach { value -> updateUi(value) }
 *     .onCompletion { cause -> updateUi(if (cause == null) "Done" else "Failed") }
 *     .catch { cause -> LOG.error("Exception: $cause") }
 *     .launchIn(uiScope)
 * ```
 *
 * In this example, note that the `job` returned by [launchIn] is not used, and the provided scope takes care of cancellation.
 */
// Transliterated from: kotlinx-coroutines-core/common/src/flow/terminal/Collect.kt:45-47
template <typename T>
inline std::shared_ptr<Job> launch_in(std::shared_ptr<Flow<T>> flow, CoroutineScope* scope) {
    return kotlinx::coroutines::launch(scope, EmptyCoroutineContext::instance(), CoroutineStart::DEFAULT,
        [flow = std::move(flow)](CoroutineScope*, std::shared_ptr<Continuation<void*>> completion) -> void* {
            return collect<T>(flow, std::move(completion)); // tail-call
        });
}

/**
 * Reference overload of [launch_in].
 *
 * Transliterated from: kotlinx-coroutines-core/common/src/flow/terminal/Collect.kt:45-47
 */
template <typename T>
inline std::shared_ptr<Job> launch_in(std::shared_ptr<Flow<T>> flow, CoroutineScope& scope) {
    return launch_in(std::move(flow), &scope);
}

/**
 * Shared pointer overload of [launch_in].
 *
 * Transliterated from: kotlinx-coroutines-core/common/src/flow/terminal/Collect.kt:45-47
 */
template <typename T>
inline std::shared_ptr<Job> launch_in(std::shared_ptr<Flow<T>> flow, const std::shared_ptr<CoroutineScope>& scope) {
    return launch_in(std::move(flow), scope.get());
}

/**
 * Terminal flow operator that collects the given flow with a provided [action] that takes the index of an element (zero-based) and the element.
 * If any exception occurs during collect or in the provided flow, this exception is rethrown from this method.
 *
 * See also [collect] and [withIndex].
 */
// Transliterated from: kotlinx-coroutines-core/common/src/flow/terminal/Collect.kt:55-59
template <typename T>
inline void* collect_indexed(
    std::shared_ptr<Flow<T>> flow,
    std::function<void*(int, T, Continuation<void*>*)> action,
    Continuation<void*>* continuation) {
    // Transliterated from: kotlinx-coroutines-core/common/src/flow/terminal/Collect.kt:55-59
    class IndexedActionCollector : public FlowCollector<T> {
        std::function<void*(int, T, Continuation<void*>*)> action_;
        int index_ = 0;
    public:
        // Transliterated from: kotlinx-coroutines-core/common/src/flow/terminal/Collect.kt:55-59
        IndexedActionCollector(std::function<void*(int, T, Continuation<void*>*)> a)
            : action_(std::move(a)) {}
        // Transliterated from: kotlinx-coroutines-core/common/src/flow/terminal/Collect.kt:58-58
        void* emit(T value, Continuation<void*>* cont) override {
            const int index = index_;
            // NOTE(port): Kotlin Int post-increment wraps; signed C++ overflow is undefined.
            index_ = index_ == std::numeric_limits<int>::max() ? std::numeric_limits<int>::min() : index_ + 1;
            return action_(internal::check_index_overflow(index), std::move(value), cont);
        }
    };
    auto collector = std::make_shared<IndexedActionCollector>(std::move(action));
    return internal::collect_with_retained_arguments<T>(std::move(flow), collector.get(), collector, kotlinx::coroutines::internal::retain_continuation(continuation));
}

/**
 * Non-suspending action overload of [collect_indexed].
 *
 * Transliterated from: kotlinx-coroutines-core/common/src/flow/terminal/Collect.kt:55-59
 */
template <typename T>
inline void* collect_indexed(
    std::shared_ptr<Flow<T>> flow,
    std::function<void(int, T)> action,
    Continuation<void*>* continuation) {
    return collect_indexed<T>(
        std::move(flow),
        [action = std::move(action)](int idx, T val, Continuation<void*>*) -> void* {
            action(idx, std::move(val));
            return nullptr;
        },
        continuation);
}

namespace internal {
// Transliterated from: kotlinx-coroutines-core/common/src/flow/terminal/Collect.kt:82-97
void* collect_latest_impl(std::shared_ptr<Flow<Unit>> mapped, Continuation<void*>* completion);
// Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Merge.kt:211-213
void* map_latest_action(FlowCollector<Unit>* collector,
                       std::function<void*(Continuation<void*>*)> action,
                       Continuation<void*>* completion);
}

/**
 * Terminal flow operator that collects the given flow with a provided [action].
 * The crucial difference from [collect] is that when the original flow emits a new value
 * then the [action] block for the previous value is cancelled.
 *
 * It can be demonstrated by the following example:
 *
 * ```kotlin
 * flow {
 *     emit(1)
 *     delay(50)
 *     emit(2)
 * }.collectLatest { value ->
 *     println("Collecting $value")
 *     delay(100) // Emulate work
 *     println("$value collected")
 * }
 * ```
 *
 * prints "Collecting 1, Collecting 2, 2 collected"
 */
// Transliterated from: kotlinx-coroutines-core/common/src/flow/terminal/Collect.kt:82-97
template <typename T>
inline void* collect_latest(
    std::shared_ptr<Flow<T>> flow,
    std::function<void*(T, Continuation<void*>*)> action,
    Continuation<void*>* continuation) {
    auto mapped = transform_latest<T, Unit>(
        std::move(flow),
        [action = std::move(action)](FlowCollector<Unit>* collector, T val, Continuation<void*>* cont) -> void* {
            return internal::map_latest_action(collector,
                [action, value = std::move(val)](Continuation<void*>* current) mutable -> void* {
                    return action(std::move(value), current);
                }, cont);
        });
    return internal::collect_latest_impl(buffer<Unit>(std::move(mapped), 0), continuation);
}

/**
 * Non-suspending action overload of [collect_latest].
 *
 * Transliterated from: kotlinx-coroutines-core/common/src/flow/terminal/Collect.kt:82-97
 */
template <typename T>
inline void* collect_latest(
    std::shared_ptr<Flow<T>> flow,
    std::function<void(T)> action,
    Continuation<void*>* continuation) {
    return collect_latest<T>(
        std::move(flow),
        [action = std::move(action)](T val, Continuation<void*>*) -> void* {
            action(std::move(val));
            return nullptr;
        },
        continuation);
}

/**
 * Collects all the values from the given [flow] and emits them to the collector.
 * It is a shorthand for `flow.collect { value -> emit(value) }`.
 */
// Transliterated from: kotlinx-coroutines-core/common/src/flow/terminal/Collect.kt:103-106
template <typename T>
inline void* emit_all(FlowCollector<T>* collector, Flow<T>* flow, Continuation<void*>* continuation) {
    ensure_active(collector);
    return flow->collect(collector, continuation);
}

/**
 * Shared pointer overload of [emit_all].
 *
 * Transliterated from: kotlinx-coroutines-core/common/src/flow/terminal/Collect.kt:103-106
 */
template <typename T>
inline void* emit_all(FlowCollector<T>* collector, std::shared_ptr<Flow<T>> flow, Continuation<void*>* continuation) {
    ensure_active(collector);
    return internal::collect_with_retained_arguments<T>(std::move(flow), collector, nullptr, kotlinx::coroutines::internal::retain_continuation(continuation));
}

} // namespace kotlinx::coroutines::flow
