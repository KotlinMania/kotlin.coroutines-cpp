#pragma once
/**
 * @file Merge.hpp
 * @brief Flow merge operators: merge, flattenMerge, transformLatest, mapLatest
 *
 * Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Merge.kt
 */

#include "kotlinx/coroutines/flow/Flow.hpp"
#include "kotlinx/coroutines/flow/FlowBuilders.hpp"
#include "kotlinx/coroutines/flow/internal/ChannelFlow.hpp"
#include "kotlinx/coroutines/channels/BufferedChannel.hpp"
#include <vector>
#include <thread>
#include <atomic>
#include <functional>

namespace kotlinx {
namespace coroutines {
namespace flow {

using kotlinx::coroutines::channels::BufferedChannel;
using kotlinx::coroutines::channels::Channel;

constexpr int DEFAULT_CONCURRENCY = 16;

// Alias for the flow builder to avoid shadowing issues with parameters
template<typename T>
inline std::shared_ptr<Flow<T>> make_flow(std::function<void(FlowCollector<T>*)> block) {
    return flow<T>(block);
}

/**
 * Merges the given flows into a single flow without preserving an order of elements.
 * All flows are merged concurrently, without limit on the number of simultaneously collected flows.
 *
 * TODO: Implement proper coroutine-based concurrent collection
 */
template <typename T>
std::shared_ptr<Flow<T>> merge(std::vector<std::shared_ptr<Flow<T>>> flows) {
    // TODO: Implement proper merge using ChannelFlow and coroutine scope
    // For now, collect sequentially as a simple fallback
    return make_flow<T>([flows](FlowCollector<T>* collector) {
        for (auto& f : flows) {
            f->collect([collector](T value) {
                collector->emit(value);
            });
        }
    });
}

/**
 * Flattens the given flow of flows into a single flow with a [concurrency] limit.
 *
 * TODO: Implement ChannelFlowMerge with proper concurrency limiting
 */
template <typename T>
std::shared_ptr<Flow<T>> flatten_merge(std::shared_ptr<Flow<std::shared_ptr<Flow<T>>>> upstream, int concurrency = DEFAULT_CONCURRENCY) {
    // TODO: Implement ChannelFlowMerge with proper concurrency limiting
    // For now, collect sequentially
    return make_flow<T>([upstream](FlowCollector<T>* collector) {
        upstream->collect([collector](std::shared_ptr<Flow<T>> inner) {
            inner->collect([collector](T value) {
                collector->emit(value);
            });
        });
    });
}

/**
 * Returns a flow that produces element by [transform] function every time the original flow emits a value.
 * When the original flow emits a new value, the previous `transform` block is cancelled.
 *
 * TODO: Implement ChannelFlowTransformLatest with proper cancellation
 */
template <typename T, typename R>
std::shared_ptr<Flow<R>> transform_latest(std::shared_ptr<Flow<T>> upstream, std::function<void(FlowCollector<R>*, T)> transform_fn) {
    // TODO: Implement ChannelFlowTransformLatest with proper cancellation
    // For now, just transform without cancellation
    return make_flow<R>([upstream, transform_fn](FlowCollector<R>* collector) {
        upstream->collect([collector, transform_fn](T value) {
            transform_fn(collector, value);
        });
    });
}

/**
 * Returns a flow that emits elements from the original flow transformed by [transform] function.
 * When the original flow emits a new value, computation of the [transform] block for previous value is cancelled.
 */
template <typename T, typename R>
std::shared_ptr<Flow<R>> map_latest(std::shared_ptr<Flow<T>> upstream, std::function<R(T)> transform_fn) {
    return transform_latest<T, R>(upstream, [transform_fn](FlowCollector<R>* collector, T value) {
        collector->emit(transform_fn(value));
    });
}


} // namespace flow
} // namespace coroutines
} // namespace kotlinx
