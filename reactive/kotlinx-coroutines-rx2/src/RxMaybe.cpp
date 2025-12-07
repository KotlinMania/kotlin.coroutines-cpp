// Transliterated from: reactive/kotlinx-coroutines-rx2/src/RxMaybe.kt

// TODO: #include <io/reactivex/Maybe.hpp>
// TODO: #include <io/reactivex/MaybeEmitter.hpp>
// TODO: #include <kotlinx/coroutines/CoroutineScope.hpp>
// TODO: #include <kotlinx/coroutines/CoroutineStart.hpp>
// TODO: #include <kotlinx/coroutines/GlobalScope.hpp>
// TODO: #include <kotlinx/coroutines/AbstractCoroutine.hpp>
// TODO: #include <kotlin/coroutines/CoroutineContext.hpp>

namespace kotlinx {
namespace coroutines {
namespace rx2 {

/**
 * Creates cold [maybe][Maybe] that will run a given [block] in a coroutine and emits its result.
 * If [block] result is `null`, [onComplete][MaybeObserver.onComplete] is invoked without a value.
 * Every time the returned observable is subscribed, it starts a new coroutine.
 * Unsubscribing cancels running coroutine.
 * Coroutine context can be specified with [context] argument.
 * If the context does not have any dispatcher nor any other [ContinuationInterceptor], then [Dispatchers.Default] is used.
 * Method throws [IllegalArgumentException] if provided [context] contains a [Job] instance.
 */
// fun <T> rxMaybe(context: CoroutineContext = EmptyCoroutineContext, block: suspend CoroutineScope.() -> T?): Maybe<T>
template<typename T, typename Block>
/* Maybe<T> */ void* rx_maybe(/* CoroutineContext */ const void* context /* = EmptyCoroutineContext */, Block&& block) {
    // require(context[Job] === null) { "Maybe context cannot contain job in it." +
    //         "Its lifecycle should be managed via Disposable handle. Had $context" }
    // return rxMaybeInternal(GlobalScope, context, block)
    // TODO: implement context validation and rxMaybeInternal
    return nullptr;
}

// private fun <T> rxMaybeInternal(scope: CoroutineScope, context: CoroutineContext, block: suspend CoroutineScope.() -> T?): Maybe<T>
template<typename T, typename Block>
/* Maybe<T> */ void* rx_maybe_internal(
    /* CoroutineScope* */ void* scope,
    /* CoroutineContext */ const void* context,
    Block&& block)
{
    // return Maybe.create { subscriber ->
    //     val newContext = scope.newCoroutineContext(context)
    //     val coroutine = RxMaybeCoroutine(newContext, subscriber)
    //     subscriber.setCancellable(RxCancellable(coroutine))
    //     coroutine.start(CoroutineStart.DEFAULT, coroutine, block)
    // }
    // TODO: implement Maybe.create and coroutine launching
    return nullptr;
}

// private class RxMaybeCoroutine<T>(parentContext: CoroutineContext, private val subscriber: MaybeEmitter<T>)
//     : AbstractCoroutine<T>(parentContext, false, true)
template<typename T>
class RxMaybeCoroutine /* : public AbstractCoroutine<T> */ {
private:
    /* CoroutineContext */ const void* parent_context_;
    /* MaybeEmitter<T>* */ void* subscriber_;

public:
    RxMaybeCoroutine(/* CoroutineContext */ const void* parent_context, /* MaybeEmitter<T>* */ void* subscriber)
        : parent_context_(parent_context), subscriber_(subscriber)
    {
        // Initialize AbstractCoroutine with (parentContext, false, true)
        // TODO: implement AbstractCoroutine construction
    }

    // override fun onCompleted(value: T)
    void on_completed(T* value) {
        try {
            // if (value == null) subscriber.onComplete() else subscriber.onSuccess(value)
            // TODO: implement subscriber.onComplete() and subscriber.onSuccess()
        } catch (const std::exception& e) {
            // handleUndeliverableException(e, context)
            // TODO: implement handleUndeliverableException
        }
    }

    // override fun onCancelled(cause: Throwable, handled: Boolean)
    void on_cancelled(const std::exception& cause, bool handled) {
        try {
            // if (subscriber.tryOnError(cause)) {
            //     return
            // }
            // TODO: implement subscriber.tryOnError()
        } catch (const std::exception& e) {
            // cause.addSuppressed(e)
            // TODO: implement addSuppressed
        }
        // handleUndeliverableException(cause, context)
        // TODO: implement handleUndeliverableException
    }
};

// @Deprecated(
//     message = "CoroutineScope.rxMaybe is deprecated in favour of top-level rxMaybe",
//     level = DeprecationLevel.HIDDEN,
//     replaceWith = ReplaceWith("rxMaybe(context, block)")
// ) // Since 1.3.0, will be error in 1.3.1 and hidden in 1.4.0
template<typename T, typename Block>
[[deprecated("CoroutineScope.rxMaybe is deprecated in favour of top-level rxMaybe")]]
/* Maybe<T> */ void* rx_maybe_scoped(
    /* CoroutineScope* */ void* scope,
    /* CoroutineContext */ const void* context /* = EmptyCoroutineContext */,
    Block&& block)
{
    return rx_maybe_internal<T>(scope, context, std::forward<Block>(block));
}

} // namespace rx2
} // namespace coroutines
} // namespace kotlinx

/*
 * TODO List for semantic implementation:
 *
 * 1. Implement Maybe.create() factory method
 * 2. Implement MaybeEmitter<T> interface (onComplete, onSuccess, tryOnError, setCancellable)
 * 3. Implement AbstractCoroutine<T> base class
 * 4. Implement nullable return type handling (T? -> T*)
 * 5. Implement CoroutineScope and newCoroutineContext
 * 6. Implement CoroutineStart.DEFAULT
 * 7. Implement Job validation in CoroutineContext
 * 8. Implement RxCancellable and Disposable integration
 * 9. Implement handleUndeliverableException
 * 10. Implement exception suppression (Throwable.addSuppressed)
 * 11. Implement GlobalScope singleton
 * 12. Implement coroutine.start() with block execution
 */
