// Transliterated from: reactive/kotlinx-coroutines-rx3/src/RxConvert.cpp

namespace kotlinx {
namespace coroutines {
namespace rx3 {

// TODO: #include equivalent
// import io.reactivex.rxjava3.core.*
// import io.reactivex.rxjava3.disposables.*
// import kotlinx.coroutines.*
// import kotlinx.coroutines.channels.*
// import kotlinx.coroutines.flow.*
// import kotlinx.coroutines.reactive.*
// import org.reactivestreams.*
// import java.util.concurrent.atomic.*
// import kotlin.coroutines.*

/**
 * Converts this job to the hot reactive completable that signals
 * with [onCompleted][CompletableObserver.onComplete] when the corresponding job completes.
 *
 * Every subscriber gets the signal at the same time.
 * Unsubscribing from the resulting completable **does not** affect the original job in any way.
 *
 * **Note: This is an experimental api.** Conversion of coroutines primitives to reactive entities may change
 *    in the future to account for the concept of structured concurrency.
 *
 * @param context -- the coroutine context from which the resulting completable is going to be signalled
 */
Completable* as_completable(Job* job, const CoroutineContext& context) {
    // fun Job.asCompletable(context: CoroutineContext): Completable = rxCompletable(context) {
    //     this@asCompletable.join()
    // }
    // TODO: Implement
    return nullptr;
}

/**
 * Converts this deferred value to the hot reactive maybe that signals
 * [onComplete][MaybeEmitter.onComplete], [onSuccess][MaybeEmitter.onSuccess] or [onError][MaybeEmitter.onError].
 *
 * Every subscriber gets the same completion value.
 * Unsubscribing from the resulting maybe **does not** affect the original deferred value in any way.
 *
 * **Note: This is an experimental api.** Conversion of coroutines primitives to reactive entities may change
 *    in the future to account for the concept of structured concurrency.
 *
 * @param context -- the coroutine context from which the resulting maybe is going to be signalled
 */
template<typename T>
Maybe<T>* as_maybe(Deferred<T*>* deferred, const CoroutineContext& context) {
    // fun <T> Deferred<T?>.asMaybe(context: CoroutineContext): Maybe<T & Any> = rxMaybe(context) {
    //     this@asMaybe.await()
    // }
    // TODO: Implement
    return nullptr;
}

/**
 * Converts this deferred value to the hot reactive single that signals either
 * [onSuccess][SingleObserver.onSuccess] or [onError][SingleObserver.onError].
 *
 * Every subscriber gets the same completion value.
 * Unsubscribing from the resulting single **does not** affect the original deferred value in any way.
 *
 * **Note: This is an experimental api.** Conversion of coroutines primitives to reactive entities may change
 *    in the future to account for the concept of structured concurrency.
 *
 * @param context -- the coroutine context from which the resulting single is going to be signalled
 */
template<typename T>
Single<T>* as_single(Deferred<T>* deferred, const CoroutineContext& context) {
    // fun <T : Any> Deferred<T>.asSingle(context: CoroutineContext): Single<T> = rxSingle(context) {
    //     this@asSingle.await()
    // }
    // TODO: Implement
    return nullptr;
}

/**
 * Transforms given cold [ObservableSource] into cold [Flow].
 *
 * The resulting flow is _cold_, which means that [ObservableSource.subscribe] is called every time a terminal operator
 * is applied to the resulting flow.
 *
 * A channel with the [default][Channel.BUFFERED] buffer size is used. Use the [buffer] operator on the
 * resulting flow to specify a user-defined value and to control what happens when data is produced faster
 * than consumed, i.e. to control the back-pressure behavior. Check [callbackFlow] for more details.
 */
template<typename T>
Flow<T>* as_flow(ObservableSource<T>& source) {
    // fun <T: Any> ObservableSource<T>.asFlow(): Flow<T> = callbackFlow { ... }
    // TODO: Implement callbackFlow with Observer
    return nullptr;
}

/**
 * Converts the given flow to a cold observable.
 * The original flow is cancelled when the observable subscriber is disposed.
 *
 * An optional [context] can be specified to control the execution context of calls to [Observer] methods.
 * You can set a [CoroutineDispatcher] to confine them to a specific thread and/or various [ThreadContextElement] to
 * inject additional context into the caller thread. By default, the [Unconfined][Dispatchers.Unconfined] dispatcher
 * is used, so calls are performed from an arbitrary thread.
 */
template<typename T>
Observable<T>* as_observable(Flow<T>& flow, const CoroutineContext& context = kEmptyCoroutineContext) {
    // fun <T: Any> Flow<T>.asObservable(context: CoroutineContext = EmptyCoroutineContext) : Observable<T> = Observable.create { ... }
    // TODO: Implement Observable.create with flow collection
    return nullptr;
}

/**
 * Converts the given flow to a cold flowable.
 * The original flow is cancelled when the flowable subscriber is disposed.
 *
 * An optional [context] can be specified to control the execution context of calls to [Subscriber] methods.
 * You can set a [CoroutineDispatcher] to confine them to a specific thread and/or various [ThreadContextElement] to
 * inject additional context into the caller thread. By default, the [Unconfined][Dispatchers.Unconfined] dispatcher
 * is used, so calls are performed from an arbitrary thread.
 */
template<typename T>
Flowable<T>* as_flowable(Flow<T>& flow, const CoroutineContext& context = kEmptyCoroutineContext) {
    // fun <T: Any> Flow<T>.asFlowable(context: CoroutineContext = EmptyCoroutineContext): Flowable<T> =
    //     Flowable.fromPublisher(asPublisher(context))
    // TODO: Implement via asPublisher
    return nullptr;
}

// @Suppress("UNUSED") // KT-42513
// @JvmOverloads // binary compatibility
// @JvmName("from")
// @Deprecated(level = DeprecationLevel.HIDDEN, message = "") // Since 1.4, was experimental prior to that
template<typename T>
[[deprecated("Hidden API")]]
Flowable<T>* _as_flowable(Flow<T>& flow, const CoroutineContext& context = kEmptyCoroutineContext) {
    return as_flowable(flow, context);
}

// @Suppress("UNUSED") // KT-42513
// @JvmOverloads // binary compatibility
// @JvmName("from")
// @Deprecated(level = DeprecationLevel.HIDDEN, message = "") // Since 1.4, was experimental prior to that
template<typename T>
[[deprecated("Hidden API")]]
Observable<T>* _as_observable(Flow<T>& flow, const CoroutineContext& context = kEmptyCoroutineContext) {
    return as_observable(flow, context);
}

} // namespace rx3
} // namespace coroutines
} // namespace kotlinx

/*
 * TODO: Semantic Implementation Tasks
 *
 * 1. Implement Job, Deferred<T> types
 * 2. Implement asPublisher for Flow conversion
 * 3. Implement callbackFlow builder
 * 4. Implement Observable.create and Flowable.fromPublisher
 * 5. Implement AtomicReference for disposable management
 * 6. Implement GlobalScope.launch with ATOMIC start
 * 7. Implement RxCancellable integration
 * 8. Handle exceptions with tryOnError
 * 9. Implement thread-safe emission
 * 10. Add unit tests
 */
