#pragma once
#include "kotlinx/coroutines/flow/Flow.hpp"
#include "kotlinx/coroutines/flow/FlowBuilders.hpp"
#include <functional>
#include <memory>

namespace kotlinx {
namespace coroutines {
namespace flow {

template <typename T>
std::shared_ptr<Flow<T>> filter(std::shared_ptr<Flow<T>> flow, std::function<bool(T)> predicate) {
    return flow<T>([flow, predicate](FlowCollector<T>* collector) {
        flow->collect([&](T value) {
            if (predicate(value)) {
                collector->emit(value);
            }
        });
    });
}

template <typename T, typename R>
std::shared_ptr<Flow<R>> map(std::shared_ptr<Flow<T>> flow, std::function<R(T)> transform) {
    return flow<R>([flow, transform](FlowCollector<R>* collector) {
        flow->collect([&](T value) {
            collector->emit(transform(value));
        });
    });
}

template <typename T, typename R>
std::shared_ptr<Flow<R>> mapNotNull(std::shared_ptr<Flow<T>> flow, std::function<R*(T)> transform) { // Pointer return for nullable
    return flow<R>([flow, transform](FlowCollector<R>* collector) {
        flow->collect([&](T value) {
            auto result = transform(value);
            if (result != nullptr) {
                collector->emit(*result);
            }
        });
    });
}
// Overload for std::optional or other nullable types? Keeping it simple for now.

} // namespace flow
} // namespace coroutines
} // namespace kotlinx

