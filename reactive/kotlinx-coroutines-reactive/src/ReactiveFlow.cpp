// Transliterated from: reactive/kotlinx-coroutines-reactive/src/ReactiveFlow.kt
// TODO: Implement semantic correctness for reactive flow operations

namespace kotlinx {
namespace coroutines {
namespace reactive {

// TODO: #include <kotlinx/atomicfu/atomic.hpp>
// TODO: #include <kotlinx/coroutines/coroutines.hpp>
// TODO: #include <kotlinx/coroutines/channels/channels.hpp>
// TODO: #include <kotlinx/coroutines/flow/flow.hpp>
// TODO: #include <kotlinx/coroutines/flow/internal/internal.hpp>
// TODO: #include <kotlinx/coroutines/intrinsics/intrinsics.hpp>
// TODO: #include <org/reactivestreams/Publisher.hpp>
// TODO: #include <vector>
// TODO: #include <kotlin/coroutines/CoroutineContext.hpp>
// TODO: #include <kotlinx/coroutines/internal/internal.hpp>

/**
 * Transforms the given reactive [Publisher] into [Flow].
 * Use the [buffer] operator on the resulting flow to specify the size of the back-pressure.
 * In effect, it specifies the value of the subscription's [request][Subscription.request].
 * The [default buffer capacity][Channel.BUFFERED] for a suspending channel is used by default.
 *
 * If any of the resulting flow transformations fails, the subscription is immediately cancelled and all the in-flight
 * elements are discarded.
 *
 * This function is integrated with `ReactorContext` from `kotlinx-coroutines-reactor` module,
 * see its documentation for additional details.
 */
template<typename T>
Flow<T> as_flow(Publisher<T>& publisher) {
    return PublisherAsFlow<T>(publisher);
}

/**
 * Transforms the given flow into a reactive specification compliant [Publisher].
 *
 * This function is integrated with `ReactorContext` from `kotlinx-coroutines-reactor` module,
 * see its documentation for additional details.
 *
 * An optional [context] can be specified to control the execution context of calls to the [Subscriber] methods.
 * A [CoroutineDispatcher] can be set to confine them to a specific thread; various [ThreadContextElement] can be set to
 * inject additional context into the caller thread. By default, the [Unconfined][Dispatchers.Unconfined] dispatcher
 * is used, so calls are performed from an arbitrary thread.
 */
// @JvmOverloads // binary compatibility
template<typename T>
Publisher<T> as_publisher(Flow<T>& flow, const CoroutineContext& context = EmptyCoroutineContext) {
    return FlowAsPublisher<T>(flow, Dispatchers::kUnconfined + context);
}

template<typename T>
class PublisherAsFlow : public ChannelFlow<T> {
private:
    Publisher<T>& publisher_;

    // @Suppress("INVISIBLE_MEMBER", "INVISIBLE_REFERENCE")
    long request_size() const {
        if (on_buffer_overflow_ != BufferOverflow::kSuspend) {
            return LONG_MAX; // request all, since buffering strategy is to never suspend
        }
        switch (capacity_) {
            case Channel::kRendezvous: return 1L; // need to request at least one anyway
            case Channel::kUnlimited: return LONG_MAX; // reactive streams way to say "give all", must be Long.MAX_VALUE
            case Channel::kBuffered: return static_cast<long>(Channel::kChannelDefaultCapacity);
            default:
                long result = static_cast<long>(capacity_);
                // TODO: check(result >= 1)
                return result;
        }
    }

public:
    PublisherAsFlow(
        Publisher<T>& publisher,
        const CoroutineContext& context = EmptyCoroutineContext,
        int capacity = Channel::kBuffered,
        BufferOverflow on_buffer_overflow = BufferOverflow::kSuspend
    ) : ChannelFlow<T>(context, capacity, on_buffer_overflow),
        publisher_(publisher) {}

    ChannelFlow<T>* create(const CoroutineContext& context, int capacity, BufferOverflow on_buffer_overflow) override {
        return new PublisherAsFlow<T>(publisher_, context, capacity, on_buffer_overflow);
    }

    // TODO: implement coroutine suspension
    void collect(FlowCollector<T>& collector) override {
        // TODO: Implement full collect logic
        // Key points:
        // - Get collect context
        // - Check for dispatcher change
        // - Fast path: subscribe directly
        // - Slow path: produce in separate dispatcher
        throw std::runtime_error("Not implemented");
    }

private:
    // TODO: implement coroutine suspension
    void collect_slow_path(FlowCollector<T>& collector) {
        // TODO: Use coroutineScope and produceImpl
        throw std::runtime_error("Not implemented");
    }

    // TODO: implement coroutine suspension
    void collect_impl(const CoroutineContext& inject_context, FlowCollector<T>& collector) {
        auto subscriber = ReactiveSubscriber<T>(capacity_, on_buffer_overflow_, request_size());
        // inject subscribe context into publisher
        publisher_.inject_coroutine_context(inject_context).subscribe(subscriber);
        // TODO: Implement collection loop with request management
        throw std::runtime_error("Not implemented");
    }

    // TODO: implement coroutine suspension
    void collect_to(ProducerScope<T>& scope) override {
        collect_impl(scope.coroutine_context(), SendingCollector<T>(scope.channel()));
    }
};

// @Suppress("ReactiveStreamsSubscriberImplementation")
template<typename T>
class ReactiveSubscriber : public Subscriber<T> {
private:
    int capacity_;
    BufferOverflow on_buffer_overflow_;
    long request_size_;
    Subscription* subscription_;
    // This implementation of ReactiveSubscriber always uses "offer" in its onNext implementation and it cannot
    // be reliable with rendezvous channel, so a rendezvous channel is replaced with buffer=1 channel
    Channel<T> channel_;

public:
    ReactiveSubscriber(int capacity, BufferOverflow on_buffer_overflow, long request_size)
        : capacity_(capacity),
          on_buffer_overflow_(on_buffer_overflow),
          request_size_(request_size),
          subscription_(nullptr),
          channel_(capacity == Channel::kRendezvous ? 1 : capacity, on_buffer_overflow) {}

    // TODO: implement coroutine suspension
    std::optional<T> take_next_or_null() {
        // TODO: Implement with channel.receiveCatching()
        throw std::runtime_error("Not implemented");
    }

    void on_next(const T& value) override {
        // Controlled by requestSize
        bool success = channel_.try_send(value).is_success();
        if (!success) {
            throw std::runtime_error("Element was not added to channel because it was full");
        }
    }

    void on_complete() override {
        channel_.close();
    }

    void on_error(std::exception_ptr t) override {
        channel_.close(t);
    }

    void on_subscribe(Subscription& s) override {
        subscription_ = &s;
        make_request();
    }

    void make_request() {
        subscription_->request(request_size_);
    }

    void cancel() {
        subscription_->cancel();
    }
};

// ContextInjector service is implemented in `kotlinx-coroutines-reactor` module only.
// If `kotlinx-coroutines-reactor` module is not included, the list is empty.
// TODO: Implement ServiceLoader equivalent
std::vector<ContextInjector*> context_injectors_;

template<typename T>
Publisher<T> inject_coroutine_context(Publisher<T>& publisher, const CoroutineContext& coroutine_context) {
    Publisher<T> result = publisher;
    for (auto* injector : context_injectors_) {
        result = injector->inject_coroutine_context(result, coroutine_context);
    }
    return result;
}

/**
 * Adapter that transforms [Flow] into TCK-complaint [Publisher].
 * [cancel] invocation cancels the original flow.
 */
// @Suppress("ReactiveStreamsPublisherImplementation")
template<typename T>
class FlowAsPublisher : public Publisher<T> {
private:
    Flow<T>& flow_;
    CoroutineContext context_;

public:
    FlowAsPublisher(Flow<T>& flow, const CoroutineContext& context)
        : flow_(flow), context_(context) {}

    void subscribe(Subscriber<T>* subscriber) override {
        if (subscriber == nullptr) throw std::invalid_argument("Subscriber cannot be null");
        subscriber->on_subscribe(new FlowSubscription<T>(flow_, subscriber, context_));
    }
};

/** @suppress */
// @InternalCoroutinesApi
template<typename T>
class FlowSubscription : public Subscription, public AbstractCoroutine<void> {
private:
    Flow<T>& flow_;
    Subscriber<T>* subscriber_;
    std::atomic<long> requested_{0L};
    std::atomic<Continuation<void>*> producer_;
    std::atomic<bool> cancellation_requested_{false};

public:
    FlowSubscription(
        Flow<T>& flow,
        Subscriber<T>* subscriber,
        const CoroutineContext& context
    ) : AbstractCoroutine<void>(context, false, true),
        flow_(flow),
        subscriber_(subscriber),
        producer_(create_initial_continuation()) {}

    /*
     * We deliberately set initParentJob to false and do not establish parent-child
     * relationship because FlowSubscription doesn't support it
     */

    // This code wraps startCoroutineCancellable into continuation
    Continuation<void>* create_initial_continuation() {
        // TODO: Implement continuation wrapping
        return nullptr;
    }

    // TODO: implement coroutine suspension
    void flow_processing() {
        // TODO: Implement full flow processing logic
        // Key points:
        // - Call consumeFlow()
        // - Handle exceptions with unwrap
        // - Check cancellation_requested_
        // - Call subscriber_->on_complete() or on_error()
        throw std::runtime_error("Not implemented");
    }

    /*
     * This method has at most one caller at any time (triggered from the `request` method)
     */
    // TODO: implement coroutine suspension
    void consume_flow() {
        // TODO: Implement flow collection loop
        // Key points:
        // - flow_.collect([](T value) { ... })
        // - Call subscriber_->on_next(value)
        // - Decrement requested_
        // - Suspend when requested <= 0
        throw std::runtime_error("Not implemented");
    }

    // @Deprecated("Since 1.2.0, binary compatibility with versions <= 1.1.x", level = DeprecationLevel.HIDDEN)
    [[deprecated("Since 1.2.0, binary compatibility with versions <= 1.1.x")]]
    void cancel() override {
        cancellation_requested_.store(true);
        AbstractCoroutine<void>::cancel(nullptr);
    }

    void request(long n) override {
        if (n <= 0) return;
        // TODO: Implement lock-free request update
        // Key points:
        // - Atomic update of requested_
        // - Handle overflow (cap at LONG_MAX)
        // - Resume producer if was suspended
        throw std::runtime_error("Not implemented");
    }
};

} // namespace reactive
} // namespace coroutines
} // namespace kotlinx

/*
 * TODO List for semantic implementation:
 *
 * 1. Implement Flow<T> and ChannelFlow<T> interfaces
 * 2. Implement FlowCollector<T> interface
 * 3. Implement ProducerScope<T> and SendingCollector<T>
 * 4. Implement Channel<T> with BufferOverflow support
 * 5. Implement BufferOverflow enum (SUSPEND, DROP_OLDEST, DROP_LATEST)
 * 6. Implement Channel capacity constants (RENDEZVOUS, UNLIMITED, BUFFERED, etc.)
 * 7. Implement Continuation<T> and coroutine primitives
 * 8. Implement AbstractCoroutine<T> base class
 * 9. Implement CoroutineContext operations (get, plus)
 * 10. Implement Dispatchers (Unconfined, etc.)
 * 11. Implement suspendCancellableCoroutine
 * 12. Implement context injection (inject_coroutine_context)
 * 13. Implement ServiceLoader for ContextInjector
 * 14. Implement exception unwrapping (unwrap)
 * 15. Implement coroutineScope builder
 * 16. Implement produceImpl/emitAll for slow path
 * 17. Test reactive streams compliance
 * 18. Test backpressure scenarios
 * 19. Test cancellation behavior
 * 20. Test context propagation
 */
