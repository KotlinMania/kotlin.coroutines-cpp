// Transliterated from: reactive/kotlinx-coroutines-rx2/src/RxObservable.kt

// TODO: #include <io/reactivex/Observable.hpp>
// TODO: #include <io/reactivex/ObservableEmitter.hpp>
// TODO: #include <io/reactivex/exceptions/UndeliverableException.hpp>
// TODO: #include <kotlinx/atomicfu/atomic.hpp>
// TODO: #include <kotlinx/coroutines/CoroutineScope.hpp>
// TODO: #include <kotlinx/coroutines/channels/ProducerScope.hpp>
// TODO: #include <kotlinx/coroutines/channels/SendChannel.hpp>
// TODO: #include <kotlinx/coroutines/channels/ChannelResult.hpp>
// TODO: #include <kotlinx/coroutines/internal/unwrap.hpp>
// TODO: #include <kotlinx/coroutines/selects/SelectClause2.hpp>
// TODO: #include <kotlinx/coroutines/sync/Mutex.hpp>
// TODO: #include <kotlin/coroutines/CoroutineContext.hpp>

namespace kotlinx {
namespace coroutines {
namespace rx2 {

/**
 * Creates cold [observable][Observable] that will run a given [block] in a coroutine.
 * Every time the returned observable is subscribed, it starts a new coroutine.
 *
 * Coroutine emits ([ObservableEmitter.onNext]) values with `send`, completes ([ObservableEmitter.onComplete])
 * when the coroutine completes or channel is explicitly closed and emits error ([ObservableEmitter.onError])
 * if coroutine throws an exception or closes channel with a cause.
 * Unsubscribing cancels running coroutine.
 *
 * Invocations of `send` are suspended appropriately to ensure that `onNext` is not invoked concurrently.
 * Note that Rx 2.x [Observable] **does not support backpressure**.
 *
 * Coroutine context can be specified with [context] argument.
 * If the context does not have any dispatcher nor any other [ContinuationInterceptor], then [Dispatchers.Default] is used.
 * Method throws [IllegalArgumentException] if provided [context] contains a [Job] instance.
 */
// fun <T : Any> rxObservable(context: CoroutineContext = EmptyCoroutineContext, @BuilderInference block: suspend ProducerScope<T>.() -> Unit): Observable<T>
template<typename T, typename Block>
/* Observable<T> */ void* rx_observable(/* CoroutineContext */ const void* context /* = EmptyCoroutineContext */, Block&& block) {
    // require(context[Job] === null) { "Observable context cannot contain job in it." +
    //         "Its lifecycle should be managed via Disposable handle. Had $context" }
    // return rxObservableInternal(GlobalScope, context, block)
    // TODO: implement context validation and rxObservableInternal
    return nullptr;
}

// private fun <T : Any> rxObservableInternal(scope: CoroutineScope, context: CoroutineContext, block: suspend ProducerScope<T>.() -> Unit): Observable<T>
template<typename T, typename Block>
/* Observable<T> */ void* rx_observable_internal(
    /* CoroutineScope* */ void* scope,
    /* CoroutineContext */ const void* context,
    Block&& block)
{
    // return Observable.create { subscriber ->
    //     val newContext = scope.newCoroutineContext(context)
    //     val coroutine = RxObservableCoroutine(newContext, subscriber)
    //     subscriber.setCancellable(RxCancellable(coroutine))
    //     coroutine.start(CoroutineStart.DEFAULT, coroutine, block)
    // }
    // TODO: implement Observable.create and coroutine launching
    return nullptr;
}

// private const val OPEN = 0        // open channel, still working
// private const val CLOSED = -1     // closed, but have not signalled onCompleted/onError yet
// private const val SIGNALLED = -2  // already signalled subscriber onCompleted/onError
static const int kOpen = 0;
static const int kClosed = -1;
static const int kSignalled = -2;

// private class RxObservableCoroutine<T : Any>(parentContext: CoroutineContext, private val subscriber: ObservableEmitter<T>)
//     : AbstractCoroutine<Unit>(parentContext, false, true), ProducerScope<T>
template<typename T>
class RxObservableCoroutine /* : public AbstractCoroutine<void>, public ProducerScope<T> */ {
private:
    /* CoroutineContext */ const void* parent_context_;
    /* ObservableEmitter<T>* */ void* subscriber_;
    // private val _signal = atomic(OPEN)
    /* atomic<int> */ int signal_; // TODO: implement atomic
    // private val mutex: Mutex = Mutex()
    /* Mutex */ void* mutex_; // TODO: implement Mutex

public:
    RxObservableCoroutine(/* CoroutineContext */ const void* parent_context, /* ObservableEmitter<T>* */ void* subscriber)
        : parent_context_(parent_context), subscriber_(subscriber), signal_(kOpen)
    {
        // Initialize AbstractCoroutine with (parentContext, false, true)
        // TODO: implement AbstractCoroutine and Mutex construction
    }

    // override val channel: SendChannel<T> get() = this
    /* SendChannel<T>& */ void* get_channel() {
        return this;
    }

    // override val isClosedForSend: Boolean get() = !isActive
    bool is_closed_for_send() const {
        // return !isActive
        // TODO: implement isActive check
        return false;
    }

    // override fun close(cause: Throwable?): Boolean
    bool close(const std::exception* cause = nullptr) {
        // return cancelCoroutine(cause)
        // TODO: implement cancelCoroutine
        return false;
    }

    // override fun invokeOnClose(handler: (Throwable?) -> Unit)
    template<typename Handler>
    void invoke_on_close(Handler&& handler) {
        // throw UnsupportedOperationException("RxObservableCoroutine doesn't support invokeOnClose")
        throw std::runtime_error("RxObservableCoroutine doesn't support invokeOnClose");
    }

    // override val onSend: SelectClause2<T, SendChannel<T>>
    // TODO: implement SelectClause2 support

    // override fun trySend(element: T): ChannelResult<Unit>
    /* ChannelResult<void> */ void* try_send(const T& element) {
        // if (!mutex.tryLock()) {
        //     return ChannelResult.failure()
        // } else {
        //     when (val throwable = doLockedNext(element)) {
        //         null -> ChannelResult.success(Unit)
        //         else -> ChannelResult.closed(throwable)
        //     }
        // }
        // TODO: implement try_send with mutex
        return nullptr;
    }

    // override suspend fun send(element: T)
    void send(const T& element) {
        // TODO: implement coroutine suspension
        // mutex.lock()
        // doLockedNext(element)?.let { throw it }
    }

    // private fun doLockedNext(elem: T): Throwable?
    std::exception* do_locked_next(const T& elem) {
        // TODO: implement doLockedNext logic
        // Check if active, call subscriber.onNext(elem), handle exceptions
        return nullptr;
    }

    // private fun unlockAndCheckCompleted()
    void unlock_and_check_completed() {
        // mutex.unlock()
        // if (!isActive && mutex.tryLock())
        //     doLockedSignalCompleted(completionCause, completionCauseHandled)
        // TODO: implement unlock and completion check
    }

    // private fun doLockedSignalCompleted(cause: Throwable?, handled: Boolean)
    void do_locked_signal_completed(const std::exception* cause, bool handled) {
        // TODO: implement signal completion logic
        // Handle SIGNALLED state, call subscriber.onComplete() or subscriber.onError()
    }

    // private fun signalCompleted(cause: Throwable?, handled: Boolean)
    void signal_completed(const std::exception* cause, bool handled) {
        // if (!_signal.compareAndSet(OPEN, CLOSED)) return
        // if (mutex.tryLock())
        //     doLockedSignalCompleted(cause, handled)
        // TODO: implement signal_completed
    }

    // override fun onCompleted(value: Unit)
    void on_completed(/* Unit */ void* value) {
        signal_completed(nullptr, false);
    }

    // override fun onCancelled(cause: Throwable, handled: Boolean)
    void on_cancelled(const std::exception& cause, bool handled) {
        signal_completed(&cause, handled);
    }
};

// @Deprecated(
//     message = "CoroutineScope.rxObservable is deprecated in favour of top-level rxObservable",
//     level = DeprecationLevel.HIDDEN,
//     replaceWith = ReplaceWith("rxObservable(context, block)")
// ) // Since 1.3.0, will be error in 1.3.1 and hidden in 1.4.0
template<typename T, typename Block>
[[deprecated("CoroutineScope.rxObservable is deprecated in favour of top-level rxObservable")]]
/* Observable<T> */ void* rx_observable_scoped(
    /* CoroutineScope* */ void* scope,
    /* CoroutineContext */ const void* context /* = EmptyCoroutineContext */,
    Block&& block)
{
    return rx_observable_internal<T>(scope, context, std::forward<Block>(block));
}

} // namespace rx2
} // namespace coroutines
} // namespace kotlinx

/*
 * TODO List for semantic implementation:
 *
 * 1. Implement Observable.create() factory method
 * 2. Implement ObservableEmitter<T> interface
 * 3. Implement AbstractCoroutine<void> and ProducerScope<T> multiple inheritance
 * 4. Implement atomic<int> for _signal state machine
 * 5. Implement Mutex for thread-safe onNext invocations
 * 6. Implement SendChannel<T> interface
 * 7. Implement SelectClause2<T, SendChannel<T>> for select support
 * 8. Implement ChannelResult<Unit> for trySend
 * 9. Implement coroutine suspension for send()
 * 10. Implement doLockedNext, doLockedSignalCompleted state machines
 * 11. Implement UndeliverableException handling
 * 12. Implement unwrap() for exception handling
 * 13. Implement isActive, completionCause, completionCauseHandled from AbstractCoroutine
 * 14. Handle T: Any constraint (non-nullable type)
 */
