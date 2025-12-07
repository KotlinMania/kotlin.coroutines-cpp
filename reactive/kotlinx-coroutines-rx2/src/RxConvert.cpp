// Transliterated from: reactive/kotlinx-coroutines-rx2/src/RxConvert.kt

// TODO: #include <io/reactivex/Completable.hpp>
// TODO: #include <io/reactivex/Maybe.hpp>
// TODO: #include <io/reactivex/Single.hpp>
// TODO: #include <io/reactivex/Observable.hpp>
// TODO: #include <io/reactivex/Flowable.hpp>
// TODO: #include <io/reactivex/disposables/Disposable.hpp>
// TODO: #include <io/reactivex/disposables/Disposables.hpp>
// TODO: #include <kotlinx/coroutines/Job.hpp>
// TODO: #include <kotlinx/coroutines/Deferred.hpp>
// TODO: #include <kotlinx/coroutines/channels/ReceiveChannel.hpp>
// TODO: #include <kotlinx/coroutines/flow/Flow.hpp>
// TODO: #include <kotlinx/coroutines/reactive/asPublisher.hpp>
// TODO: #include <org/reactivestreams/Publisher.hpp>
// TODO: #include <java/util/concurrent/atomic/AtomicReference.hpp>
// TODO: #include <kotlin/coroutines/CoroutineContext.hpp>

namespace kotlinx {
namespace coroutines {
namespace rx2 {

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
// fun Job.asCompletable(context: CoroutineContext): Completable
template<typename Job>
/* Completable */ void* as_completable(Job& job, /* CoroutineContext */ const void* context) {
    // return rxCompletable(context) {
    //     this@asCompletable.join()
    // }
    // TODO: implement rxCompletable and Job.join()
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
// fun <T> Deferred<T?>.asMaybe(context: CoroutineContext): Maybe<T>
template<typename T, typename Deferred>
/* Maybe<T> */ void* as_maybe(Deferred<T*>& deferred, /* CoroutineContext */ const void* context) {
    // return rxMaybe(context) {
    //     this@asMaybe.await()
    // }
    // TODO: implement rxMaybe and Deferred.await()
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
// fun <T : Any> Deferred<T>.asSingle(context: CoroutineContext): Single<T>
template<typename T, typename Deferred>
/* Single<T> */ void* as_single(Deferred<T>& deferred, /* CoroutineContext */ const void* context) {
    // return rxSingle(context) {
    //     this@asSingle.await()
    // }
    // TODO: implement rxSingle and Deferred.await()
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
// fun <T: Any> ObservableSource<T>.asFlow(): Flow<T>
template<typename T, typename ObservableSource>
/* Flow<T> */ void* as_flow(ObservableSource<T>& source) {
    // return callbackFlow {
    //     val disposableRef = AtomicReference<Disposable>()
    //     val observer = object : Observer<T> {
    //         override fun onComplete() { close() }
    //         override fun onSubscribe(d: Disposable) { if (!disposableRef.compareAndSet(null, d)) d.dispose() }
    //         override fun onNext(t: T) { try { trySendBlocking(t) } catch (e: InterruptedException) { } }
    //         override fun onError(e: Throwable) { close(e) }
    //     }
    //     subscribe(observer)
    //     awaitClose { disposableRef.getAndSet(Disposables.disposed())?.dispose() }
    // }
    // TODO: implement callbackFlow and Observer
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
// fun <T: Any> Flow<T>.asObservable(context: CoroutineContext = EmptyCoroutineContext) : Observable<T>
template<typename T, typename Flow>
/* Observable<T> */ void* as_observable(Flow<T>& flow, /* CoroutineContext */ const void* context /* = EmptyCoroutineContext */) {
    // return Observable.create { emitter ->
    //     val job = GlobalScope.launch(Dispatchers.Unconfined + context, start = CoroutineStart.ATOMIC) {
    //         try {
    //             collect { value -> emitter.onNext(value) }
    //             emitter.onComplete()
    //         } catch (e: Throwable) {
    //             if (e !is CancellationException) {
    //                 if (!emitter.tryOnError(e)) { handleUndeliverableException(e, coroutineContext) }
    //             } else { emitter.onComplete() }
    //         }
    //     }
    //     emitter.setCancellable(RxCancellable(job))
    // }
    // TODO: implement Observable.create and Flow.collect()
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
// fun <T: Any> Flow<T>.asFlowable(context: CoroutineContext = EmptyCoroutineContext): Flowable<T>
template<typename T, typename Flow>
/* Flowable<T> */ void* as_flowable(Flow<T>& flow, /* CoroutineContext */ const void* context /* = EmptyCoroutineContext */) {
    // return Flowable.fromPublisher(asPublisher(context))
    // TODO: implement Flowable.fromPublisher and asPublisher
    return nullptr;
}

// @Deprecated(
//     message = "Deprecated in the favour of Flow",
//     level = DeprecationLevel.HIDDEN,
//     replaceWith = ReplaceWith("this.consumeAsFlow().asObservable(context)", "kotlinx.coroutines.flow.consumeAsFlow")
// ) // Deprecated since 1.4.0
template<typename T, typename ReceiveChannel>
[[deprecated("Deprecated in the favour of Flow")]]
/* Observable<T> */ void* as_observable_deprecated(ReceiveChannel<T>& channel, /* CoroutineContext */ const void* context) {
    // return rxObservable(context) {
    //     for (t in this@asObservable)
    //         send(t)
    // }
    // TODO: implement rxObservable
    return nullptr;
}

// @Suppress("UNUSED") // KT-42513
// @JvmOverloads // binary compatibility
// @JvmName("from")
// @Deprecated(level = DeprecationLevel.HIDDEN, message = "") // Since 1.4, was experimental prior to that
template<typename T, typename Flow>
[[deprecated("")]]
/* Flowable<T> */ void* _as_flowable(Flow<T>& flow, /* CoroutineContext */ const void* context /* = EmptyCoroutineContext */) {
    return as_flowable(flow, context);
}

// @Suppress("UNUSED") // KT-42513
// @JvmOverloads // binary compatibility
// @JvmName("from")
// @Deprecated(level = DeprecationLevel.HIDDEN, message = "") // Since 1.4, was experimental prior to that
template<typename T, typename Flow>
[[deprecated("")]]
/* Observable<T> */ void* _as_observable(Flow<T>& flow, /* CoroutineContext */ const void* context /* = EmptyCoroutineContext */) {
    return as_observable(flow, context);
}

} // namespace rx2
} // namespace coroutines
} // namespace kotlinx

/*
 * TODO List for semantic implementation:
 *
 * 1. Implement Job.join() for asCompletable
 * 2. Implement Deferred<T>.await() for asMaybe and asSingle
 * 3. Implement rxCompletable, rxMaybe, rxSingle, rxObservable builders
 * 4. Implement ObservableSource.subscribe with Observer
 * 5. Implement callbackFlow with awaitClose
 * 6. Implement AtomicReference<Disposable> for thread-safe disposal
 * 7. Implement Observable.create and Flowable.fromPublisher
 * 8. Implement Flow.collect() mechanism
 * 9. Implement GlobalScope.launch with CoroutineStart.ATOMIC
 * 10. Implement RxCancellable and handleUndeliverableException
 * 11. Implement trySendBlocking for ObservableSource to Flow conversion
 * 12. Implement Disposables.disposed() singleton
 * 13. Handle T & Any intersection types (non-nullable guarantees)
 * 14. Implement asPublisher for Flow to Flowable conversion
 */
