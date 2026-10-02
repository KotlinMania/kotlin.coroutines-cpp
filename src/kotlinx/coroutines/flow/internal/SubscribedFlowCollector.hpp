#pragma once
/**
 * Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Share.kt (lines 415-428)
 */

#include "kotlinx/coroutines/flow/FlowCollector.hpp"
#include "kotlinx/coroutines/Continuation.hpp"
#include <functional>
#include <memory>
#include <utility>

namespace kotlinx::coroutines::flow::internal {

template <typename T>
class SubscribedFlowCollector : public FlowCollector<T> {
private:
    FlowCollector<T>* collector_;
    std::function<void*(FlowCollector<T>*, std::shared_ptr<Continuation<void*>>)> action_;

public:
    SubscribedFlowCollector(
        FlowCollector<T>* collector,
        std::function<void*(FlowCollector<T>*, std::shared_ptr<Continuation<void*>>)> action)
        : collector_(collector), action_(std::move(action)) {}

    void* emit(T value, Continuation<void*>* cont) override {
        return collector_->emit(std::move(value), cont);
    }

    void* on_subscription(Continuation<void*>* cont) {
        if (action_) {
            action_(collector_, nullptr);
        }
        if (auto* sub = dynamic_cast<SubscribedFlowCollector<T>*>(collector_)) {
            return sub->on_subscription(cont);
        }
        return nullptr;
    }
};

} // namespace kotlinx::coroutines::flow::internal
