#pragma once
#include "kotlinx/coroutines/CoroutineScope.hpp"
#include "kotlinx/coroutines/flow/Flow.hpp"
#include <functional>
#include <memory>
#include <vector>

namespace kotlinx {
namespace coroutines {
namespace flow {

// Temporarily disable Flow builder stubs with complex types until full C++ port is ready.
#if 0
/**
 * Creates a flow from the given suspendable block.
 */
template <typename T>
std::shared_ptr<Flow<T>> flow(std::function<void(std::shared_ptr<FlowCollector<T>>)> block) {
    // Stub implementation
    return nullptr;
}

/**
 * Creates a flow that produces a single value from the given functional type.
 */
template <typename T>
std::shared_ptr<Flow<T>> as_flow(std::function<T()> func) {
    return flow<T>([func](std::shared_ptr<FlowCollector<T>> collector) {
        collector->emit(func());
    });
}

/**
 * Creates a flow that produces values from the given iterable.
 */
template <typename T>
std::shared_ptr<Flow<T>> as_flow(const std::vector<T>& iterable) {
    return flow<T>([iterable](std::shared_ptr<FlowCollector<T>> collector) {
        for (const auto& value : iterable) {
            collector->emit(value);
        }
    });
}

/**
 * Creates a flow from elements.
 */
template <typename T>
std::shared_ptr<Flow<T>> flow_of(std::initializer_list<T> elements) {
    std::vector<T> vec = elements;
    return as_flow(vec);
}

/**
 * flowScope builder
 */
template <typename R>
R flow_scope(std::function<R(CoroutineScope&)> block) {
    // Implementation stub 
    return R();
}

/**
 * scopedFlow builder
 */
template <typename R>
std::shared_ptr<Flow<R>> scoped_flow(std::function<void(CoroutineScope&, std::shared_ptr<FlowCollector<R>>)> block) {
    // Implementation stub
    return nullptr;
}

// channelFlow stub
template<typename T>
std::shared_ptr<Flow<T>> channel_flow(std::function<void(void*)> block) { // void* as placeholder for ProducerScope
    return nullptr;
}
#endif

} // namespace flow
} // namespace coroutines
} // namespace kotlinx
