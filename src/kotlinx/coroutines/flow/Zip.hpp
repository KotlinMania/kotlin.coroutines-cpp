#pragma once
/**
 * Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Zip.kt
 *
 * Kotlin file header (translated):
 *   @file:JvmMultifileClass
 *   @file:JvmName("FlowKt")
 *   package kotlinx.coroutines.flow
 */

#include "kotlinx/coroutines/Continuation.hpp"
#include "kotlinx/coroutines/dsl/Suspend.hpp"
#include "kotlinx/coroutines/flow/Flow.hpp"
#include "kotlinx/coroutines/flow/FlowBuilders.hpp"
#include "kotlinx/coroutines/flow/FlowCollector.hpp"
#include "kotlinx/coroutines/flow/internal/Combine.hpp"
#include "kotlinx/coroutines/flow/internal/Zip.hpp"
#include "kotlinx/coroutines/intrinsics/Intrinsics.hpp"

#include <any>
#include <functional>
#include <memory>
#include <utility>
#include <vector>

namespace kotlinx::coroutines::flow {

/**
 * Zips values from the current flow (this) with [other] using provided [transform].
 *
 * The resulting flow completes as soon as one of the flows completes and cancels the
 * remaining one.
 *
 * Upstream:
 *   public fun <T1, T2, R> Flow<T1>.zip(other: Flow<T2>, transform: suspend (T1, T2) -> R): Flow<R> =
 *       zipImpl(this, other, transform)
 */
template <typename T1, typename T2, typename R>
inline std::shared_ptr<Flow<R>> zip(
    std::shared_ptr<Flow<T1>> first,
    std::shared_ptr<Flow<T2>> other,
    std::function<R(T1, T2)> transform) {
    return internal::zip_impl<T1, T2, R>(std::move(first), std::move(other), std::move(transform));
}

/**
 * Upstream:
 *   @JvmName("flowCombine")
 *   public fun <T1, T2, R> Flow<T1>.combine(flow: Flow<T2>, transform: suspend (T1, T2) -> R): Flow<R> = flow {
 *       combineInternal(arrayOf(this@combine, flow), nullArrayFactory(),
 *           { emit(transform(it[0] as T1, it[1] as T2)) })
 *   }
 */
template <typename T1, typename T2, typename R>
inline std::shared_ptr<Flow<R>> combine(
    std::shared_ptr<Flow<T1>> first,
    std::shared_ptr<Flow<T2>> second,
    std::function<R(T1, T2)> transform) {
    return flow_builder<R>([first, second, transform](
                               FlowCollector<R>* collector,
                               std::shared_ptr<Continuation<void*>> completion) -> void* {
        std::vector<std::shared_ptr<Flow<std::any>>> sources;
        sources.push_back(internal::as_any_flow<T1>(first));
        sources.push_back(internal::as_any_flow<T2>(second));
        return internal::combine_internal<R>(
            collector, sources,
            [transform](FlowCollector<R>* sink,
                        const std::vector<std::any>& values,
                        Continuation<void*>* cont) {
                R r = transform(std::any_cast<T1>(values[0]), std::any_cast<T2>(values[1]));
                return sink->emit(std::move(r), cont);
            },
            completion);
    });
}

/**
 * Upstream:
 *   public fun <T1, T2, R> combine(flow: Flow<T1>, flow2: Flow<T2>, transform: suspend (T1, T2) -> R): Flow<R> =
 *       flow.combine(flow2, transform)
 */
template <typename T1, typename T2, typename R>
inline std::shared_ptr<Flow<R>> combine_free(
    std::shared_ptr<Flow<T1>> first,
    std::shared_ptr<Flow<T2>> second,
    std::function<R(T1, T2)> transform) {
    return combine<T1, T2, R>(std::move(first), std::move(second), std::move(transform));
}

/**
 * Upstream:
 *   @JvmName("flowCombineTransform")
 *   public fun <T1, T2, R> Flow<T1>.combineTransform(
 *       flow: Flow<T2>,
 *       transform: suspend FlowCollector<R>.(T1, T2) -> Unit
 *   ): Flow<R> = combineTransformUnsafe(this, flow) { args: Array<*> ->
 *       transform(args[0] as T1, args[1] as T2)
 *   }
 */
template <typename T1, typename T2, typename R>
inline std::shared_ptr<Flow<R>> combine_transform(
    std::shared_ptr<Flow<T1>> first,
    std::shared_ptr<Flow<T2>> second,
    std::function<void*(FlowCollector<R>*, T1, T2, Continuation<void*>*)> transform) {
    std::vector<std::shared_ptr<Flow<std::any>>> sources;
    sources.push_back(internal::as_any_flow<T1>(first));
    sources.push_back(internal::as_any_flow<T2>(second));
    return internal::combine_transform_unsafe<R>(
        std::move(sources),
        [transform](FlowCollector<R>* sink,
                    const std::vector<std::any>& values,
                    Continuation<void*>* cont) {
            return transform(sink,
                             std::any_cast<T1>(values[0]),
                             std::any_cast<T2>(values[1]),
                             cont);
        });
}

/**
 * Upstream:
 *   public fun <T1, T2, T3, R> combine(flow, flow2, flow3, transform: suspend (T1, T2, T3) -> R): Flow<R> =
 *       combineUnsafe(flow, flow2, flow3) { args: Array<*> ->
 *           transform(args[0] as T1, args[1] as T2, args[2] as T3)
 *       }
 */
template <typename T1, typename T2, typename T3, typename R>
inline std::shared_ptr<Flow<R>> combine(
    std::shared_ptr<Flow<T1>> flow1,
    std::shared_ptr<Flow<T2>> flow2,
    std::shared_ptr<Flow<T3>> flow3,
    std::function<R(T1, T2, T3)> transform) {
    std::vector<std::shared_ptr<Flow<std::any>>> sources;
    sources.push_back(internal::as_any_flow<T1>(flow1));
    sources.push_back(internal::as_any_flow<T2>(flow2));
    sources.push_back(internal::as_any_flow<T3>(flow3));
    return internal::combine_unsafe<R>(
        std::move(sources),
        [transform](const std::vector<std::any>& values) {
            return transform(std::any_cast<T1>(values[0]),
                             std::any_cast<T2>(values[1]),
                             std::any_cast<T3>(values[2]));
        });
}

/**
 * Upstream:
 *   public fun <T1, T2, T3, T4, R> combine(flow, flow2, flow3, flow4, transform): Flow<R> =
 *       combineUnsafe(flow, flow2, flow3, flow4) { args -> transform(args[0..3]) }
 */
template <typename T1, typename T2, typename T3, typename T4, typename R>
inline std::shared_ptr<Flow<R>> combine(
    std::shared_ptr<Flow<T1>> flow1,
    std::shared_ptr<Flow<T2>> flow2,
    std::shared_ptr<Flow<T3>> flow3,
    std::shared_ptr<Flow<T4>> flow4,
    std::function<R(T1, T2, T3, T4)> transform) {
    std::vector<std::shared_ptr<Flow<std::any>>> sources;
    sources.push_back(internal::as_any_flow<T1>(flow1));
    sources.push_back(internal::as_any_flow<T2>(flow2));
    sources.push_back(internal::as_any_flow<T3>(flow3));
    sources.push_back(internal::as_any_flow<T4>(flow4));
    return internal::combine_unsafe<R>(
        std::move(sources),
        [transform](const std::vector<std::any>& values) {
            return transform(std::any_cast<T1>(values[0]),
                             std::any_cast<T2>(values[1]),
                             std::any_cast<T3>(values[2]),
                             std::any_cast<T4>(values[3]));
        });
}

/**
 * Vector-based N-arity combine.
 *
 * Upstream:
 *   public inline fun <reified T, R> combine(
 *       vararg flows: Flow<T>,
 *       crossinline transform: suspend (Array<T>) -> R
 *   ): Flow<R> = combineUnsafe(*flows) { args -> transform(args as Array<T>) }
 */
template <typename T, typename R>
inline std::shared_ptr<Flow<R>> combine_all(
    std::vector<std::shared_ptr<Flow<T>>> flows,
    std::function<R(std::vector<T>)> transform) {
    std::vector<std::shared_ptr<Flow<std::any>>> sources;
    sources.reserve(flows.size());
    for (auto& f : flows) sources.push_back(internal::as_any_flow<T>(f));
    return internal::combine_unsafe<R>(
        std::move(sources),
        [transform](const std::vector<std::any>& values) {
            std::vector<T> typed;
            typed.reserve(values.size());
            for (const auto& v : values) typed.push_back(std::any_cast<T>(v));
            return transform(std::move(typed));
        });
}

} // namespace kotlinx::coroutines::flow
