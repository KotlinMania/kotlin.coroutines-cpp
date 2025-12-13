#pragma once
/**
 * @file Merge.hpp
 * @brief Flow merge operators: merge, flattenMerge, transformLatest, mapLatest
 *
 * Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Merge.kt
 */

#include "kotlinx/coroutines/flow/Flow.hpp"
#include "kotlinx/coroutines/flow/FlowBuilders.hpp"
#include "kotlinx/coroutines/flow/internal/Merge.hpp"
#include <stdexcept>
#include <string>

namespace kotlinx {
namespace coroutines {
namespace flow {


using kotlinx::coroutines::channels::Channel;
using kotlinx::coroutines::flow::internal::ChannelFlowMerge;
using kotlinx::coroutines::flow::internal::ChannelLimitedFlowMerge;
using kotlinx::coroutines::flow::internal::ChannelFlowTransformLatest;

constexpr int DEFAULT_CONCURRENCY = 16;

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
                // TODO(semantics): this needs a proper suspend state machine to be safe across suspension.
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
 * Flattens the given flow of flows into a single flow with a [concurrency] limit.
 */
template <typename T>
std::shared_ptr<Flow<T>> flatten_merge(std::shared_ptr<Flow<std::shared_ptr<Flow<T>>>> upstream, int concurrency = DEFAULT_CONCURRENCY) {
    // Kotlin: require(concurrency > 0) { "Expected positive concurrency level, but had $concurrency" }
    if (concurrency <= 0) {
        throw std::invalid_argument("Expected positive concurrency level, but had " + std::to_string(concurrency)); // TODO(port): exception type parity
    }

    // Kotlin: if (concurrency == 1) flattenConcat() else ChannelFlowMerge(this, concurrency)
    if (concurrency == 1) {
        return flatten_concat<T>(upstream);
    }

    return std::make_shared<ChannelFlowMerge<T>>(upstream, concurrency);
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
        collector->emit(transform_fn(value), cont);
        return nullptr;
    });
}


} // namespace flow
} // namespace coroutines
} // namespace kotlinx
