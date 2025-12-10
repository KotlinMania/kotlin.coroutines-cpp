#pragma once
#include "kotlinx/coroutines/flow/Flow.hpp"
#include "kotlinx/coroutines/flow/internal/ChannelFlow.hpp"
#include <vector>
#include <thread>

namespace kotlinx {
namespace coroutines {
namespace flow {

constexpr int DEFAULT_CONCURRENCY = 16;

/**
 * Merges the given flows into a single flow without preserving an order of elements.
 * All flows are merged concurrently, without limit on the number of simultaneously collected flows.
 */
template <typename T>
Flow<T>* merge(std::vector<Flow<T>*> flows) {
    // Naive implementation using threads (Systematic Stub for Porting)
    return new internal::ChannelFlowOperatorImpl<T>(nullptr); // TODO: Implement ChannelLimitedFlowMerge
}

/**
 * Flattens the given flow of flows into a single flow with a [concurrency] limit.
 */
template <typename T>
Flow<T>* flattenMerge(Flow<Flow<T>*>* flow, int concurrency = DEFAULT_CONCURRENCY) {
    // TODO: Implement ChannelFlowMerge
    return new internal::ChannelFlowOperatorImpl<T>(nullptr);
}

/**
 * Returns a flow that produces element by [transform] function every time the original flow emits a value.
 * When the original flow emits a new value, the previous `transform` block is cancelled.
 */
template <typename T, typename R>
Flow<R>* transformLatest(Flow<T>* flow, std::function<void(FlowCollector<R>*, T)> transform) {
    // TODO: Implement ChannelFlowTransformLatest
    return new internal::ChannelFlowOperatorImpl<R>(nullptr);
}

/**
 * Returns a flow that emits elements from the original flow transformed by [transform] function.
 * When the original flow emits a new value, computation of the [transform] block for previous value is cancelled.
 */
template <typename T, typename R>
Flow<R>* mapLatest(Flow<T>* flow, std::function<R(T)> transform) {
    return transformLatest<T, R>(flow, [transform](FlowCollector<R>* collector, T value) {
        collector->emit(transform(value));
    });
}

} // namespace flow
} // namespace coroutines
} // namespace kotlinx
