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
#include "kotlinx/coroutines/Dispatchers.hpp"
#include "kotlinx/coroutines/Job.hpp"
#include "kotlinx/coroutines/Result.hpp"
#include "kotlinx/coroutines/channels/BufferOverflow.hpp"
#include "kotlinx/coroutines/channels/Channel.hpp"
#include "kotlinx/coroutines/dsl/Suspend.hpp"
#include "kotlinx/coroutines/intrinsics/Intrinsics.hpp"
#include "kotlinx/coroutines/flow/Distinct.hpp"
#include "kotlinx/coroutines/flow/internal/FlowExceptions.hpp"
#include "kotlinx/coroutines/flow/internal/SubscribedFlowCollector.hpp"

#include <algorithm>
#include <exception>
#include <functional>
#include <memory>
#include <mutex>
#include <optional>
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
                channel_flow->context(),
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

    std::vector<T> get_replay_cache() const override { return delegate_->get_replay_cache(); }
    std::vector<T> replay_cache() const { return get_replay_cache(); }
    std::shared_ptr<StateFlow<int>> subscription_count() const override {
        return delegate_->subscription_count();
    }
    StateFlow<int>* get_subscription_count() const override {
        return delegate_->get_subscription_count();
    }

    Flow<T>* fuse(std::shared_ptr<CoroutineContext> context,
                  int capacity,
                  BufferOverflow on_buffer_overflow) override {
        return fuse_shared_flow<T>(delegate_, context, capacity, on_buffer_overflow).get();
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
    T value() const override { return delegate_->value(); }
    std::vector<T> get_replay_cache() const override { return delegate_->get_replay_cache(); }
    std::vector<T> replay_cache() const { return get_replay_cache(); }
    std::shared_ptr<StateFlow<int>> subscription_count() const override {
        return delegate_->subscription_count();
    }
    StateFlow<int>* get_subscription_count() const override {
        return delegate_->get_subscription_count();
    }

    Flow<T>* fuse(std::shared_ptr<CoroutineContext> context,
                  int capacity,
                  BufferOverflow on_buffer_overflow) override {
        return fuse_state_flow<T>(*delegate_, context, capacity, on_buffer_overflow).get();
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
    std::optional<T> initial_value) {
    const bool is_eager = (started == SharingStarted::eagerly() ||
                           dynamic_cast<StartedEagerly*>(started) != nullptr);

    auto sharing_context = context ? context : EmptyCoroutineContext::instance();
    auto combined = scope->get_coroutine_context()->operator+(sharing_context);
    if (!combined->get(ContinuationInterceptor::type_key)) {
        sharing_context = sharing_context->operator+(std::shared_ptr<CoroutineContext>(
            &Dispatchers::get_default(), [](CoroutineContext*) {}));
    }

    return launch(scope, sharing_context, CoroutineStart::DEFAULT, [upstream, shared, started, initial_value, is_eager](CoroutineScope* sharing_scope) {
        if (is_eager) {
            auto cont = std::make_shared<FunctionalContinuation<void*>>(
                sharing_scope ? sharing_scope->get_coroutine_context() : nullptr,
                [](Result<void*>) {}
            );
            if (sharing_scope && sharing_scope->get_coroutine_context()) {
                auto job = std::dynamic_pointer_cast<Job>(sharing_scope->get_coroutine_context()->get(Job::type_key));
                if (job) {
                    job->invoke_on_completion([cont](std::exception_ptr) {});
                }
            }
            upstream->collect(shared.get(), cont.get());
        } else if (started == SharingStarted::lazily() ||
                   dynamic_cast<StartedLazily*>(started) != nullptr) {
            if (shared->subscription_count()->value() <= 0) {
                try {
                    class FirstSubscriberCollector : public FlowCollector<int> {
                    public:
                        void* emit(int count, Continuation<void*>*) override {
                            if (count > 0) {
                                throw internal::AbortFlowException(this);
                            }
                            return nullptr;
                        }
                    };
                    FirstSubscriberCollector sub_collector;
                    shared->subscription_count()->collect(&sub_collector, nullptr);
                } catch (const internal::AbortFlowException&) {
                    // First subscriber arrived
                }
            }
            auto cont = std::make_shared<FunctionalContinuation<void*>>(
                sharing_scope ? sharing_scope->get_coroutine_context() : nullptr,
                [](Result<void*>) {}
            );
            if (sharing_scope && sharing_scope->get_coroutine_context()) {
                auto job = std::dynamic_pointer_cast<Job>(sharing_scope->get_coroutine_context()->get(Job::type_key));
                if (job) {
                    job->invoke_on_completion([cont](std::exception_ptr) {});
                }
            }
            upstream->collect(shared.get(), cont.get());
        } else {
            auto command_flow = distinct_until_changed<SharingCommand>(
                started->command(shared->subscription_count()));

            class CommandCollector : public FlowCollector<SharingCommand> {
            private:
                CoroutineScope* scope_;
                std::shared_ptr<Flow<T>> upstream_;
                std::shared_ptr<MutableSharedFlow<T>> shared_;
                std::optional<T> initial_value_;
                std::shared_ptr<Job> upstream_job_;
                std::mutex job_mutex_;

            public:
                CommandCollector(CoroutineScope* scope,
                                 std::shared_ptr<Flow<T>> upstream,
                                 std::shared_ptr<MutableSharedFlow<T>> shared,
                                 std::optional<T> initial_value)
                    : scope_(scope),
                      upstream_(std::move(upstream)),
                      shared_(std::move(shared)),
                      initial_value_(std::move(initial_value)),
                      upstream_job_(nullptr) {}

                void cancel_upstream() {
                    std::unique_lock<std::mutex> lock(job_mutex_);
                    if (upstream_job_) {
                        upstream_job_->cancel(std::make_exception_ptr(CancellationException("Sharing coroutine completed")));
                        auto old_job = upstream_job_;
                        upstream_job_ = nullptr;
                        lock.unlock();
                        old_job->join_blocking();
                    }
                }

                void* emit(SharingCommand cmd, Continuation<void*>*) override {
                    std::unique_lock<std::mutex> lock(job_mutex_);
                    if (upstream_job_) {
                        upstream_job_->cancel(std::make_exception_ptr(CancellationException("Sharing command changed")));
                        auto old_job = upstream_job_;
                        upstream_job_ = nullptr;
                        lock.unlock();
                        old_job->join_blocking();
                        lock.lock();
                    }

                    switch (cmd) {
                        case SharingCommand::START: {
                            upstream_job_ = kotlinx::coroutines::launch(
                                scope_,
                                nullptr,
                                CoroutineStart::DEFAULT,
                                [upstream = upstream_, shared = shared_](CoroutineScope* s) {
                                    auto cont = std::make_shared<FunctionalContinuation<void*>>(
                                        s ? s->get_coroutine_context() : nullptr,
                                        [](Result<void*>) {}
                                    );
                                    if (s && s->get_coroutine_context()) {
                                        auto job = std::dynamic_pointer_cast<Job>(s->get_coroutine_context()->get(Job::type_key));
                                        if (job) {
                                            job->invoke_on_completion([cont](std::exception_ptr) {});
                                        }
                                    }
                                    upstream->collect(shared.get(), cont.get());
                                }
                            );
                            break;
                        }
                        case SharingCommand::STOP: {
                            break;
                        }
                        case SharingCommand::STOP_AND_RESET_REPLAY_CACHE: {
                            if (!initial_value_.has_value()) {
                                shared_->reset_replay_cache();
                            } else {
                                shared_->try_emit(*initial_value_);
                            }
                            break;
                        }
                    }
                    return nullptr;
                }
            };

            auto collector = std::make_shared<CommandCollector>(sharing_scope, upstream, shared, initial_value);
            auto sharing_cont = std::make_shared<FunctionalContinuation<void*>>(
                sharing_scope ? sharing_scope->get_coroutine_context() : nullptr,
                [collector](Result<void*>) {}
            );
            if (sharing_scope && sharing_scope->get_coroutine_context()) {
                auto job = std::dynamic_pointer_cast<Job>(sharing_scope->get_coroutine_context()->get(Job::type_key));
                if (job) {
                    job->invoke_on_completion([collector, sharing_cont](std::exception_ptr) {
                        collector->cancel_upstream();
                    });
                }
            }
            try {
                command_flow->collect(collector.get(), sharing_cont.get());
            } catch (...) {
                collector->cancel_upstream();
                throw;
            }
            collector->cancel_upstream();
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
                        state_ = make_mutable_state_flow<T>(value);
                        auto job_el = scope_->get_coroutine_context()->get(Job::type_key);
                        auto job = std::dynamic_pointer_cast<Job>(job_el);
                        result_->complete(Result<std::shared_ptr<StateFlow<T>>>::success(
                            std::shared_ptr<ReadonlyStateFlow<T>>(new ReadonlyStateFlow<T>(state_, job))));
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

    std::vector<T> get_replay_cache() const override { return shared_flow_->get_replay_cache(); }
    std::vector<T> replay_cache() const { return get_replay_cache(); }
    std::shared_ptr<StateFlow<int>> subscription_count() const override {
        return shared_flow_->subscription_count();
    }
    StateFlow<int>* get_subscription_count() const override {
        return shared_flow_->get_subscription_count();
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
    auto shared = make_mutable_shared_flow<T>(
        replay, config.extra_buffer_capacity, config.on_buffer_overflow);
    auto job = launch_sharing<T>(
        scope, config.context, config.upstream, shared, started, std::nullopt);
    return std::shared_ptr<ReadonlySharedFlow<T>>(new ReadonlySharedFlow<T>(shared, std::move(job)));
}

// -------------------------------- stateIn --------------------------------

template <typename T>
inline std::shared_ptr<StateFlow<T>> state_in(
    std::shared_ptr<Flow<T>> upstream,
    CoroutineScope* scope,
    SharingStarted* started,
    T initial_value) {
    auto config = configure_sharing<T>(std::move(upstream), /*replay=*/1);
    auto state = make_mutable_state_flow<T>(initial_value);
    auto job = launch_sharing<T>(
        scope, config.context, config.upstream, state, started, std::make_optional<T>(initial_value));
    return std::shared_ptr<ReadonlyStateFlow<T>>(new ReadonlyStateFlow<T>(state, std::move(job)));
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
    return std::shared_ptr<ReadonlySharedFlow<T>>(new ReadonlySharedFlow<T>(std::move(mutable_flow), nullptr));
}

template <typename T>
inline std::shared_ptr<StateFlow<T>> as_state_flow(std::shared_ptr<MutableStateFlow<T>> mutable_flow) {
    return std::shared_ptr<ReadonlyStateFlow<T>>(new ReadonlyStateFlow<T>(std::move(mutable_flow), nullptr));
}

// -------------------------------- onSubscription --------------------------------

template <typename T>
inline std::shared_ptr<SharedFlow<T>> on_subscription(
    std::shared_ptr<SharedFlow<T>> shared_flow,
    typename SubscribedSharedFlow<T>::ActionFn action) {
    return std::make_shared<SubscribedSharedFlow<T>>(std::move(shared_flow), std::move(action));
}

} // namespace kotlinx::coroutines::flow
