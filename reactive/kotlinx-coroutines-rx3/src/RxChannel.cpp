// Transliterated from: reactive/kotlinx-coroutines-rx3/src/RxChannel.cpp

namespace kotlinx {
namespace coroutines {
namespace rx3 {

// TODO: #include equivalent
// import io.reactivex.rxjava3.core.*
// import io.reactivex.rxjava3.disposables.*
// import kotlinx.atomicfu.*
// import kotlinx.coroutines.channels.*
// import kotlinx.coroutines.flow.*

/**
 * Subscribes to this [MaybeSource] and returns a channel to receive elements emitted by it.
 * The resulting channel shall be [cancelled][ReceiveChannel.cancel] to unsubscribe from this source.
 *
 * This API is internal in the favour of [Flow].
 * [MaybeSource] doesn't have a corresponding [Flow] adapter, so it should be transformed to [Observable] first.
 */
// @PublishedApi
template<typename T>
ReceiveChannel<T>* open_subscription(MaybeSource<T>& source) {
    // internal fun <T> MaybeSource<T & Any>.openSubscription(): ReceiveChannel<T> {
    //     val channel = SubscriptionChannel<T>()
    //     subscribe(channel)
    //     return channel
    // }

    // TODO: Create SubscriptionChannel
    // TODO: Subscribe to source
    return nullptr;
}

/**
 * Subscribes to this [ObservableSource] and returns a channel to receive elements emitted by it.
 * The resulting channel shall be [cancelled][ReceiveChannel.cancel] to unsubscribe from this source.
 *
 * This API is internal in the favour of [Flow].
 * [ObservableSource] doesn't have a corresponding [Flow] adapter, so it should be transformed to [Observable] first.
 */
// @PublishedApi
template<typename T>
ReceiveChannel<T>* open_subscription(ObservableSource<T>& source) {
    // internal fun <T> ObservableSource<T & Any>.openSubscription(): ReceiveChannel<T> {
    //     val channel = SubscriptionChannel<T>()
    //     subscribe(channel)
    //     return channel
    // }

    // TODO: Create SubscriptionChannel
    // TODO: Subscribe to source
    return nullptr;
}

/**
 * Subscribes to this [MaybeSource] and performs the specified action for each received element.
 *
 * If [action] throws an exception at some point or if the [MaybeSource] raises an error, the exception is rethrown from
 * [collect].
 */
// TODO: implement coroutine suspension
template<typename T, typename Action>
void collect(MaybeSource<T>& source, Action&& action) {
    // suspend inline fun <T> MaybeSource<T & Any>.collect(action: (T) -> Unit): Unit =
    //     openSubscription().consumeEach(action)

    // TODO: Open subscription
    // TODO: Consume each element
}

/**
 * Subscribes to this [ObservableSource] and performs the specified action for each received element.
 *
 * If [action] throws an exception at some point, the subscription is cancelled, and the exception is rethrown from
 * [collect]. Also, if the [ObservableSource] signals an error, that error is rethrown from [collect].
 */
// TODO: implement coroutine suspension
template<typename T, typename Action>
void collect(ObservableSource<T>& source, Action&& action) {
    // suspend inline fun <T> ObservableSource<T & Any>.collect(action: (T) -> Unit): Unit =
    //     openSubscription().consumeEach(action)

    // TODO: Open subscription
    // TODO: Consume each element
}

// @Suppress("INVISIBLE_REFERENCE", "INVISIBLE_MEMBER")
template<typename T>
class SubscriptionChannel : public BufferedChannel<T>,
                            public Observer<T>,
                            public MaybeObserver<T> {
private:
    std::atomic<Disposable*> subscription_;

public:
    SubscriptionChannel()
        : BufferedChannel<T>(Channel::kUnlimited), subscription_(nullptr) {}

    // @Suppress("CANNOT_OVERRIDE_INVISIBLE_MEMBER")
    void on_closed_idempotent() override {
        // override fun onClosedIdempotent() {
        //     _subscription.getAndSet(null)?.dispose() // dispose exactly once
        // }
        Disposable* old = subscription_.exchange(nullptr);
        if (old) {
            old->dispose();
        }
    }

    // Observer override
    void on_subscribe(Disposable* sub) override {
        // override fun onSubscribe(sub: Disposable) {
        //     _subscription.value = sub
        // }
        subscription_.store(sub);
    }

    void on_success(const T& t) override {
        // override fun onSuccess(t: T & Any) {
        //     trySend(t)
        //     close(cause = null)
        // }
        try_send(t);
        close(nullptr);
    }

    void on_next(const T& t) override {
        // override fun onNext(t: T & Any) {
        //     trySend(t) // Safe to ignore return value here, expectedly racing with cancellation
        // }
        try_send(t); // Safe to ignore return value here, expectedly racing with cancellation
    }

    void on_complete() override {
        // override fun onComplete() {
        //     close(cause = null)
        // }
        close(nullptr);
    }

    void on_error(const std::exception& e) override {
        // override fun onError(e: Throwable) {
        //     close(cause = e)
        // }
        close(&e);
    }
};

} // namespace rx3
} // namespace coroutines
} // namespace kotlinx

/*
 * TODO: Semantic Implementation Tasks
 *
 * 1. Implement ReceiveChannel<T> interface
 * 2. Implement BufferedChannel<T> base class
 * 3. Implement Observer<T> and MaybeObserver<T> interfaces from RxJava3
 * 4. Implement Disposable interface and disposal mechanism
 * 5. Implement atomic operations for subscription management
 * 6. Implement Channel::kUnlimited constant
 * 7. Implement try_send and close methods
 * 8. Implement consumeEach coroutine function
 * 9. Implement MaybeSource and ObservableSource types
 * 10. Add proper exception handling and propagation
 * 11. Implement thread-safe channel operations
 * 12. Add unit tests for subscription and collection
 */
