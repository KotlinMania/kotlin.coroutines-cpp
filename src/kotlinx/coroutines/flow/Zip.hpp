#pragma once
// port-lint: source kotlinx-coroutines-core/common/src/flow/operators/Zip.kt
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
// Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Zip.kt:327-327
template <typename T1, typename T2, typename R>
inline std::shared_ptr<Flow<R>> zip(
    std::shared_ptr<Flow<T1>> first,
    std::shared_ptr<Flow<T2>> other,
    std::function<R(T1, T2)> transform) {
    return internal::zip_impl<T1, T2, R>(std::move(first), std::move(other), std::move(transform));
}

// Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Zip.kt:327-327
// NOTE(port): The suspend transform returns an owning R box through the Continuation ABI.
template <typename T1, typename T2, typename R>
inline std::shared_ptr<Flow<R>> zip(
    std::shared_ptr<Flow<T1>> first, std::shared_ptr<Flow<T2>> other,
    std::function<void*(T1, T2, Continuation<void*>*)> transform) {
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
// Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Zip.kt:28-30
template <typename T1, typename T2, typename R>
inline std::shared_ptr<Flow<R>> combine(
    std::shared_ptr<Flow<T1>> first, std::shared_ptr<Flow<T2>> second,
    std::function<void*(T1, T2, Continuation<void*>*)> transform) {
    std::vector<std::shared_ptr<Flow<std::any>>> sources{
        internal::as_any_flow<T1>(std::move(first)), internal::as_any_flow<T2>(std::move(second))};
    return internal::combine_unsafe<R>(std::move(sources),
        [transform = std::move(transform)](const std::vector<std::any>& values,
                                         Continuation<void*>* continuation) {
            return transform(std::any_cast<T1>(values[0]), std::any_cast<T2>(values[1]), continuation);
        });
}

// Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Zip.kt:28-30
// NOTE(port): Ordinary C++ transforms bind to the source suspend transform ABI.
template <typename T1, typename T2, typename R>
inline std::shared_ptr<Flow<R>> combine(
    std::shared_ptr<Flow<T1>> first, std::shared_ptr<Flow<T2>> second,
    std::function<R(T1, T2)> transform) {
    return combine<T1, T2, R>(std::move(first), std::move(second),
        std::function<void*(T1, T2, Continuation<void*>*)>(
            [transform = std::move(transform)](T1 left, T2 right, Continuation<void*>*) -> void* {
                if constexpr (std::is_same_v<R, Unit>) {
                    transform(std::move(left), std::move(right));
                    return nullptr;
                } else return new R(transform(std::move(left), std::move(right)));
            }));
}

/**
 * Upstream:
 *   public fun <T1, T2, R> combine(flow: Flow<T1>, flow2: Flow<T2>, transform: suspend (T1, T2) -> R): Flow<R> =
 *       flow.combine(flow2, transform)
 */
// Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Zip.kt:47-48
template <typename T1, typename T2, typename R>
inline std::shared_ptr<Flow<R>> combine_free(
    std::shared_ptr<Flow<T1>> first, std::shared_ptr<Flow<T2>> second,
    std::function<void*(T1, T2, Continuation<void*>*)> transform) {
    return combine<T1, T2, R>(std::move(first), std::move(second), std::move(transform));
}

// Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Zip.kt:47-48
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
// Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Zip.kt:77-85,95-104
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
// Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Zip.kt:110-121
template <typename T1, typename T2, typename T3, typename R>
inline std::shared_ptr<Flow<R>> combine(
    std::shared_ptr<Flow<T1>> flow1,
    std::shared_ptr<Flow<T2>> flow2,
    std::shared_ptr<Flow<T3>> flow3,
    std::function<void*(T1, T2, T3, Continuation<void*>*)> transform) {
    std::vector<std::shared_ptr<Flow<std::any>>> sources{
        internal::as_any_flow<T1>(std::move(flow1)),
        internal::as_any_flow<T2>(std::move(flow2)),
        internal::as_any_flow<T3>(std::move(flow3))};
    return internal::combine_unsafe<R>(std::move(sources),
        [transform = std::move(transform)](const std::vector<std::any>& values,
                                         Continuation<void*>* continuation) {
            return transform(std::any_cast<T1>(values[0]),
                             std::any_cast<T2>(values[1]),
                             std::any_cast<T3>(values[2]), continuation);
        });
}

// Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Zip.kt:110-121
// NOTE(port): Ordinary C++ transform binding.
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
 * Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Zip.kt:129-140
 * Upstream:
 *   public fun <T1, T2, T3, R> combineTransform(flow, flow2, flow3, transform): Flow<R>
 */
template <typename T1, typename T2, typename T3, typename R>
inline std::shared_ptr<Flow<R>> combine_transform(
    std::shared_ptr<Flow<T1>> flow1,
    std::shared_ptr<Flow<T2>> flow2,
    std::shared_ptr<Flow<T3>> flow3,
    std::function<void*(FlowCollector<R>*, T1, T2, T3, Continuation<void*>*)> transform) {
    std::vector<std::shared_ptr<Flow<std::any>>> sources;
    sources.push_back(internal::as_any_flow<T1>(flow1));
    sources.push_back(internal::as_any_flow<T2>(flow2));
    sources.push_back(internal::as_any_flow<T3>(flow3));
    return internal::combine_transform_unsafe<R>(
        std::move(sources),
        [transform](FlowCollector<R>* sink,
                    const std::vector<std::any>& values,
                    Continuation<void*>* cont) {
            return transform(sink,
                             std::any_cast<T1>(values[0]),
                             std::any_cast<T2>(values[1]),
                             std::any_cast<T3>(values[2]),
                             cont);
        });
}

/**
 * Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Zip.kt:146-159
 * Upstream:
 *   public fun <T1, T2, T3, T4, R> combine(flow, flow2, flow3, flow4, transform): Flow<R>
 */
// Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Zip.kt:146-159
template <typename T1, typename T2, typename T3, typename T4, typename R>
inline std::shared_ptr<Flow<R>> combine(
    std::shared_ptr<Flow<T1>> flow1,
    std::shared_ptr<Flow<T2>> flow2,
    std::shared_ptr<Flow<T3>> flow3,
    std::shared_ptr<Flow<T4>> flow4,
    std::function<void*(T1, T2, T3, T4, Continuation<void*>*)> transform) {
    std::vector<std::shared_ptr<Flow<std::any>>> sources{
        internal::as_any_flow<T1>(std::move(flow1)),
        internal::as_any_flow<T2>(std::move(flow2)),
        internal::as_any_flow<T3>(std::move(flow3)),
        internal::as_any_flow<T4>(std::move(flow4))};
    return internal::combine_unsafe<R>(std::move(sources),
        [transform = std::move(transform)](const std::vector<std::any>& values,
                                         Continuation<void*>* continuation) {
            return transform(std::any_cast<T1>(values[0]),
                             std::any_cast<T2>(values[1]),
                             std::any_cast<T3>(values[2]),
                             std::any_cast<T4>(values[3]), continuation);
        });
}

// Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Zip.kt:146-159
// NOTE(port): Ordinary C++ transform binding.
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
 * Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Zip.kt:167-180
 * Upstream:
 *   public fun <T1, T2, T3, T4, R> combineTransform(flow, flow2, flow3, flow4, transform): Flow<R>
 */
template <typename T1, typename T2, typename T3, typename T4, typename R>
inline std::shared_ptr<Flow<R>> combine_transform(
    std::shared_ptr<Flow<T1>> flow1,
    std::shared_ptr<Flow<T2>> flow2,
    std::shared_ptr<Flow<T3>> flow3,
    std::shared_ptr<Flow<T4>> flow4,
    std::function<void*(FlowCollector<R>*, T1, T2, T3, T4, Continuation<void*>*)> transform) {
    std::vector<std::shared_ptr<Flow<std::any>>> sources;
    sources.push_back(internal::as_any_flow<T1>(flow1));
    sources.push_back(internal::as_any_flow<T2>(flow2));
    sources.push_back(internal::as_any_flow<T3>(flow3));
    sources.push_back(internal::as_any_flow<T4>(flow4));
    return internal::combine_transform_unsafe<R>(
        std::move(sources),
        [transform](FlowCollector<R>* sink,
                    const std::vector<std::any>& values,
                    Continuation<void*>* cont) {
            return transform(sink,
                             std::any_cast<T1>(values[0]),
                             std::any_cast<T2>(values[1]),
                             std::any_cast<T3>(values[2]),
                             std::any_cast<T4>(values[3]),
                             cont);
        });
}

/**
 * Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Zip.kt:186-201
 * Upstream:
 *   public fun <T1, T2, T3, T4, T5, R> combine(flow, flow2, flow3, flow4, flow5, transform): Flow<R>
 */
// Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Zip.kt:186-201
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename R>
inline std::shared_ptr<Flow<R>> combine(
    std::shared_ptr<Flow<T1>> flow1,
    std::shared_ptr<Flow<T2>> flow2,
    std::shared_ptr<Flow<T3>> flow3,
    std::shared_ptr<Flow<T4>> flow4,
    std::shared_ptr<Flow<T5>> flow5,
    std::function<void*(T1, T2, T3, T4, T5, Continuation<void*>*)> transform) {
    std::vector<std::shared_ptr<Flow<std::any>>> sources{
        internal::as_any_flow<T1>(std::move(flow1)),
        internal::as_any_flow<T2>(std::move(flow2)),
        internal::as_any_flow<T3>(std::move(flow3)),
        internal::as_any_flow<T4>(std::move(flow4)),
        internal::as_any_flow<T5>(std::move(flow5))};
    return internal::combine_unsafe<R>(std::move(sources),
        [transform = std::move(transform)](const std::vector<std::any>& values,
                                         Continuation<void*>* continuation) {
            return transform(std::any_cast<T1>(values[0]),
                             std::any_cast<T2>(values[1]),
                             std::any_cast<T3>(values[2]),
                             std::any_cast<T4>(values[3]),
                             std::any_cast<T5>(values[4]), continuation);
        });
}

// Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Zip.kt:186-201
// NOTE(port): Ordinary C++ transform binding.
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename R>
inline std::shared_ptr<Flow<R>> combine(
    std::shared_ptr<Flow<T1>> flow1,
    std::shared_ptr<Flow<T2>> flow2,
    std::shared_ptr<Flow<T3>> flow3,
    std::shared_ptr<Flow<T4>> flow4,
    std::shared_ptr<Flow<T5>> flow5,
    std::function<R(T1, T2, T3, T4, T5)> transform) {
    std::vector<std::shared_ptr<Flow<std::any>>> sources;
    sources.push_back(internal::as_any_flow<T1>(flow1));
    sources.push_back(internal::as_any_flow<T2>(flow2));
    sources.push_back(internal::as_any_flow<T3>(flow3));
    sources.push_back(internal::as_any_flow<T4>(flow4));
    sources.push_back(internal::as_any_flow<T5>(flow5));
    return internal::combine_unsafe<R>(
        std::move(sources),
        [transform](const std::vector<std::any>& values) {
            return transform(std::any_cast<T1>(values[0]),
                             std::any_cast<T2>(values[1]),
                             std::any_cast<T3>(values[2]),
                             std::any_cast<T4>(values[3]),
                             std::any_cast<T5>(values[4]));
        });
}

/**
 * Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Zip.kt:209-224
 * Upstream:
 *   public fun <T1, T2, T3, T4, T5, R> combineTransform(flow, flow2, flow3, flow4, flow5, transform): Flow<R>
 */
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename R>
inline std::shared_ptr<Flow<R>> combine_transform(
    std::shared_ptr<Flow<T1>> flow1,
    std::shared_ptr<Flow<T2>> flow2,
    std::shared_ptr<Flow<T3>> flow3,
    std::shared_ptr<Flow<T4>> flow4,
    std::shared_ptr<Flow<T5>> flow5,
    std::function<void*(FlowCollector<R>*, T1, T2, T3, T4, T5, Continuation<void*>*)> transform) {
    std::vector<std::shared_ptr<Flow<std::any>>> sources;
    sources.push_back(internal::as_any_flow<T1>(flow1));
    sources.push_back(internal::as_any_flow<T2>(flow2));
    sources.push_back(internal::as_any_flow<T3>(flow3));
    sources.push_back(internal::as_any_flow<T4>(flow4));
    sources.push_back(internal::as_any_flow<T5>(flow5));
    return internal::combine_transform_unsafe<R>(
        std::move(sources),
        [transform](FlowCollector<R>* sink,
                    const std::vector<std::any>& values,
                    Continuation<void*>* cont) {
            return transform(sink,
                             std::any_cast<T1>(values[0]),
                             std::any_cast<T2>(values[1]),
                             std::any_cast<T3>(values[2]),
                             std::any_cast<T4>(values[3]),
                             std::any_cast<T5>(values[4]),
                             cont);
        });
}

// Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Zip.kt:229-235,280-292
// NOTE(port): The vector represents the source vararg/Iterable flowArray.
template <typename T, typename R>
inline std::shared_ptr<Flow<R>> combine(
    std::vector<std::shared_ptr<Flow<T>>> flows,
    std::function<void*(std::vector<T>, Continuation<void*>*)> transform) {
    std::vector<std::shared_ptr<Flow<std::any>>> flow_array;
    flow_array.reserve(flows.size());
    for (auto& source : flows) flow_array.push_back(internal::as_any_flow<T>(std::move(source)));
    return internal::unsafe_flow<R>([flow_array = std::move(flow_array), transform = std::move(transform)](
        FlowCollector<R>* collector, Continuation<void*>* completion) {
        return internal::combine_internal<R>(collector, flow_array,
            [size = flow_array.size()] { return new std::vector<std::any>(size); },
            [transform](FlowCollector<R>* sink, const std::vector<std::any>& values,
                        Continuation<void*>* frame) {
                return internal::emit_combine_result(
                    [transform, &values](Continuation<void*>* continuation) {
                        std::vector<T> typed;
                        typed.reserve(values.size());
                        for (const auto& value : values) typed.push_back(std::any_cast<T>(value));
                        return transform(std::move(typed), continuation);
                    },
                    [sink](void* box, Continuation<void*>* continuation) {
                        return internal::emit_transformed_result(sink, box, continuation);
                    },
                    [](void* box) { delete static_cast<R*>(box); }, frame);
            }, completion);
    });
}

// Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Zip.kt:229-235,280-292
// NOTE(port): Ordinary C++ transforms enter the same suspend/copy-factory path.
template <typename T, typename R>
inline std::shared_ptr<Flow<R>> combine(
    std::vector<std::shared_ptr<Flow<T>>> flows,
    std::function<R(std::vector<T>)> transform) {
    return combine<T, R>(std::move(flows),
        std::function<void*(std::vector<T>, Continuation<void*>*)>(
            [transform = std::move(transform)](std::vector<T> values, Continuation<void*>*) -> void* {
                if constexpr (std::is_same_v<R, Unit>) { transform(std::move(values)); return nullptr; }
                else return new R(transform(std::move(values)));
            }));
}

// Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Zip.kt:244-250,302-310
template <typename T, typename R>
inline std::shared_ptr<Flow<R>> combine_transform(
    std::vector<std::shared_ptr<Flow<T>>> flows,
    std::function<void*(FlowCollector<R>*, std::vector<T>, Continuation<void*>*)> transform) {
    std::vector<std::shared_ptr<Flow<std::any>>> flow_array;
    flow_array.reserve(flows.size());
    for (auto& source : flows) flow_array.push_back(internal::as_any_flow<T>(std::move(source)));
    return flow<R>([flow_array = std::move(flow_array), transform = std::move(transform)](
        FlowCollector<R>* collector, Continuation<void*>* completion) {
        return internal::combine_internal<R>(collector, flow_array,
            [size = flow_array.size()] { return new std::vector<std::any>(size); },
            [transform](FlowCollector<R>* sink, const std::vector<std::any>& values,
                        Continuation<void*>* frame) {
                std::vector<T> typed;
                typed.reserve(values.size());
                for (const auto& value : values) typed.push_back(std::any_cast<T>(value));
                return transform(sink, std::move(typed), frame);
            }, completion);
    });
}

// Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Zip.kt:229-235,280-292
// NOTE(port): Preserve the existing C++ spelling as a binding to source combine.
template <typename T, typename R>
inline std::shared_ptr<Flow<R>> combine_all(
    std::vector<std::shared_ptr<Flow<T>>> flows,
    std::function<void*(std::vector<T>, Continuation<void*>*)> transform) {
    return combine<T, R>(std::move(flows), std::move(transform));
}

// Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Zip.kt:229-235,280-292
template <typename T, typename R>
inline std::shared_ptr<Flow<R>> combine_all(
    std::vector<std::shared_ptr<Flow<T>>> flows,
    std::function<R(std::vector<T>)> transform) {
    return combine<T, R>(std::move(flows), std::move(transform));
}

} // namespace kotlinx::coroutines::flow
