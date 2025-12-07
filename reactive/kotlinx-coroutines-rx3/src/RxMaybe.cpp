// Transliterated from: reactive/kotlinx-coroutines-rx3/src/RxMaybe.cpp

namespace kotlinx {
namespace coroutines {
namespace rx3 {

// TODO: #include equivalent
// import io.reactivex.rxjava3.core.*
// import kotlinx.coroutines.*
// import kotlin.coroutines.*

/**
 * Creates cold [maybe][Maybe] that will run a given [block] in a coroutine and emits its result.
 * If [block] result is `null`, [onComplete][MaybeObserver.onComplete] is invoked without a value.
 * Every time the returned observable is subscribed, it starts a new coroutine.
 * Unsubscribing cancels running coroutine.
 * Coroutine context can be specified with [context] argument.
 * If the context does not have any dispatcher nor any other [ContinuationInterceptor], then [Dispatchers.Default] is used.
 * Method throws [IllegalArgumentException] if provided [context] contains a [Job] instance.
 */
template<typename T, typename Block>
Maybe<T>* rx_maybe(
    const CoroutineContext& context = kEmptyCoroutineContext,
    Block&& block
) {
    // fun <T> rxMaybe(
    //     context: CoroutineContext = EmptyCoroutineContext,
    //     block: suspend CoroutineScope.() -> T?
    // ): Maybe<T & Any> {
    //     require(context[Job] === null) { "Maybe context cannot contain job in it." +
    //             "Its lifecycle should be managed via Disposable handle. Had $context" }
    //     return rxMaybeInternal(GlobalScope, context, block)
    // }

    // TODO: Validate context doesn't contain Job
    // TODO: Call rx_maybe_internal
    return nullptr;
}

template<typename T, typename Block>
Maybe<T>* rx_maybe_internal(
    CoroutineScope* scope, // support for legacy rxMaybe in scope
    const CoroutineContext& context,
    Block&& block
) {
    // private fun <T> rxMaybeInternal(
    //     scope: CoroutineScope, // support for legacy rxMaybe in scope
    //     context: CoroutineContext,
    //     block: suspend CoroutineScope.() -> T?
    // ): Maybe<T & Any> = Maybe.create { subscriber ->
    //     val newContext = scope.newCoroutineContext(context)
    //     val coroutine = RxMaybeCoroutine(newContext, subscriber)
    //     subscriber.setCancellable(RxCancellable(coroutine))
    //     coroutine.start(CoroutineStart.DEFAULT, coroutine, block)
    // }

    // TODO: Implement Maybe.create
    // TODO: Create new context
    // TODO: Create RxMaybeCoroutine
    // TODO: Set cancellable
    // TODO: Start coroutine
    return nullptr;
}

template<typename T>
class RxMaybeCoroutine : public AbstractCoroutine<T*> {
private:
    MaybeEmitter<T>* subscriber_;

public:
    RxMaybeCoroutine(
        const CoroutineContext& parent_context,
        MaybeEmitter<T>* subscriber
    ) : AbstractCoroutine<T*>(parent_context, false, true),
        subscriber_(subscriber) {}

    void on_completed(T* value) override {
        // override fun onCompleted(value: T?) {
        //     try {
        //         if (value == null) subscriber.onComplete() else subscriber.onSuccess(value)
        //     } catch (e: Throwable) {
        //         handleUndeliverableException(e, context)
        //     }
        // }
        try {
            if (value == nullptr) {
                subscriber_->on_complete();
            } else {
                subscriber_->on_success(*value);
            }
        } catch (const std::exception& e) {
            handle_undeliverable_exception(e, get_context());
        }
    }

    void on_cancelled(const std::exception& cause, bool handled) override {
        // override fun onCancelled(cause: Throwable, handled: Boolean) {
        //     try {
        //         if (subscriber.tryOnError(cause)) {
        //             return
        //         }
        //     } catch (e: Throwable) {
        //         cause.addSuppressed(e)
        //     }
        //     handleUndeliverableException(cause, context)
        // }
        try {
            if (subscriber_->try_on_error(cause)) {
                return;
            }
        } catch (const std::exception& e) {
            // TODO: cause.addSuppressed(e)
        }
        handle_undeliverable_exception(cause, get_context());
    }
};

} // namespace rx3
} // namespace coroutines
} // namespace kotlinx

/*
 * TODO: Semantic Implementation Tasks
 *
 * 1. Implement Maybe<T> type from RxJava3
 * 2. Implement MaybeEmitter<T> interface
 * 3. Implement AbstractCoroutine<T*> base class
 * 4. Implement CoroutineScope and CoroutineContext
 * 5. Implement kEmptyCoroutineContext constant
 * 6. Implement context validation (no Job allowed)
 * 7. Implement RxCancellable integration
 * 8. Implement coroutine start mechanism
 * 9. Implement CoroutineStart.DEFAULT
 * 10. Handle nullable return values (T?)
 * 11. Add exception suppression mechanism
 * 12. Add unit tests
 */
