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
std::shared_ptr<Flow<T>> merge(std::vector<std::shared_ptr<Flow<T>>> flows) {
    return flow<T>([flows](FlowCollector<T>* collector) {
        flow_scope<void>([&](CoroutineScope& scope){
            auto channel = std::make_shared<BufferedChannel<T>>(Channel<T>::UNLIMITED);
            std::atomic<int> active_flows(flows.size());
            
            for(auto f : flows) { // f is shared_ptr
                scope.launch([&, f](){
                    try {
                        f->collect([&](T val) {
                            channel->send(val);
                        });
                    } catch(...) {
                    }
                    if(--active_flows == 0) {
                        channel->close();
                    }
                });
            }
            if (flows.empty()) channel->close();
            
            for(auto it = channel->iterator(); it->has_next(); ) {
                 collector->emit(it->next());
            }
        });
    });
}

/**
 * Flattens the given flow of flows into a single flow with a [concurrency] limit.
 */
template <typename T>
std::shared_ptr<Flow<T>> flattenMerge(std::shared_ptr<Flow<std::shared_ptr<Flow<T>>>> flow, int concurrency = DEFAULT_CONCURRENCY) {
    // TODO: Implement ChannelFlowMerge
    return nullptr;
}

/**
 * Returns a flow that produces element by [transform] function every time the original flow emits a value.
 * When the original flow emits a new value, the previous `transform` block is cancelled.
 */
template <typename T, typename R>
std::shared_ptr<Flow<R>> transformLatest(std::shared_ptr<Flow<T>> flow, std::function<void(FlowCollector<R>*, T)> transform) {
    // TODO: Implement ChannelFlowTransformLatest
    return nullptr;
}

/**
 * Returns a flow that emits elements from the original flow transformed by [transform] function.
 * When the original flow emits a new value, computation of the [transform] block for previous value is cancelled.
 */
template <typename T, typename R>
std::shared_ptr<Flow<R>> mapLatest(std::shared_ptr<Flow<T>> flow, std::function<R(T)> transform) {
    return transformLatest<T, R>(flow, [transform](FlowCollector<R>* collector, T value) {
        collector->emit(transform(value));
    });
}


} // namespace flow
} // namespace coroutines
} // namespace kotlinx
