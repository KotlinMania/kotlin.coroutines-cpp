#pragma once
/**
 * Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Share.kt (lines 415-428)
 */

#include "kotlinx/coroutines/flow/FlowCollector.hpp"
#include "kotlinx/coroutines/Continuation.hpp"
#include "kotlinx/coroutines/ContinuationImpl.hpp"
#include "kotlinx/coroutines/flow/internal/SafeCollector.hpp"
#include "kotlinx/coroutines/dsl/Suspend.hpp"
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
        // The frame owns the action's safe collector through completion and cleanup.
        class SubscriptionFrame final : public ContinuationImpl {
        public:
            SubscriptionFrame(
                FlowCollector<T>* collector,
                std::function<void*(FlowCollector<T>*, std::shared_ptr<Continuation<void*>>)> action,
                Continuation<void*>* completion)
                : ContinuationImpl(kotlinx::coroutines::internal::retain_continuation(completion)),
                  collector_(collector), action_(std::move(action)),
                  synchronous_(completion == nullptr),
                  safe_collector_(std::make_shared<SafeCollector<T>>(collector, get_context())) {}

            void retain() { self_ref_ = shared_from_this(); }

            void* invoke_suspend(Result<void*> result) override {
                try {
                    coroutine_begin(this)
                    if (action_) {
                        coroutine_yield(this, action_(safe_collector_.get(),
                            synchronous_ ? nullptr : shared_from_this()));
                    }
                    release_safe_collector();
                    subscribed_ = dynamic_cast<SubscribedFlowCollector<T>*>(collector_);
                    if (subscribed_) {
                        coroutine_yield(this, subscribed_->on_subscription(synchronous_ ? nullptr : this));
                    }
                    self_ref_.reset();
                    coroutine_end(this)
                } catch (...) {
                    release_safe_collector();
                    self_ref_.reset();
                    throw;
                }
            }

        private:
            void release_safe_collector() {
                if (safe_collector_) {
                    safe_collector_->release_intercepted();
                    safe_collector_.reset();
                }
            }
            void* _label = nullptr;
            FlowCollector<T>* collector_;
            std::function<void*(FlowCollector<T>*, std::shared_ptr<Continuation<void*>>)> action_;
            bool synchronous_;
            std::shared_ptr<SafeCollector<T>> safe_collector_;
            SubscribedFlowCollector<T>* subscribed_ = nullptr;
            std::shared_ptr<BaseContinuationImpl> self_ref_;
        };
        auto frame = std::make_shared<SubscriptionFrame>(collector_, action_, cont);
        frame->retain();
        return frame->start(Result<void*>::success(nullptr));
    }
};

} // namespace kotlinx::coroutines::flow::internal
