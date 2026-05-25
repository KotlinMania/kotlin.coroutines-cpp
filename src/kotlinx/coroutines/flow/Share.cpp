/**
 * Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Share.kt
 *
 * Kotlin file header (translated):
 *   package kotlinx.coroutines.flow
 *   imports: kotlinx.coroutines.*, kotlinx.coroutines.channels.*, kotlinx.coroutines.flow.internal.*,
 *            kotlin.coroutines.*, kotlin.jvm.JvmField
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

namespace kotlinx::coroutines::flow {

using channels::BufferOverflow;
using channels::Channel;

namespace {

/**
 * Sentinel mirroring upstream `NO_VALUE` — `SharedFlow.shareIn` passes it as the
 * `initialValue` so the launch coroutine can distinguish between "no initial value
 * for state reset" and "reset to a real initial value".
 */
inline void* NO_VALUE_SENTINEL() {
    static int sentinel = 0;
    return &sentinel;
}

/**
 * Upstream:
 *   private class SharingConfig<T>(
 *       val upstream: Flow<T>,
 *       val extraBufferCapacity: Int,
 *       val onBufferOverflow: BufferOverflow,
 *       val context: CoroutineContext)
 */
template <typename T>
struct SharingConfig {
    std::shared_ptr<Flow<T>> upstream;
    int extra_buffer_capacity;
    BufferOverflow on_buffer_overflow;
    std::shared_ptr<CoroutineContext> context;
};

/**
 * Upstream:
 *   private fun <T> Flow<T>.configureSharing(replay: Int): SharingConfig<T> { ... }
 *
 * Decomposes the upstream so we can fuse with any preceding buffer/flowOn ChannelFlow
 * operators. When the upstream is a ChannelFlow whose `dropChannelOperators()` returns a
 * channel-free downstream, we eliminate the intermediate channel; otherwise we wrap in a
 * default-sized buffer.
 */
template <typename T>
SharingConfig<T> configure_sharing(std::shared_ptr<Flow<T>> upstream, int replay) {
    if (replay < 0) {
        throw std::invalid_argument("replay must be non-negative, was " + std::to_string(replay));
    }
    const int default_extra_capacity =
        std::max(replay, Channel::CHANNEL_DEFAULT_CAPACITY()) - replay;

    if (auto channel_flow = std::dynamic_pointer_cast<internal::ChannelFlow<T>>(upstream)) {
        if (auto dropped = channel_flow->drop_channel_operators()) {
            int extra_capacity = 0;
            const int capacity = channel_flow->capacity();
            const BufferOverflow on_overflow = channel_flow->on_buffer_overflow();
            if (capacity == Channel::OPTIONAL_CHANNEL ||
                capacity == Channel::CHANNEL_BUFFERED ||
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
                std::move(dropped),
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

/**
 * Upstream:
 *   private fun <T> CoroutineScope.launchSharing(
 *       context: CoroutineContext, upstream: Flow<T>, shared: MutableSharedFlow<T>,
 *       started: SharingStarted, initialValue: T): Job { ... }
 */
template <typename T>
std::shared_ptr<Job> launch_sharing(
    CoroutineScope* scope,
    std::shared_ptr<CoroutineContext> context,
    std::shared_ptr<Flow<T>> upstream,
    std::shared_ptr<MutableSharedFlow<T>> shared,
    SharingStarted* started,
    T* initial_value) {
    const CoroutineStart start = (started == SharingStarted::Eagerly())
                                     ? CoroutineStart::DEFAULT
                                     : CoroutineStart::UNDISPATCHED;
    return scope->launch(context, start, [=]() mutable {
        if (started == SharingStarted::Eagerly()) {
            upstream->collect(shared);
        } else if (started == SharingStarted::Lazily()) {
            shared->subscription_count()->first([](int n) { return n > 0; });
            upstream->collect(shared);
        } else {
            started->command(shared->subscription_count())
                ->distinct_until_changed()
                ->collect_latest([=](SharingCommand cmd) mutable {
                    switch (cmd) {
                        case SharingCommand::START:
                            upstream->collect(shared);
                            break;
                        case SharingCommand::STOP:
                            break;
                        case SharingCommand::STOP_AND_RESET_REPLAY_CACHE:
                            if (initial_value == nullptr) {
                                shared->reset_replay_cache();
                            } else {
                                shared->try_emit(*initial_value);
                            }
                            break;
                    }
                });
        }
    });
}

/**
 * Upstream:
 *   private fun <T> CoroutineScope.launchSharingDeferred(
 *       context: CoroutineContext, upstream: Flow<T>,
 *       result: CompletableDeferred<Result<StateFlow<T>>>) { ... }
 */
template <typename T>
void launch_sharing_deferred(
    CoroutineScope* scope,
    std::shared_ptr<CoroutineContext> context,
    std::shared_ptr<Flow<T>> upstream,
    std::shared_ptr<CompletableDeferred<Result<std::shared_ptr<StateFlow<T>>>>> result) {
    scope->launch(context, CoroutineStart::DEFAULT, [=]() mutable {
        try {
            std::shared_ptr<MutableStateFlow<T>> state;
            upstream->collect_each([&](const T& value) {
                if (state) {
                    state->set_value(value);
                } else {
                    state = std::make_shared<MutableStateFlow<T>>(value);
                    auto job = scope->coroutine_context()->get_job();
                    result->complete(Result<std::shared_ptr<StateFlow<T>>>::success(
                        std::make_shared<ReadonlyStateFlow<T>>(state, job)));
                }
            });
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

/**
 * Upstream:
 *   private class ReadonlySharedFlow<T>(
 *       flow: SharedFlow<T>, private val job: Job?
 *   ) : SharedFlow<T> by flow, CancellableFlow<T>, FusibleFlow<T> {
 *       override fun fuse(context, capacity, onBufferOverflow) =
 *           fuseSharedFlow(context, capacity, onBufferOverflow)
 *   }
 */
template <typename T>
class ReadonlySharedFlow : public SharedFlow<T>,
                           public internal::CancellableFlow<T>,
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
        return internal::fuse_shared_flow<T>(delegate_, context, capacity, on_buffer_overflow);
    }

private:
    std::shared_ptr<SharedFlow<T>> delegate_;
    std::shared_ptr<Job> job_;  // keeps a strong reference to the job (if present)
};

/**
 * Upstream:
 *   private class ReadonlyStateFlow<T>(
 *       flow: StateFlow<T>, private val job: Job?
 *   ) : StateFlow<T> by flow, CancellableFlow<T>, FusibleFlow<T> {
 *       override fun fuse(context, capacity, onBufferOverflow) =
 *           fuseStateFlow(context, capacity, onBufferOverflow)
 *   }
 */
template <typename T>
class ReadonlyStateFlow : public StateFlow<T>,
                          public internal::CancellableFlow<T>,
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
        return internal::fuse_state_flow<T>(delegate_, context, capacity, on_buffer_overflow);
    }

private:
    std::shared_ptr<StateFlow<T>> delegate_;
    std::shared_ptr<Job> job_;
};

/**
 * Upstream:
 *   private class SubscribedSharedFlow<T>(
 *       private val sharedFlow: SharedFlow<T>,
 *       private val action: suspend FlowCollector<T>.() -> Unit
 *   ) : SharedFlow<T> by sharedFlow {
 *       override suspend fun collect(collector: FlowCollector<T>) =
 *           sharedFlow.collect(SubscribedFlowCollector(collector, action))
 *   }
 */
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

} // namespace

// -------------------------------- shareIn --------------------------------

/**
 * Upstream:
 *   public fun <T> Flow<T>.shareIn(
 *       scope: CoroutineScope, started: SharingStarted, replay: Int = 0
 *   ): SharedFlow<T> {
 *       val config = configureSharing(replay)
 *       val shared = MutableSharedFlow<T>(
 *           replay = replay,
 *           extraBufferCapacity = config.extraBufferCapacity,
 *           onBufferOverflow = config.onBufferOverflow)
 *       val job = scope.launchSharing(config.context, config.upstream, shared, started, NO_VALUE as T)
 *       return ReadonlySharedFlow(shared, job)
 *   }
 */
template <typename T>
std::shared_ptr<SharedFlow<T>> share_in(
    std::shared_ptr<Flow<T>> upstream,
    CoroutineScope* scope,
    SharingStarted* started,
    int replay = 0) {
    auto config = configure_sharing<T>(std::move(upstream), replay);
    auto shared = std::make_shared<MutableSharedFlow<T>>(
        replay, config.extra_buffer_capacity, config.on_buffer_overflow);
    auto job = launch_sharing<T>(
        scope, config.context, config.upstream, shared, started, /*initial_value=*/nullptr);
    return std::make_shared<ReadonlySharedFlow<T>>(shared, std::move(job));
}

// -------------------------------- stateIn --------------------------------

/**
 * Upstream:
 *   public fun <T> Flow<T>.stateIn(
 *       scope: CoroutineScope, started: SharingStarted, initialValue: T
 *   ): StateFlow<T> {
 *       val config = configureSharing(1)
 *       val state = MutableStateFlow(initialValue)
 *       val job = scope.launchSharing(config.context, config.upstream, state, started, initialValue)
 *       return ReadonlyStateFlow(state, job)
 *   }
 */
template <typename T>
std::shared_ptr<StateFlow<T>> state_in(
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

/**
 * Upstream:
 *   public suspend fun <T> Flow<T>.stateIn(scope: CoroutineScope): StateFlow<T> {
 *       val config = configureSharing(1)
 *       val result = CompletableDeferred<Result<StateFlow<T>>>(scope.coroutineContext[Job])
 *       scope.launchSharingDeferred(config.context, config.upstream, result)
 *       return result.await().getOrThrow()
 *   }
 */
template <typename T>
[[suspend]]
void* state_in(
    std::shared_ptr<Flow<T>> upstream,
    CoroutineScope* scope,
    std::shared_ptr<Continuation<void*>> completion) {
    auto config = configure_sharing<T>(std::move(upstream), /*replay=*/1);
    auto parent_job = scope->coroutine_context()->get_job();
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

/**
 * Upstream:
 *   public fun <T> MutableSharedFlow<T>.asSharedFlow(): SharedFlow<T> =
 *       ReadonlySharedFlow(this, null)
 */
template <typename T>
std::shared_ptr<SharedFlow<T>> as_shared_flow(std::shared_ptr<MutableSharedFlow<T>> mutable_flow) {
    return std::make_shared<ReadonlySharedFlow<T>>(std::move(mutable_flow), nullptr);
}

/**
 * Upstream:
 *   public fun <T> MutableStateFlow<T>.asStateFlow(): StateFlow<T> =
 *       ReadonlyStateFlow(this, null)
 */
template <typename T>
std::shared_ptr<StateFlow<T>> as_state_flow(std::shared_ptr<MutableStateFlow<T>> mutable_flow) {
    return std::make_shared<ReadonlyStateFlow<T>>(std::move(mutable_flow), nullptr);
}

// -------------------------------- onSubscription --------------------------------

/**
 * Upstream:
 *   public fun <T> SharedFlow<T>.onSubscription(
 *       action: suspend FlowCollector<T>.() -> Unit
 *   ): SharedFlow<T> = SubscribedSharedFlow(this, action)
 */
template <typename T>
std::shared_ptr<SharedFlow<T>> on_subscription(
    std::shared_ptr<SharedFlow<T>> shared_flow,
    typename SubscribedSharedFlow<T>::ActionFn action) {
    return std::make_shared<SubscribedSharedFlow<T>>(std::move(shared_flow), std::move(action));
}

} // namespace kotlinx::coroutines::flow
