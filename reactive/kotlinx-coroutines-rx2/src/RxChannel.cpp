// Transliterated from: reactive/kotlinx-coroutines-rx2/src/RxChannel.kt

// TODO: #include <io/reactivex/MaybeSource.hpp>
// TODO: #include <io/reactivex/ObservableSource.hpp>
// TODO: #include <io/reactivex/Observer.hpp>
// TODO: #include <io/reactivex/MaybeObserver.hpp>
// TODO: #include <io/reactivex/disposables/Disposable.hpp>
// TODO: #include <kotlinx/atomicfu/atomic.hpp>
// TODO: #include <kotlinx/coroutines/channels/ReceiveChannel.hpp>
// TODO: #include <kotlinx/coroutines/channels/BufferedChannel.hpp>
// TODO: #include <kotlinx/coroutines/channels/Channel.hpp>
// TODO: #include <kotlinx/coroutines/flow/Flow.hpp>
// TODO: #include <kotlinx/coroutines/reactive/...>

namespace kotlinx {
namespace coroutines {
namespace rx2 {

/**
 * Subscribes to this [MaybeSource] and performs the specified action for each received element.
 *
 * If [action] throws an exception at some point or if the [MaybeSource] raises an error, the exception is rethrown from
 * [collect].
 */
// suspend inline fun <T> MaybeSource<T>.collect(action: (T) -> Unit): Unit
template<typename T, typename MaybeSource, typename Action>
void collect(MaybeSource<T>& source, Action&& action) {
    // TODO: implement coroutine suspension
    // toChannel().consumeEach(action)
}

/**
 * Subscribes to this [ObservableSource] and performs the specified action for each received element.
 *
 * If [action] throws an exception at some point, the subscription is cancelled, and the exception is rethrown from
 * [collect]. Also, if the [ObservableSource] signals an error, that error is rethrown from [collect].
 */
// suspend inline fun <T> ObservableSource<T>.collect(action: (T) -> Unit): Unit
template<typename T, typename ObservableSource, typename Action>
void collect(ObservableSource<T>& source, Action&& action) {
    // TODO: implement coroutine suspension
    // toChannel().consumeEach(action)
}

// @PublishedApi
// internal fun <T> MaybeSource<T>.toChannel(): ReceiveChannel<T>
template<typename T, typename MaybeSource>
/* ReceiveChannel<T>* */ void* to_channel(MaybeSource<T>& source) {
    // val channel = SubscriptionChannel<T>()
    // subscribe(channel)
    // return channel
    // TODO: implement SubscriptionChannel and subscription
    return nullptr;
}

// @PublishedApi
// internal fun <T> ObservableSource<T>.toChannel(): ReceiveChannel<T>
template<typename T, typename ObservableSource>
/* ReceiveChannel<T>* */ void* to_channel(ObservableSource<T>& source) {
    // val channel = SubscriptionChannel<T>()
    // subscribe(channel)
    // return channel
    // TODO: implement SubscriptionChannel and subscription
    return nullptr;
}

// @Suppress("INVISIBLE_REFERENCE", "INVISIBLE_MEMBER")
// private class SubscriptionChannel<T> :
//     BufferedChannel<T>(capacity = Channel.UNLIMITED), Observer<T>, MaybeObserver<T>
template<typename T>
class SubscriptionChannel /* : public BufferedChannel<T>, public Observer<T>, public MaybeObserver<T> */ {
private:
    // private val _subscription = atomic<Disposable?>(null)
    /* atomic<Disposable*> */ void* subscription_; // TODO: implement atomic

public:
    SubscriptionChannel() {
        // Initialize with Channel.UNLIMITED capacity
        // TODO: implement BufferedChannel construction
    }

    // @Suppress("CANNOT_OVERRIDE_INVISIBLE_MEMBER")
    // override fun onClosedIdempotent()
    void on_closed_idempotent() {
        // _subscription.getAndSet(null)?.dispose() // dispose exactly once
        // TODO: implement atomic getAndSet and Disposable.dispose()
    }

    // Observer override
    // override fun onSubscribe(sub: Disposable)
    void on_subscribe(/* Disposable* */ void* sub) {
        // _subscription.value = sub
        // TODO: implement atomic value assignment
    }

    // override fun onSuccess(t: T & Any)
    void on_success(const T& t) {
        // trySend(t)
        // close(cause = null)
        // TODO: implement trySend and close
    }

    // override fun onNext(t: T & Any)
    void on_next(const T& t) {
        // trySend(t) // Safe to ignore return value here, expectedly racing with cancellation
        // TODO: implement trySend
    }

    // override fun onComplete()
    void on_complete() {
        // close(cause = null)
        // TODO: implement close
    }

    // override fun onError(e: Throwable)
    void on_error(const std::exception& e) {
        // close(cause = e)
        // TODO: implement close with exception
    }
};

// @Deprecated(message = "Deprecated in the favour of Flow", level = DeprecationLevel.HIDDEN) // ERROR in 1.4.0, HIDDEN in 1.6.0
// fun <T> ObservableSource<T & Any>.openSubscription(): ReceiveChannel<T>
template<typename T, typename ObservableSource>
[[deprecated("Deprecated in the favour of Flow")]]
/* ReceiveChannel<T>* */ void* open_subscription(ObservableSource<T>& source) {
    // val channel = SubscriptionChannel<T>()
    // subscribe(channel)
    // return channel
    // TODO: implement
    return nullptr;
}

// @Deprecated(message = "Deprecated in the favour of Flow", level = DeprecationLevel.HIDDEN) // ERROR in 1.4.0, HIDDEN in 1.6.0
// fun <T> MaybeSource<T & Any>.openSubscription(): ReceiveChannel<T>
template<typename T, typename MaybeSource>
[[deprecated("Deprecated in the favour of Flow")]]
/* ReceiveChannel<T>* */ void* open_subscription(MaybeSource<T>& source) {
    // val channel = SubscriptionChannel<T>()
    // subscribe(channel)
    // return channel
    // TODO: implement
    return nullptr;
}

} // namespace rx2
} // namespace coroutines
} // namespace kotlinx

/*
 * TODO List for semantic implementation:
 *
 * 1. Implement BufferedChannel<T> with UNLIMITED capacity
 * 2. Implement Observer<T> and MaybeObserver<T> interfaces from RxJava
 * 3. Implement atomic<T> for thread-safe Disposable storage
 * 4. Implement Disposable interface and dispose() method
 * 5. Implement ReceiveChannel<T> interface
 * 6. Implement trySend() and close() channel operations
 * 7. Implement consumeEach() for channel iteration
 * 8. Implement multiple inheritance for SubscriptionChannel
 * 9. Handle T & Any intersection types (non-nullable guarantees)
 * 10. Implement proper exception propagation through channels
 * 11. Implement subscribe() mechanism for MaybeSource and ObservableSource
 */
