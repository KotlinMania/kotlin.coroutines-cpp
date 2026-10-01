#pragma once
/**
 * Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Share.kt
 */

#include "kotlinx/coroutines/flow/Flow.hpp"
#include "kotlinx/coroutines/flow/FlowCollector.hpp"
#include "kotlinx/coroutines/flow/SharedFlow.hpp"
#include "kotlinx/coroutines/flow/SharingStarted.hpp"
#include "kotlinx/coroutines/flow/StateFlow.hpp"
#include "kotlinx/coroutines/flow/internal/ChannelFlow.hpp"
#include "kotlinx/coroutines/CompletableDeferred.hpp"
#include "kotlinx/coroutines/CoroutineContext.hpp"
#include "kotlinx/coroutines/CoroutineScope.hpp"
#include "kotlinx/coroutines/CoroutineStart.hpp"
#include "kotlinx/coroutines/Builders.hpp"
#include "kotlinx/coroutines/Job.hpp"
#include "kotlinx/coroutines/Result.hpp"
#include "kotlinx/coroutines/channels/BufferOverflow.hpp"
#include "kotlinx/coroutines/channels/Channel.hpp"
#include "kotlinx/coroutines/dsl/Suspend.hpp"
#include "kotlinx/coroutines/intrinsics/Intrinsics.hpp"

#include <algorithm>
#include <exception>
#include <functional>
#include <memory>
#include <utility>
#include <stdexcept>

namespace kotlinx::coroutines::flow {

using channels::BufferOverflow;

namespace detail {

inline void* NO_VALUE_SENTINEL() {
    static int sentinel = 0;
    return &sentinel;
}

} // namespace detail

namespace internal {

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

} // namespace internal

template <typename T>
inline std::shared_ptr<Flow<T>> fuse_shared_flow(
    std::shared_ptr<SharedFlow<T>> flow,
    std::shared_ptr<CoroutineContext> context,
    int capacity,
    channels::BufferOverflow on_buffer_overflow) {
    if ((capacity == channels::CHANNEL_OPTIONAL || capacity == channels::CHANNEL_BUFFERED) &&
        on_buffer_overflow == channels::BufferOverflow::SUSPEND) {
        if (!context || context == EmptyCoroutineContext::instance()) {
            return flow;
        }
    }
    return std::make_shared<internal::ChannelFlowOperatorImpl<T>>(
        flow, context, capacity, on_buffer_overflow);
}

template <typename T>
struct SharingConfig {
    std::shared_ptr<Flow<T>> upstream;
    int extra_buffer_capacity;
    BufferOverflow on_buffer_overflow;
    std::shared_ptr<CoroutineContext> context;
};

template <typename T>
inline SharingConfig<T> configure_sharing(std::shared_ptr<Flow<T>> upstream, int replay) {
    if (replay < 0) {
        throw std::invalid_argument("replay must be non-negative, was " + std::to_string(replay));
    }
    const int default_extra_capacity =
        std::max(replay, 64) - replay;

    if (auto channel_flow = std::dynamic_pointer_cast<internal::ChannelFlow<T>>(upstream)) {
        if (auto dropped = channel_flow->drop_channel_operators()) {
            int extra_capacity = 0;
            const int capacity = channel_flow->capacity();
            const BufferOverflow on_overflow = channel_flow->on_buffer_overflow();
            if (capacity == channels::CHANNEL_OPTIONAL ||
                capacity == channels::CHANNEL_BUFFERED ||
                capacity == 0) {
                if (on_overflow == BufferOverflow::SUSPEND) {
                    extra_capacity = (capacity == 0) ? 0 : default_extra_capacity;
                } else if (replay == 0) {
                    extra_capacity = 1;
                } else {
                    extra_capacity = 0;
                }
            } else {
                extra_capacity = capacity;
            }
            return SharingConfig<T>{
                std::shared_ptr<Flow<T>>(upstream, dropped),
                extra_capacity,
                on_overflow,
                channel_flow->get_context(),
            };
        }
    }
    return SharingConfig<T>{
        std::move(upstream),
        default_extra_capacity,
        BufferOverflow::SUSPEND,
        EmptyCoroutineContext::instance(),
    };
}

template <typename T>
class ReadonlySharedFlow : public SharedFlow<T>,
                           public CancellableFlow<T>,
                           public internal::FusibleFlow<T> {
public:
    ReadonlySharedFlow(std::shared_ptr<SharedFlow<T>> flow, std::shared_ptr<Job> job)
        : delegate_(std::move(flow)), job_(std::move(job)) {}

    void* collect(FlowCollector<T>* collector, Continuation<void*>* cont) override {
        return delegate_->collect(collector, cont);
    }

    const std::vector<T>& replay_cache() const override { return delegate_->replay_cache(); }
    std::shared_ptr<StateFlow<int>> subscription_count() const override {
        return delegate_->subscription_count();
    }

    std::shared_ptr<Flow<T>> fuse(std::shared_ptr<CoroutineContext> context,
                                  int capacity,
                                  BufferOverflow on_buffer_overflow) override {
        return fuse_shared_flow<T>(delegate_, context, capacity, on_buffer_overflow);
    }

private:
    std::shared_ptr<SharedFlow<T>> delegate_;
    std::shared_ptr<Job> job_;
};

template <typename T>
class ReadonlyStateFlow : public StateFlow<T>,
                          public CancellableFlow<T>,
                          public internal::FusibleFlow<T> {
public:
    ReadonlyStateFlow(std::shared_ptr<StateFlow<T>> flow, std::shared_ptr<Job> job)
        : delegate_(std::move(flow)), job_(std::move(job)) {}

    void* collect(FlowCollector<T>* collector, Continuation<void*>* cont) override {
        return delegate_->collect(collector, cont);
    }
    const T& value() const override { return delegate_->value(); }
    const std::vector<T>& replay_cache() const override { return delegate_->replay_cache(); }
    std::shared_ptr<StateFlow<int>> subscription_count() const override {
        return delegate_->subscription_count();
    }

    std::shared_ptr<Flow<T>> fuse(std::shared_ptr<CoroutineContext> context,
                                  int capacity,
                                  BufferOverflow on_buffer_overflow) override {
        return fuse_state_flow<T>(delegate_, context, capacity, on_buffer_overflow);
    }

private:
    std::shared_ptr<StateFlow<T>> delegate_;
    std::shared_ptr<Job> job_;
};

template <typename T>
inline std::shared_ptr<Job> launch_sharing(
    CoroutineScope* scope,
    std::shared_ptr<CoroutineContext> context,
    std::shared_ptr<Flow<T>> upstream,
    std::shared_ptr<MutableSharedFlow<T>> shared,
    SharingStarted* started,
    T* initial_value) {
    const CoroutineStart start = (started == SharingStarted::eagerly())
                                     ? CoroutineStart::DEFAULT
                                     : CoroutineStart::UNDISPATCHED;
    return launch(scope, context, start, [upstream, shared, started, initial_value](CoroutineScope*) {
        if (started == SharingStarted::eagerly()) {
            upstream->collect(shared.get(), nullptr);
        } else if (started == SharingStarted::lazily()) {
            upstream->collect(shared.get(), nullptr);
        } else {
            upstream->collect(shared.get(), nullptr);
        }
    });
}

template <typename T>
inline void launch_sharing_deferred(
    CoroutineScope* scope,
    std::shared_ptr<CoroutineContext> context,
    std::shared_ptr<Flow<T>> upstream,
    std::shared_ptr<CompletableDeferred<Result<std::shared_ptr<StateFlow<T>>>>> result) {
    launch(scope, context, CoroutineStart::DEFAULT, [scope, upstream, result](CoroutineScope*) {
        try {
            std::shared_ptr<MutableStateFlow<T>> state;
            class DeferredCollector : public FlowCollector<T> {
            public:
                CoroutineScope* scope_;
                std::shared_ptr<CompletableDeferred<Result<std::shared_ptr<StateFlow<T>>>>> result_;
                std::shared_ptr<MutableStateFlow<T>>& state_;

                DeferredCollector(CoroutineScope* s, std::shared_ptr<CompletableDeferred<Result<std::shared_ptr<StateFlow<T>>>>> r, std::shared_ptr<MutableStateFlow<T>>& st)
                    : scope_(s), result_(r), state_(st) {}

                void* emit(T value, Continuation<void*>*) override {
                    if (state_) {
                        state_->set_value(value);
                    } else {
                        state_ = std::make_shared<MutableStateFlow<T>>(value);
                        auto job_el = scope_->get_coroutine_context()->get(Job::type_key);
                        auto job = std::dynamic_pointer_cast<Job>(job_el);
                        result_->complete(Result<std::shared_ptr<StateFlow<T>>>::success(
                            std::make_shared<ReadonlyStateFlow<T>>(state_, job)));
                    }
                    return nullptr;
                }
            };
            DeferredCollector collector(scope, result, state);
            upstream->collect(&collector, nullptr);
            if (!state) {
                result->complete(Result<std::shared_ptr<StateFlow<T>>>::failure(
                    std::make_exception_ptr(
                        std::out_of_range("Flow is empty"))));
            }
        } catch (...) {
            auto exception = std::current_exception();
            result->complete_exceptionally(exception);
            std::rethrow_exception(exception);
        }
    });
}

template <typename T>
class SubscribedSharedFlow : public SharedFlow<T> {
public:
    using ActionFn =
        std::function<void*(FlowCollector<T>*, std::shared_ptr<Continuation<void*>>)>;

    SubscribedSharedFlow(std::shared_ptr<SharedFlow<T>> shared, ActionFn action)
        : shared_flow_(std::move(shared)), action_(std::move(action)) {}

    const std::vector<T>& replay_cache() const override { return shared_flow_->replay_cache(); }
    std::shared_ptr<StateFlow<int>> subscription_count() const override {
        return shared_flow_->subscription_count();
    }

    void* collect(FlowCollector<T>* collector, Continuation<void*>* cont) override {
        auto subscribed = std::make_shared<internal::SubscribedFlowCollector<T>>(
            collector, action_);
        return shared_flow_->collect(subscribed.get(), cont);
    }

private:
    std::shared_ptr<SharedFlow<T>> shared_flow_;
    ActionFn action_;
};

// -------------------------------- shareIn --------------------------------

template <typename T>
inline std::shared_ptr<SharedFlow<T>> share_in(
    std::shared_ptr<Flow<T>> upstream,
    CoroutineScope* scope,
    SharingStarted* started,
    int replay = 0) {
    auto config = configure_sharing<T>(std::move(upstream), replay);
    auto shared = std::make_shared<MutableSharedFlow<T>>(
        replay, config.extra_buffer_capacity, config.on_buffer_overflow);
    auto job = launch_sharing<T>(
        scope, config.context, config.upstream, shared, started, nullptr);
    return std::make_shared<ReadonlySharedFlow<T>>(shared, std::move(job));
}

// -------------------------------- stateIn --------------------------------

template <typename T>
inline std::shared_ptr<StateFlow<T>> state_in(
    std::shared_ptr<Flow<T>> upstream,
    CoroutineScope* scope,
    SharingStarted* started,
    T initial_value) {
    auto config = configure_sharing<T>(std::move(upstream), /*replay=*/1);
    auto state = std::make_shared<MutableStateFlow<T>>(initial_value);
    auto job = launch_sharing<T>(
        scope, config.context, config.upstream, state, started, &initial_value);
    return std::make_shared<ReadonlyStateFlow<T>>(state, std::move(job));
}

template <typename T>
[[suspend]]
inline void* state_in(
    std::shared_ptr<Flow<T>> upstream,
    CoroutineScope* scope,
    std::shared_ptr<Continuation<void*>> completion) {
    auto config = configure_sharing<T>(std::move(upstream), /*replay=*/1);
    auto job_el = scope->get_coroutine_context()->get(Job::type_key);
    auto parent_job = std::dynamic_pointer_cast<Job>(job_el);
    auto result =
        make_completable_deferred<Result<std::shared_ptr<StateFlow<T>>>>(parent_job);
    launch_sharing_deferred<T>(scope, config.context, config.upstream, result);
    void* awaited = dsl::suspend(result->await(completion.get()));
    if (intrinsics::is_coroutine_suspended(awaited)) {
        return intrinsics::get_COROUTINE_SUSPENDED();
    }
    auto* outcome = static_cast<Result<std::shared_ptr<StateFlow<T>>>*>(awaited);
    return new std::shared_ptr<StateFlow<T>>(outcome->get_or_throw());
}

// -------------------------------- asSharedFlow / asStateFlow --------------------------------

template <typename T>
inline std::shared_ptr<SharedFlow<T>> as_shared_flow(std::shared_ptr<MutableSharedFlow<T>> mutable_flow) {
    return std::make_shared<ReadonlySharedFlow<T>>(std::move(mutable_flow), nullptr);
}

template <typename T>
inline std::shared_ptr<StateFlow<T>> as_state_flow(std::shared_ptr<MutableStateFlow<T>> mutable_flow) {
    return std::make_shared<ReadonlyStateFlow<T>>(std::move(mutable_flow), nullptr);
}

// -------------------------------- onSubscription --------------------------------

template <typename T>
inline std::shared_ptr<SharedFlow<T>> on_subscription(
    std::shared_ptr<SharedFlow<T>> shared_flow,
    typename SubscribedSharedFlow<T>::ActionFn action) {
    return std::make_shared<SubscribedSharedFlow<T>>(std::move(shared_flow), std::move(action));
}

} // namespace kotlinx::coroutines::flow
