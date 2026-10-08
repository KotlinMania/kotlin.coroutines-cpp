#pragma once
// port-lint: source kotlinx-coroutines-core/common/src/flow/operators/Merge.kt
/**
 * @file Merge.hpp
 * @brief Flow merge operators: merge, flatten_merge, transform_latest, map_latest
 *
 * Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Merge.kt
 */

#include "kotlinx/coroutines/flow/Flow.hpp"
#include "kotlinx/coroutines/flow/FlowBuilders.hpp"
#include "kotlinx/coroutines/flow/Transform.hpp"
#include "kotlinx/coroutines/flow/internal/Merge.hpp"
#include "kotlinx/coroutines/internal/SystemProps.hpp"
#include <stdexcept>
#include <string>
#include <limits>

namespace kotlinx {
namespace coroutines {
namespace flow {


using kotlinx::coroutines::channels::Channel;
using kotlinx::coroutines::flow::internal::ChannelFlowMerge;
using kotlinx::coroutines::flow::internal::ChannelLimitedFlowMerge;
using kotlinx::coroutines::flow::internal::ChannelFlowTransformLatest;

// Transliterated from: kotlinx-coroutines-core/common/src/flow/terminal/Collect.kt:110-113
template <typename T>
void* collect(std::shared_ptr<Flow<T>> upstream,
    std::function<void*(T, Continuation<void*>*)> action, Continuation<void*>* completion);
// Transliterated from: kotlinx-coroutines-core/common/src/flow/terminal/Collect.kt:103-106
template <typename T>
void* emit_all(FlowCollector<T>* collector, std::shared_ptr<Flow<T>> upstream,
    Continuation<void*>* completion);

/**
 * Name of the property that defines the value of DEFAULT_CONCURRENCY.
 *
 * Kotlin source: kotlinx-coroutines-core/common/src/flow/operators/Merge.kt
 *   public const val DEFAULT_CONCURRENCY_PROPERTY_NAME: String = "kotlinx.coroutines.flow.defaultConcurrency"
 */
// Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Merge.kt:19
inline constexpr const char* DEFAULT_CONCURRENCY_PROPERTY_NAME = "kotlinx.coroutines.flow.defaultConcurrency";

/**
 * Default concurrency limit used by flatten_merge and flat_map_merge.
 *
 * Kotlin source: kotlinx-coroutines-core/common/src/flow/operators/Merge.kt
 *   public val DEFAULT_CONCURRENCY: Int = systemProp(DEFAULT_CONCURRENCY_PROPERTY_NAME, 16, 1, Int.MAX_VALUE)
 */
// Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Merge.kt:26-30
inline const int DEFAULT_CONCURRENCY = kotlinx::coroutines::internal::system_prop_int(
    DEFAULT_CONCURRENCY_PROPERTY_NAME,
    16,
    1,
    std::numeric_limits<int>::max()
);

/**
 * Flattens the given flow of flows into a single flow in a sequential manner, without interleaving nested flows.
 *
 * Inner flows are collected by this operator *sequentially*.
 *
 */
// Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Merge.kt:78-80
template <typename T>
std::shared_ptr<Flow<T>> flatten_concat(std::shared_ptr<Flow<std::shared_ptr<Flow<T>>>> upstream) {
    return internal::unsafe_flow<T>([upstream = std::move(upstream)](
        FlowCollector<T>* collector, Continuation<void*>* completion) -> void* {
        return collect<std::shared_ptr<Flow<T>>>(upstream,
            std::function<void*(std::shared_ptr<Flow<T>>, Continuation<void*>*)>(
                [collector](std::shared_ptr<Flow<T>> value, Continuation<void*>* continuation) -> void* {
                    return emit_all<T>(collector, std::move(value), continuation);
                }), completion);
    });
}

/**
 * Merges the given flows into a single flow without preserving an order of elements.
 * All flows are merged concurrently, without limit on the number of simultaneously collected flows.
 */
// Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Merge.kt:91-103
template <typename T>
std::shared_ptr<Flow<T>> merge(std::vector<std::shared_ptr<Flow<T>>> flows) {
    return std::make_shared<ChannelLimitedFlowMerge<T>>(flows);
}

/**
 * Merges the given flows into a single flow without preserving an order of elements.
 *
 * Kotlin source: kotlinx-coroutines-core/common/src/flow/operators/Merge.kt
 *   public fun <T> merge(vararg flows: Flow<T>): Flow<T> = flows.asIterable().merge()
 */
// Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Merge.kt:122
template <typename T>
std::shared_ptr<Flow<T>> merge(std::initializer_list<std::shared_ptr<Flow<T>>> flows) {
    return merge<T>(std::vector<std::shared_ptr<Flow<T>>>(flows));
}

/**
 * Flattens the given flow of flows into a single flow with a [concurrency] limit.
 */
// Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Merge.kt:154-157
template <typename T>
std::shared_ptr<Flow<T>> flatten_merge(std::shared_ptr<Flow<std::shared_ptr<Flow<T>>>> upstream, int concurrency = DEFAULT_CONCURRENCY) {
    // Kotlin: require(concurrency > 0) { "Expected positive concurrency level, but had $concurrency" }
    if (concurrency <= 0) {
        throw std::invalid_argument("Expected positive concurrency level, but had " + std::to_string(concurrency));
    }

    // Kotlin: if (concurrency == 1) flattenConcat() else ChannelFlowMerge(this, concurrency)
    if (concurrency == 1) {
        return flatten_concat<T>(upstream);
    }

    return std::make_shared<ChannelFlowMerge<T>>(upstream, concurrency);
}

/**
 * Transforms elements emitted by the original flow by applying transform, that returns another flow,
 * and then concatenating and flattening these flows.
 *
 * Kotlin source: kotlinx-coroutines-core/common/src/flow/operators/Merge.kt
 *   public fun <T, R> Flow<T>.flatMapConcat(transform: suspend (value: T) -> Flow<R>): Flow<R> =
 *       map(transform).flattenConcat()
 */
// Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Merge.kt:42-43
template <typename T, typename R>
std::shared_ptr<Flow<R>> flat_map_concat(
    std::shared_ptr<Flow<T>> upstream,
    std::function<std::shared_ptr<Flow<R>>(T)> transform) {
    return flatten_concat<R>(map<T, std::shared_ptr<Flow<R>>>(std::move(upstream), std::move(transform)));
}

// Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Merge.kt:42-43
// NOTE(port): The transform returns an owned std::shared_ptr<Flow<R>> result box;
// map unboxes and deletes it through its existing Continuation ABI projection.
template <typename T, typename R>
std::shared_ptr<Flow<R>> flat_map_concat(
    std::shared_ptr<Flow<T>> upstream,
    std::function<void*(T, Continuation<void*>*)> transform) {
    return flatten_concat<R>(map<T, std::shared_ptr<Flow<R>>>(std::move(upstream), std::move(transform)));
}

/**
 * Transforms elements by applying transform sequentially, then merges the returned
 * flows with a concurrency limit. It is a shortcut for map(transform).flatten_merge(concurrency).
 *
 * Applications of flow_on, buffer and produce_in after this operator fuse with
 * its concurrent merging so that one configured channel executes the merge.
 * At most concurrency flows are collected at the same time.
 */
// Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Merge.kt:66-70
template <typename T, typename R>
std::shared_ptr<Flow<R>> flat_map_merge(
    std::shared_ptr<Flow<T>> upstream, int concurrency,
    std::function<std::shared_ptr<Flow<R>>(T)> transform) {
    return flatten_merge<R>(map<T, std::shared_ptr<Flow<R>>>(std::move(upstream), std::move(transform)), concurrency);
}

// Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Merge.kt:66-70
template <typename T, typename R>
std::shared_ptr<Flow<R>> flat_map_merge(
    std::shared_ptr<Flow<T>> upstream, int concurrency,
    std::function<void*(T, Continuation<void*>*)> transform) {
    return flatten_merge<R>(map<T, std::shared_ptr<Flow<R>>>(std::move(upstream), std::move(transform)), concurrency);
}

// Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Merge.kt:66-70
template <typename T, typename R>
std::shared_ptr<Flow<R>> flat_map_merge(
    std::shared_ptr<Flow<T>> upstream,
    std::function<std::shared_ptr<Flow<R>>(T)> transform) {
    return flat_map_merge<T, R>(std::move(upstream), DEFAULT_CONCURRENCY, std::move(transform));
}

// Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Merge.kt:66-70
template <typename T, typename R>
std::shared_ptr<Flow<R>> flat_map_merge(
    std::shared_ptr<Flow<T>> upstream,
    std::function<void*(T, Continuation<void*>*)> transform) {
    return flat_map_merge<T, R>(std::move(upstream), DEFAULT_CONCURRENCY, std::move(transform));
}

/**
 * Returns a flow that produces element by [transform] function every time the original flow emits a value.
 * When the original flow emits a new value, the previous `transform` block is cancelled.
 */
template <typename T, typename R>
std::shared_ptr<Flow<R>> transform_latest(std::shared_ptr<Flow<T>> upstream, 
                                        std::function<void*(FlowCollector<R>*, T, Continuation<void*>*)> transform_fn) {
    return std::make_shared<ChannelFlowTransformLatest<T, R>>(transform_fn, upstream);
}

/**
 * Returns a flow that emits elements from the original flow transformed by [transform] function.
 * When the original flow emits a new value, computation of the [transform] block for previous value is cancelled.
 */
template <typename T, typename R>
std::shared_ptr<Flow<R>> map_latest(std::shared_ptr<Flow<T>> upstream, std::function<R(T)> transform_fn) {
    return transform_latest<T, R>(upstream, [transform_fn](FlowCollector<R>* collector, T value, Continuation<void*>* cont) -> void* {
        // emit() returns the Continuation-ABI value: either a real result or
        // COROUTINE_SUSPENDED. transform_latest already drives suspension through `cont`
        // so callers see the standard suspend semantics.
        return collector->emit(transform_fn(value), cont);
    });
}

/**
 * Returns a flow that switches to a new flow produced by transform every time the original flow emits a value.
 *
 * Kotlin source: kotlinx-coroutines-core/common/src/flow/operators/Merge.kt
 *   public inline fun <T, R> Flow<T>.flatMapLatest(crossinline transform: suspend (value: T) -> Flow<R>): Flow<R> =
 *       transformLatest { emitAll(transform(it)) }
 */
template <typename T, typename R>
std::shared_ptr<Flow<R>> flat_map_latest(
    std::shared_ptr<Flow<T>> upstream,
    std::function<std::shared_ptr<Flow<R>>(T)> transform
) {
    // Upstream:
    //   public fun <T, R> Flow<T>.flatMapLatest(
    //       transform: suspend (value: T) -> Flow<R>): Flow<R> =
    //       transformLatest { emitAll(transform(it)) }
    //
    // The transform callable's suspension is carried by the inner collect call's
    // Continuation; transform_latest cancels the previous inner collect on each new value
    // matching the upstream `emitAll(transform(it))` semantics.
    return transform_latest<T, R>(upstream, [transform](FlowCollector<R>* collector, T value, Continuation<void*>* cont) -> void* {
        auto inner = transform(value);
        return inner->collect(collector, cont);
    });
}


} // namespace flow
} // namespace coroutines
} // namespace kotlinx

// NOTE(port): Definitions for the source collect/emit_all extensions follow the
// merge declarations because Collect.hpp also consumes transform_latest.
#include "kotlinx/coroutines/flow/Collect.hpp"
