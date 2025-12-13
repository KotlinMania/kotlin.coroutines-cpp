#pragma once
/**
 * @file Merge.hpp
 * @brief Flow merge operators: merge, flatten_merge, transform_latest, map_latest
 *
 * Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Merge.kt
 */

#include "kotlinx/coroutines/flow/Flow.hpp"
#include "kotlinx/coroutines/flow/FlowBuilders.hpp"
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

/**
 * Name of the property that defines the value of DEFAULT_CONCURRENCY.
 *
 * Kotlin source: kotlinx-coroutines-core/common/src/flow/operators/Merge.kt
 *   public const val DEFAULT_CONCURRENCY_PROPERTY_NAME: String = "kotlinx.coroutines.flow.defaultConcurrency"
 */
constexpr const char* DEFAULT_CONCURRENCY_PROPERTY_NAME = "kotlinx.coroutines.flow.defaultConcurrency";

/**
 * Default concurrency limit used by flatten_merge and flat_map_merge.
 *
 * Kotlin source: kotlinx-coroutines-core/common/src/flow/operators/Merge.kt
 *   public val DEFAULT_CONCURRENCY: Int = systemProp(DEFAULT_CONCURRENCY_PROPERTY_NAME, 16, 1, Int.MAX_VALUE)
 */
inline int DEFAULT_CONCURRENCY = kotlinx::coroutines::internal::system_prop_int(
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
 * Kotlin source: kotlinx-coroutines-core/common/src/flow/operators/Merge.kt
 *   public fun <T> Flow<Flow<T>>.flattenConcat(): Flow<T> = flow { collect { value -> emitAll(value) } }
 */
template <typename T>
std::shared_ptr<Flow<T>> flatten_concat(std::shared_ptr<Flow<std::shared_ptr<Flow<T>>>> upstream) {
    return flow<T>([upstream](FlowCollector<T>* collector, Continuation<void*>* cont) -> void* {
        class EmitAllCollector : public FlowCollector<std::shared_ptr<Flow<T>>> {
        public:
            explicit EmitAllCollector(FlowCollector<T>* downstream) : downstream_(downstream) {}

            void* emit(std::shared_ptr<Flow<T>> value, Continuation<void*>* continuation) override {
                // Kotlin: emitAll(value)
                // Tail call: safe to return the inner collect result directly.
                return value->collect(downstream_, continuation);
            }

        private:
            FlowCollector<T>* downstream_;
        };

        EmitAllCollector inner(collector);
        return upstream->collect(&inner, cont);
    });
}

/**
 * Merges the given flows into a single flow without preserving an order of elements.
 * All flows are merged concurrently, without limit on the number of simultaneously collected flows.
 */
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
template <typename T>
std::shared_ptr<Flow<T>> merge(std::initializer_list<std::shared_ptr<Flow<T>>> flows) {
    return merge<T>(std::vector<std::shared_ptr<Flow<T>>>(flows));
}

/**
 * Flattens the given flow of flows into a single flow with a [concurrency] limit.
 */
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
template <typename T, typename R>
std::shared_ptr<Flow<R>> flat_map_concat(
    std::shared_ptr<Flow<T>> upstream,
    std::function<std::shared_ptr<Flow<R>>(T)> transform
) {
    // Kotlin: map(transform).flattenConcat()
    // TODO(port): transform is suspend in Kotlin; needs Continuation ABI once flow/map are fully suspend-aware.
    auto mapped = flow<std::shared_ptr<Flow<R>>>([upstream, transform](FlowCollector<std::shared_ptr<Flow<R>>>* collector, Continuation<void*>* cont) -> void* {
        class MapCollector : public FlowCollector<T> {
        public:
            MapCollector(FlowCollector<std::shared_ptr<Flow<R>>>* downstream, std::function<std::shared_ptr<Flow<R>>(T)> fn)
                : downstream_(downstream), fn_(std::move(fn)) {}

            void* emit(T value, Continuation<void*>* continuation) override {
                return downstream_->emit(fn_(value), continuation);
            }

        private:
            FlowCollector<std::shared_ptr<Flow<R>>>* downstream_;
            std::function<std::shared_ptr<Flow<R>>(T)> fn_;
        };

        MapCollector mapper(collector, transform);
        return upstream->collect(&mapper, cont);
    });

    return flatten_concat<R>(mapped);
}

/**
 * Transforms elements emitted by the original flow by applying transform, that returns another flow,
 * and then merging and flattening these flows.
 *
 * Kotlin source: kotlinx-coroutines-core/common/src/flow/operators/Merge.kt
 *   public fun <T, R> Flow<T>.flatMapMerge(concurrency: Int = DEFAULT_CONCURRENCY, transform: suspend (value: T) -> Flow<R>): Flow<R> =
 *       map(transform).flattenMerge(concurrency)
 */
template <typename T, typename R>
std::shared_ptr<Flow<R>> flat_map_merge(
    std::shared_ptr<Flow<T>> upstream,
    int concurrency,
    std::function<std::shared_ptr<Flow<R>>(T)> transform
) {
    // Kotlin: map(transform).flattenMerge(concurrency)
    // TODO(port): transform is suspend in Kotlin; needs Continuation ABI once flow/map are fully suspend-aware.
    auto mapped = flow<std::shared_ptr<Flow<R>>>([upstream, transform](FlowCollector<std::shared_ptr<Flow<R>>>* collector, Continuation<void*>* cont) -> void* {
        class MapCollector : public FlowCollector<T> {
        public:
            MapCollector(FlowCollector<std::shared_ptr<Flow<R>>>* downstream, std::function<std::shared_ptr<Flow<R>>(T)> fn)
                : downstream_(downstream), fn_(std::move(fn)) {}

            void* emit(T value, Continuation<void*>* continuation) override {
                return downstream_->emit(fn_(value), continuation);
            }

        private:
            FlowCollector<std::shared_ptr<Flow<R>>>* downstream_;
            std::function<std::shared_ptr<Flow<R>>(T)> fn_;
        };

        MapCollector mapper(collector, transform);
        return upstream->collect(&mapper, cont);
    });

    return flatten_merge<R>(mapped, concurrency);
}

template <typename T, typename R>
std::shared_ptr<Flow<R>> flat_map_merge(
    std::shared_ptr<Flow<T>> upstream,
    std::function<std::shared_ptr<Flow<R>>(T)> transform
) {
    return flat_map_merge<T, R>(upstream, DEFAULT_CONCURRENCY, std::move(transform));
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
        // TODO(suspend): emit is suspendable.
        // For now, assuming emit is synchronous or ignoring result.
        // Ideally: return collector->emit(transform_fn(value), cont);
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
    // TODO(port): transform is suspend in Kotlin; needs Continuation ABI for full parity.
    return transform_latest<T, R>(upstream, [transform](FlowCollector<R>* collector, T value, Continuation<void*>* cont) -> void* {
        auto inner = transform(value);
        return inner->collect(collector, cont);
    });
}


} // namespace flow
} // namespace coroutines
} // namespace kotlinx
