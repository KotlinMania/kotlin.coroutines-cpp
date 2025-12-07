// Transliterated from: reactive/kotlinx-coroutines-rx3/src/RxCompletable.cpp

namespace kotlinx {
namespace coroutines {
namespace rx3 {

// TODO: #include equivalent
// import io.reactivex.rxjava3.core.*
// import kotlinx.coroutines.*
// import kotlin.coroutines.*

/**
 * Creates cold [Completable] that runs a given [block] in a coroutine and emits its result.
 * Every time the returned completable is subscribed, it starts a new coroutine.
 * Unsubscribing cancels running coroutine.
 * Coroutine context can be specified with [context] argument.
 * If the context does not have any dispatcher nor any other [ContinuationInterceptor], then [Dispatchers.Default] is used.
 * Method throws [IllegalArgumentException] if provided [context] contains a [Job] instance.
 */
template<typename Block>
Completable* rx_completable(
    const CoroutineContext& context = kEmptyCoroutineContext,
    Block&& block
) {
    // fun rxCompletable(
    //     context: CoroutineContext = EmptyCoroutineContext,
    //     block: suspend CoroutineScope.() -> Unit
    // ): Completable {
    //     require(context[Job] === null) { "Completable context cannot contain job in it." +
    //             "Its lifecycle should be managed via Disposable handle. Had $context" }
    //     return rxCompletableInternal(GlobalScope, context, block)
    // }

    // TODO: Validate context doesn't contain Job
    // TODO: Call rx_completable_internal
    return nullptr;
}

template<typename Block>
Completable* rx_completable_internal(
    CoroutineScope* scope, // support for legacy rxCompletable in scope
    const CoroutineContext& context,
    Block&& block
) {
    // private fun rxCompletableInternal(
    //     scope: CoroutineScope, // support for legacy rxCompletable in scope
    //     context: CoroutineContext,
    //     block: suspend CoroutineScope.() -> Unit
    // ): Completable = Completable.create { subscriber ->
    //     val newContext = scope.newCoroutineContext(context)
    //     val coroutine = RxCompletableCoroutine(newContext, subscriber)
    //     subscriber.setCancellable(RxCancellable(coroutine))
    //     coroutine.start(CoroutineStart.DEFAULT, coroutine, block)
    // }

    // TODO: Implement Completable.create
    // TODO: Create new context
    // TODO: Create RxCompletableCoroutine
    // TODO: Set cancellable
    // TODO: Start coroutine
    return nullptr;
}

class RxCompletableCoroutine : public AbstractCoroutine<void> {
private:
    CompletableEmitter* subscriber_;

public:
    RxCompletableCoroutine(
        const CoroutineContext& parent_context,
        CompletableEmitter* subscriber
    ) : AbstractCoroutine<void>(parent_context, false, true),
        subscriber_(subscriber) {}

    void on_completed(void /* value */) override {
        // override fun onCompleted(value: Unit) {
        //     try {
        //         subscriber.onComplete()
        //     } catch (e: Throwable) {
        //         handleUndeliverableException(e, context)
        //     }
        // }
        try {
            subscriber_->on_complete();
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
 * 1. Implement Completable type from RxJava3
 * 2. Implement CompletableEmitter interface
 * 3. Implement AbstractCoroutine base class
 * 4. Implement CoroutineScope and CoroutineContext
 * 5. Implement kEmptyCoroutineContext constant
 * 6. Implement context validation (no Job allowed)
 * 7. Implement RxCancellable integration
 * 8. Implement coroutine start mechanism
 * 9. Implement CoroutineStart.DEFAULT
 * 10. Add exception suppression mechanism
 * 11. Implement GlobalScope
 * 12. Add unit tests
 */
