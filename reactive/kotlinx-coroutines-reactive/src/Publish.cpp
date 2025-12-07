// Transliterated from: reactive/kotlinx-coroutines-reactive/src/Publish.kt
// TODO: Implement semantic correctness for reactive publish operations

namespace kotlinx {
namespace coroutines {
namespace reactive {

// TODO: #include <kotlinx/atomicfu/atomic.hpp>
// TODO: #include <kotlinx/coroutines/coroutines.hpp>
// TODO: #include <kotlinx/coroutines/channels/channels.hpp>
// TODO: #include <kotlinx/coroutines/selects/selects.hpp>
// TODO: #include <kotlinx/coroutines/sync/sync.hpp>
// TODO: #include <org/reactivestreams/Publisher.hpp>
// TODO: #include <kotlin/coroutines/CoroutineContext.hpp>

/**
 * Creates a cold reactive [Publisher] that runs a given [block] in a coroutine.
 *
 * Every time the returned flux is subscribed, it starts a new coroutine in the specified [context].
 * The coroutine emits (via [Subscriber.onNext]) values with [send][ProducerScope.send],
 * completes (via [Subscriber.onComplete]) when the coroutine completes or channel is explicitly closed, and emits
 * errors (via [Subscriber.onError]) if the coroutine throws an exception or closes channel with a cause.
 * Unsubscribing cancels the running coroutine.
 *
 * Invocations of [send][ProducerScope.send] are suspended appropriately when subscribers apply back-pressure and to
 * ensure that [onNext][Subscriber.onNext] is not invoked concurrently.
 *
 * Coroutine context can be specified with [context] argument.
 * If the context does not have any dispatcher nor any other [ContinuationInterceptor], then [Dispatchers.Default] is
 * used.
 *
 * **Note: This is an experimental api.** Behaviour of publishers that work as children in a parent scope with respect
 *        to cancellation and error handling may change in the future.
 *
 * @throws IllegalArgumentException if the provided [context] contains a [Job] instance.
 */
template<typename T>
Publisher<T> publish(
    const CoroutineContext& context = EmptyCoroutineContext,
    std::function<void(ProducerScope<T>&)> block  // @BuilderInference
) {
    if (context[Job{}] != nullptr) {
        throw std::invalid_argument("Publisher context cannot contain job in it. Its lifecycle should be managed via subscription. Had " + to_string(context));
    }
    return publish_internal(GlobalScope, context, kDefaultHandler, block);
}

/** @suppress For internal use from other reactive integration modules only */
// @InternalCoroutinesApi
template<typename T>
Publisher<T> publish_internal(
    CoroutineScope& scope, // support for legacy publish in scope
    const CoroutineContext& context,
    std::function<void(std::exception_ptr, const CoroutineContext&)> exception_on_cancel_handler,
    std::function<void(ProducerScope<T>&)> block
) {
    return Publisher<T>([=, &scope](Subscriber<T>* subscriber) {
        // specification requires NPE on null subscriber
        if (subscriber == nullptr) throw std::invalid_argument("Subscriber cannot be null");
        auto new_context = scope.new_coroutine_context(context);
        auto coroutine = new PublisherCoroutine<T>(new_context, subscriber, exception_on_cancel_handler);
        subscriber->on_subscribe(*coroutine); // do it first (before starting coroutine), to avoid unnecessary suspensions
        coroutine->start(CoroutineStart::kDefault, *coroutine, block);
    });
}

constexpr long kClosed = -1L;    // closed, but have not signalled onCompleted/onError yet
constexpr long kSignalled = -2L;  // already signalled subscriber onCompleted/onError
const auto kDefaultHandler = [](std::exception_ptr t, const CoroutineContext& ctx) {
    // TODO: Check if t is CancellationException
    handle_coroutine_exception(ctx, t);
};

/** @suppress */
// @Suppress("CONFLICTING_JVM_DECLARATIONS", "RETURN_TYPE_MISMATCH_ON_INHERITANCE")
// @InternalCoroutinesApi
template<typename T>
class PublisherCoroutine : public AbstractCoroutine<void>, public ProducerScope<T>, public Subscription {
private:
    Subscriber<T>* subscriber_;
    std::function<void(std::exception_ptr, const CoroutineContext&)> exception_on_cancel_handler_;
    std::atomic<long> n_requested_{0L}; // < 0 when closed (CLOSED or SIGNALLED)
    std::atomic<bool> cancelled_{false}; // true after Subscription.cancel() is invoked
    Mutex mutex_{true}; // Mutex is locked when either nRequested == 0 or while subscriber.onXXX is being invoked

public:
    PublisherCoroutine(
        const CoroutineContext& parent_context,
        Subscriber<T>* subscriber,
        std::function<void(std::exception_ptr, const CoroutineContext&)> exception_on_cancel_handler
    ) : AbstractCoroutine<void>(parent_context, false, true),
        subscriber_(subscriber),
        exception_on_cancel_handler_(exception_on_cancel_handler) {}

    SendChannel<T>& channel() override { return *this; }

    bool is_closed_for_send() const override { return !is_active(); }

    bool close(std::exception_ptr cause = nullptr) override {
        return cancel_coroutine(cause);
    }

    void invoke_on_close(std::function<void(std::exception_ptr)> handler) override {
        throw std::runtime_error("PublisherCoroutine doesn't support invoke_on_close");
    }

    // @Suppress("UNCHECKED_CAST", "INVISIBLE_MEMBER", "INVISIBLE_REFERENCE")
    // SelectClause2<T, SendChannel<T>> on_send() override {
    //     // TODO: Implement select clause
    //     throw std::runtime_error("Not implemented");
    // }

    ChannelResult<void> try_send(const T& element) override {
        if (!mutex_.try_lock()) {
            return ChannelResult<void>::failure();
        }
        auto throwable = do_locked_next(element);
        if (throwable == nullptr) {
            return ChannelResult<void>::success();
        } else {
            return ChannelResult<void>::closed(throwable);
        }
    }

    // TODO: implement coroutine suspension
    void send(const T& element) override {
        mutex_.lock();
        auto throwable = do_locked_next(element);
        if (throwable != nullptr) {
            throw throwable;
        }
    }

    /*
     * This code is not trivial because of the following properties:
     * 1. It ensures conformance to the reactive specification that mandates that onXXX invocations should not
     *    be concurrent. It uses Mutex to protect all onXXX invocation and ensure conformance even when multiple
     *    coroutines are invoking `send` function.
     * 2. Normally, `onComplete/onError` notification is sent only when coroutine and all its children are complete.
     *    However, nothing prevents `publish` coroutine from leaking reference to it send channel to some
     *    globally-scoped coroutine that is invoking `send` outside of this context. Without extra precaution this may
     *    lead to `onNext` that is concurrent with `onComplete/onError`, so that is why signalling for
     *    `onComplete/onError` is also done under the same mutex.
     * 3. The reactive specification forbids emitting more elements than requested, so `onNext` is forbidden until the
     *    subscriber actually requests some elements. This is implemented by the mutex being locked when emitting
     *    elements is not permitted (`_nRequested.value == 0`).
     */

    /**
     * Attempts to emit a value to the subscriber and, if back-pressure permits this, unlock the mutex.
     *
     * Requires that the caller has locked the mutex before this invocation.
     *
     * If the channel is closed, returns the corresponding exception; otherwise, returns nullptr to denote success.
     *
     * @throws std::invalid_argument if the passed element is nullptr
     */
    std::exception_ptr do_locked_next(const T& elem) {
        // TODO: Implement full logic from Kotlin version
        // Key points:
        // - Check if elem is null (for pointer types)
        // - Check if is_active()
        // - Call subscriber_->on_next(elem)
        // - Handle exceptions from on_next
        // - Update n_requested_ atomically
        // - Handle back-pressure
        // - Call unlock_and_check_completed()
        throw std::runtime_error("Not implemented");
    }

    void unlock_and_check_completed() {
        mutex_.unlock();
        // check isCompleted and try to regain lock to signal completion
        if (is_completed() && mutex_.try_lock()) {
            do_locked_signal_completed(completion_cause(), completion_cause_handled());
        }
    }

    // assert: mutex.isLocked() & isCompleted
    void do_locked_signal_completed(std::exception_ptr cause, bool handled) {
        // TODO: Implement completion signaling logic
        throw std::runtime_error("Not implemented");
    }

    void request(long n) override {
        if (n <= 0) {
            // Specification requires to call onError with IAE for n <= 0
            cancel_coroutine(std::make_exception_ptr(std::invalid_argument("non-positive subscription request " + std::to_string(n))));
            return;
        }
        // TODO: Implement lock-free loop for nRequested
        // Key points:
        // - CAS loop on n_requested_
        // - Handle overflow (set to LONG_MAX)
        // - Unlock mutex when back-pressure is relieved
        throw std::runtime_error("Not implemented");
    }

    // assert: isCompleted
    void signal_completed(std::exception_ptr cause, bool handled) {
        // TODO: Implement lock-free completion signaling
        throw std::runtime_error("Not implemented");
    }

    void on_completed(void value) override {
        signal_completed(nullptr, false);
    }

    void on_cancelled(std::exception_ptr cause, bool handled) override {
        signal_completed(cause, handled);
    }

    // @Suppress("OVERRIDE_DEPRECATION") // Remove after 2.2.0
    void cancel() override {
        // Specification requires that after cancellation publisher stops signalling
        // This flag distinguishes subscription cancellation request from the job crash
        cancelled_.store(true);
        AbstractCoroutine<void>::cancel(nullptr);
    }
};

// @Deprecated(
//     message = "CoroutineScope.publish is deprecated in favour of top-level publish",
//     level = DeprecationLevel.HIDDEN,
//     replaceWith = ReplaceWith("publish(context, block)")
// ) // Since 1.3.0, will be error in 1.3.1 and hidden in 1.4.0. Binary compatibility with Spring
[[deprecated("CoroutineScope.publish is deprecated in favour of top-level publish")]]
template<typename T>
Publisher<T> publish(
    CoroutineScope& scope,
    const CoroutineContext& context = EmptyCoroutineContext,
    std::function<void(ProducerScope<T>&)> block  // @BuilderInference
) {
    return publish_internal(scope, context, kDefaultHandler, block);
}

} // namespace reactive
} // namespace coroutines
} // namespace kotlinx

/*
 * TODO List for semantic implementation:
 *
 * 1. Implement AbstractCoroutine<T> base class
 * 2. Implement ProducerScope<T> interface
 * 3. Implement SendChannel<T> interface
 * 4. Implement Subscription interface
 * 5. Implement Mutex with lock/unlock/try_lock
 * 6. Implement atomic operations (std::atomic)
 * 7. Implement ChannelResult<T> type
 * 8. Implement SelectClause2<T, R> for select support
 * 9. Implement CoroutineStart enum
 * 10. Implement GlobalScope
 * 11. Implement exception handling (handle_coroutine_exception)
 * 12. Implement completion tracking (is_completed, completion_cause, etc.)
 * 13. Implement coroutine lifecycle (start, cancel, etc.)
 * 14. Implement back-pressure management
 * 15. Implement reactive streams compliance
 * 16. Add comprehensive error handling
 * 17. Test concurrent access scenarios
 */
