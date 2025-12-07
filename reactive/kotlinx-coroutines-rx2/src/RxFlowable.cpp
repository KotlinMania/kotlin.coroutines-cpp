// Transliterated from: reactive/kotlinx-coroutines-rx2/src/RxFlowable.kt

// @file:Suppress("INVISIBLE_REFERENCE", "INVISIBLE_MEMBER")

// TODO: #include <io/reactivex/Flowable.hpp>
// TODO: #include <kotlinx/coroutines/CoroutineScope.hpp>
// TODO: #include <kotlinx/coroutines/channels/ProducerScope.hpp>
// TODO: #include <kotlinx/coroutines/reactive/publishInternal.hpp>
// TODO: #include <kotlin/coroutines/CoroutineContext.hpp>
// TODO: #include <kotlin/internal/LowPriorityInOverloadResolution.hpp>

namespace kotlinx {
namespace coroutines {
namespace rx2 {

/**
 * Creates cold [flowable][Flowable] that will run a given [block] in a coroutine.
 * Every time the returned flowable is subscribed, it starts a new coroutine.
 *
 * Coroutine emits ([ObservableEmitter.onNext]) values with `send`, completes ([ObservableEmitter.onComplete])
 * when the coroutine completes or channel is explicitly closed and emits error ([ObservableEmitter.onError])
 * if coroutine throws an exception or closes channel with a cause.
 * Unsubscribing cancels running coroutine.
 *
 * Invocations of `send` are suspended appropriately when subscribers apply back-pressure and to ensure that
 * `onNext` is not invoked concurrently.
 *
 * Coroutine context can be specified with [context] argument.
 * If the context does not have any dispatcher nor any other [ContinuationInterceptor], then [Dispatchers.Default] is used.
 * Method throws [IllegalArgumentException] if provided [context] contains a [Job] instance.
 *
 * **Note: This is an experimental api.** Behaviour of publishers that work as children in a parent scope with respect
 */
// fun <T: Any> rxFlowable(context: CoroutineContext = EmptyCoroutineContext, @BuilderInference block: suspend ProducerScope<T>.() -> Unit): Flowable<T>
template<typename T, typename Block>
/* Flowable<T> */ void* rx_flowable(/* CoroutineContext */ const void* context /* = EmptyCoroutineContext */, Block&& block) {
    // require(context[Job] === null) { "Flowable context cannot contain job in it." +
    //         "Its lifecycle should be managed via Disposable handle. Had $context" }
    // return Flowable.fromPublisher(publishInternal(GlobalScope, context, RX_HANDLER, block))
    // TODO: implement context validation, publishInternal, and Flowable.fromPublisher
    return nullptr;
}

// @Deprecated(
//     message = "CoroutineScope.rxFlowable is deprecated in favour of top-level rxFlowable",
//     level = DeprecationLevel.HIDDEN,
//     replaceWith = ReplaceWith("rxFlowable(context, block)")
// ) // Since 1.3.0, will be error in 1.3.1 and hidden in 1.4.0
// @LowPriorityInOverloadResolution
template<typename T, typename Block>
[[deprecated("CoroutineScope.rxFlowable is deprecated in favour of top-level rxFlowable")]]
/* Flowable<T> */ void* rx_flowable_scoped(
    /* CoroutineScope* */ void* scope,
    /* CoroutineContext */ const void* context /* = EmptyCoroutineContext */,
    Block&& block)
{
    // return Flowable.fromPublisher(publishInternal(this, context, RX_HANDLER, block))
    // TODO: implement publishInternal with scoped execution
    return nullptr;
}

// private val RX_HANDLER: (Throwable, CoroutineContext) -> Unit = ::handleUndeliverableException
// TODO: implement RX_HANDLER as function pointer or lambda
using RxHandler = void (*)(const std::exception&, const void*);
static const RxHandler kRxHandler = &handle_undeliverableException;

} // namespace rx2
} // namespace coroutines
} // namespace kotlinx

/*
 * TODO List for semantic implementation:
 *
 * 1. Implement Flowable.fromPublisher()
 * 2. Implement publishInternal from kotlinx.coroutines.reactive
 * 3. Implement ProducerScope<T> with send() operation
 * 4. Implement Job validation in CoroutineContext
 * 5. Implement GlobalScope singleton
 * 6. Implement RX_HANDLER as function reference
 * 7. Implement handleUndeliverableException
 * 8. Handle @BuilderInference annotation semantics
 * 9. Handle @LowPriorityInOverloadResolution for scoped version
 * 10. Implement backpressure handling
 */
