// Transliterated from: reactive/kotlinx-coroutines-rx3/src/RxFlowable.cpp

namespace kotlinx {
namespace coroutines {
namespace rx3 {

// TODO: #include equivalent
// import io.reactivex.rxjava3.core.*
// import kotlinx.coroutines.*
// import kotlinx.coroutines.channels.*
// import kotlinx.coroutines.reactive.*
// import kotlin.coroutines.*

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
// @BuilderInference
template<typename T, typename Block>
Flowable<T>* rx_flowable(
    const CoroutineContext& context = kEmptyCoroutineContext,
    Block&& block
) {
    // fun <T: Any> rxFlowable(
    //     context: CoroutineContext = EmptyCoroutineContext,
    //     @BuilderInference block: suspend ProducerScope<T>.() -> Unit
    // ): Flowable<T> {
    //     require(context[Job] === null) { "Flowable context cannot contain job in it." +
    //             "Its lifecycle should be managed via Disposable handle. Had $context" }
    //     return Flowable.fromPublisher(publishInternal(GlobalScope, context, RX_HANDLER, block))
    // }

    // TODO: Validate context doesn't contain Job
    // TODO: Call publish_internal with RX_HANDLER
    // TODO: Wrap in Flowable.fromPublisher
    return nullptr;
}

// Handler function type: (Throwable, CoroutineContext) -> Unit
using RxHandler = void (*)(const std::exception&, const CoroutineContext&);

const RxHandler kRxHandler = &handle_undeliverable_exception;
// private val RX_HANDLER: (Throwable, CoroutineContext) -> Unit = ::handleUndeliverableException

} // namespace rx3
} // namespace coroutines
} // namespace kotlinx

/*
 * TODO: Semantic Implementation Tasks
 *
 * 1. Implement Flowable<T> type from RxJava3
 * 2. Implement ProducerScope<T> interface
 * 3. Implement publish_internal function from reactive module
 * 4. Implement Flowable.fromPublisher
 * 5. Implement context validation (no Job allowed)
 * 6. Implement GlobalScope reference
 * 7. Implement @BuilderInference annotation handling
 * 8. Implement backpressure handling
 * 9. Add proper exception handling
 * 10. Add unit tests
 */
