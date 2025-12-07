#pragma once
#include "kotlinx/coroutines/core_fwd.hpp"
#include "kotlinx/coroutines/flow/FlowCollector.hpp"
#include "kotlinx/coroutines/CoroutineContext.hpp"
#include <functional>

namespace kotlinx {
namespace coroutines {
namespace flow {
namespace internal {

template <typename T>
class SafeCollector : public FlowCollector<T> {
public:
    SafeCollector(FlowCollector<T>* collector, CoroutineContext collectContext)
        : collector_(collector), collectContext_(collectContext) {}

    void emit(T value) override;

    void release_intercepted() {}

private:
    FlowCollector<T>* collector_;
    CoroutineContext collectContext_;
    int collectContextSize_ = 0; // Stub
};

template <typename T>
void SafeCollector<T>::emit(T value) {
    if (collector_) {
        // Here we would check context preservation
        collector_->emit(value);
    }
}

template <typename T>
class UnsafeFlow : public Flow<T> {
public:
    using FlowBlock = std::function<void(FlowCollector<T>*)>;

    explicit UnsafeFlow(FlowBlock block) : block_(block) {}

    void collect(FlowCollector<T>* collector) override {
        if (block_ && collector) {
            block_(collector);
        }
    }

private:
    FlowBlock block_;
};

template <typename T>
Flow<T>* unsafe_flow(std::function<void(FlowCollector<T>*)> block) {
    return new UnsafeFlow<T>(block);
}

} // namespace internal
} // namespace flow
} // namespace coroutines
} // namespace kotlinx
