// Transliterated from: reactive/kotlinx-coroutines-rx3/src/RxAwait.cpp

namespace kotlinx {
namespace coroutines {
namespace rx3 {

// TODO: #include equivalent
// import io.reactivex.rxjava3.core.*
// import io.reactivex.rxjava3.disposables.Disposable
// import kotlinx.coroutines.CancellableContinuation
// import kotlinx.coroutines.CancellationException
// import kotlinx.coroutines.Job
// import kotlinx.coroutines.suspendCancellableCoroutine
// import kotlin.coroutines.*

// ------------------------ CompletableSource ------------------------

/**
 * Awaits for completion of this completable without blocking the thread.
 * Returns `Unit`, or throws the corresponding exception if this completable produces an error.
 *
 * This suspending function is cancellable. If the [Job] of the invoking coroutine is cancelled while this
 * suspending function is suspended, this function immediately resumes with [CancellationException] and disposes of its
 * subscription.
 */
// TODO: implement coroutine suspension
template<typename CompletableSource>
void await(CompletableSource& source) {
    // TODO: suspendCancellableCoroutine implementation
    // suspend fun CompletableSource.await(): Unit = suspendCancellableCoroutine { cont ->
    //     subscribe(object : CompletableObserver {
    //         override fun onSubscribe(d: Disposable) { cont.disposeOnCancellation(d) }
    //         override fun onComplete() { cont.resume(Unit) }
    //         override fun onError(e: Throwable) { cont.resumeWithException(e) }
    //     })
    // }
}

// ------------------------ MaybeSource ------------------------

/**
 * Awaits for completion of the [MaybeSource] without blocking the thread.
 * Returns the resulting value, or `null` if no value is produced, or throws the corresponding exception if this
 * [MaybeSource] produces an error.
 *
 * This suspending function is cancellable.
 * If the [Job] of the current coroutine is cancelled while this suspending function is waiting, this
 * function immediately resumes with [CancellationException] and disposes of its subscription.
 */
// TODO: implement coroutine suspension
template<typename T>
T* await_single_or_null(MaybeSource<T>& source) {
    // TODO: suspendCancellableCoroutine implementation
    // suspend fun <T> MaybeSource<T & Any>.awaitSingleOrNull(): T? = suspendCancellableCoroutine { cont ->
    //     subscribe(object : MaybeObserver<T & Any> {
    //         override fun onSubscribe(d: Disposable) { cont.disposeOnCancellation(d) }
    //         override fun onComplete() { cont.resume(null) }
    //         override fun onSuccess(t: T & Any) { cont.resume(t) }
    //         override fun onError(error: Throwable) { cont.resumeWithException(error) }
    //     })
    // }
    return nullptr;
}

/**
 * Awaits for completion of the [MaybeSource] without blocking the thread.
 * Returns the resulting value, or throws if either no value is produced or this [MaybeSource] produces an error.
 *
 * This suspending function is cancellable.
 * If the [Job] of the current coroutine is cancelled while this suspending function is waiting, this
 * function immediately resumes with [CancellationException] and disposes of its subscription.
 *
 * @throws NoSuchElementException if no elements were produced by this [MaybeSource].
 */
// TODO: implement coroutine suspension
template<typename T>
T await_single(MaybeSource<T>& source) {
    // suspend fun <T> MaybeSource<T & Any>.awaitSingle(): T = awaitSingleOrNull() ?: throw NoSuchElementException()
    T* result = await_single_or_null(source);
    if (result == nullptr) {
        throw std::runtime_error("NoSuchElementException");
    }
    return *result;
}

/**
 * Awaits for completion of the maybe without blocking a thread.
 * Returns the resulting value, null if no value was produced or throws the corresponding exception if this
 * maybe had produced error.
 *
 * This suspending function is cancellable.
 * If the [Job] of the current coroutine is cancelled while this suspending function is waiting, this function
 * immediately resumes with [CancellationException].
 *
 * ### Deprecation
 *
 * Deprecated in favor of [awaitSingleOrNull] in order to reflect that `null` can be returned to denote the absence of
 * a value, as opposed to throwing in such case.
 *
 * @suppress
 */
// @Deprecated(
//     message = "Deprecated in favor of awaitSingleOrNull()",
//     level = DeprecationLevel.HIDDEN,
//     replaceWith = ReplaceWith("this.awaitSingleOrNull()")
// ) // Warning since 1.5, error in 1.6, hidden in 1.7
// TODO: implement coroutine suspension
template<typename T>
[[deprecated("Deprecated in favor of awaitSingleOrNull()")]]
T* await(MaybeSource<T>& source) {
    // suspend fun <T> MaybeSource<T & Any>.await(): T? = awaitSingleOrNull()
    return await_single_or_null(source);
}

/**
 * Awaits for completion of the maybe without blocking a thread.
 * Returns the resulting value, [default] if no value was produced or throws the corresponding exception if this
 * maybe had produced error.
 *
 * This suspending function is cancellable.
 * If the [Job] of the current coroutine is cancelled while this suspending function is waiting, this function
 * immediately resumes with [CancellationException].
 *
 * ### Deprecation
 *
 * Deprecated in favor of [awaitSingleOrNull] for naming consistency (see the deprecation of [MaybeSource.await] for
 * details).
 *
 * @suppress
 */
// @Deprecated(
//     message = "Deprecated in favor of awaitSingleOrNull()",
//     level = DeprecationLevel.HIDDEN,
//     replaceWith = ReplaceWith("this.awaitSingleOrNull() ?: default")
// ) // Warning since 1.5, error in 1.6, hidden in 1.7
// TODO: implement coroutine suspension
template<typename T>
[[deprecated("Deprecated in favor of awaitSingleOrNull()")]]
T await_or_default(MaybeSource<T>& source, const T& default_value) {
    // suspend fun <T> MaybeSource<T & Any>.awaitOrDefault(default: T): T = awaitSingleOrNull() ?: default
    T* result = await_single_or_null(source);
    return result ? *result : default_value;
}

// ------------------------ SingleSource ------------------------

/**
 * Awaits for completion of the single value response without blocking the thread.
 * Returns the resulting value, or throws the corresponding exception if this response produces an error.
 *
 * This suspending function is cancellable.
 * If the [Job] of the current coroutine is cancelled while the suspending function is waiting, this
 * function immediately disposes of its subscription and resumes with [CancellationException].
 */
// TODO: implement coroutine suspension
template<typename T>
T await(SingleSource<T>& source) {
    // TODO: suspendCancellableCoroutine implementation
    // suspend fun <T> SingleSource<T & Any>.await(): T = suspendCancellableCoroutine { cont ->
    //     subscribe(object : SingleObserver<T & Any> {
    //         override fun onSubscribe(d: Disposable) { cont.disposeOnCancellation(d) }
    //         override fun onSuccess(t: T & Any) { cont.resume(t) }
    //         override fun onError(error: Throwable) { cont.resumeWithException(error) }
    //     })
    // }
    throw std::runtime_error("Not implemented");
}

// ------------------------ ObservableSource ------------------------

/**
 * Awaits the first value from the given [Observable] without blocking the thread and returns the resulting value, or,
 * if the observable has produced an error, throws the corresponding exception.
 *
 * This suspending function is cancellable.
 * If the [Job] of the current coroutine is cancelled while the suspending function is waiting, this
 * function immediately disposes of its subscription and resumes with [CancellationException].
 *
 * @throws NoSuchElementException if the observable does not emit any value
 */
// @Suppress("UNCHECKED_CAST")
// TODO: implement coroutine suspension
template<typename T>
T await_first(ObservableSource<T>& source) {
    // suspend fun <T> ObservableSource<T & Any>.awaitFirst(): T = awaitOne(Mode.FIRST) as T
    return await_one(source, Mode::kFirst, nullptr);
}

/**
 * Awaits the first value from the given [Observable], or returns the [default] value if none is emitted, without
 * blocking the thread, and returns the resulting value, or, if this observable has produced an error, throws the
 * corresponding exception.
 *
 * This suspending function is cancellable.
 * If the [Job] of the current coroutine is cancelled while the suspending function is waiting, this
 * function immediately disposes of its subscription and resumes with [CancellationException].
 */
// @Suppress("UNCHECKED_CAST")
// TODO: implement coroutine suspension
template<typename T>
T await_first_or_default(ObservableSource<T>& source, const T& default_value) {
    // suspend fun <T> ObservableSource<T & Any>.awaitFirstOrDefault(default: T): T =
    //     awaitOne(Mode.FIRST_OR_DEFAULT, default) as T
    return await_one(source, Mode::kFirstOrDefault, &default_value);
}

/**
 * Awaits the first value from the given [Observable], or returns `null` if none is emitted, without blocking the
 * thread, and returns the resulting value, or, if this observable has produced an error, throws the corresponding
 * exception.
 *
 * This suspending function is cancellable.
 * If the [Job] of the current coroutine is cancelled while the suspending function is waiting, this
 * function immediately disposes of its subscription and resumes with [CancellationException].
 */
// TODO: implement coroutine suspension
template<typename T>
T* await_first_or_null(ObservableSource<T>& source) {
    // suspend fun <T> ObservableSource<T & Any>.awaitFirstOrNull(): T? = awaitOne(Mode.FIRST_OR_DEFAULT)
    return await_one(source, Mode::kFirstOrDefault, nullptr);
}

/**
 * Awaits the first value from the given [Observable], or calls [defaultValue] to get a value if none is emitted,
 * without blocking the thread, and returns the resulting value, or, if this observable has produced an error, throws
 * the corresponding exception.
 *
 * This suspending function is cancellable.
 * If the [Job] of the current coroutine is cancelled while the suspending function is waiting, this
 * function immediately disposes of its subscription and resumes with [CancellationException].
 */
// TODO: implement coroutine suspension
template<typename T, typename Func>
T await_first_or_else(ObservableSource<T>& source, Func&& default_value) {
    // suspend fun <T> ObservableSource<T & Any>.awaitFirstOrElse(defaultValue: () -> T): T =
    //     awaitOne(Mode.FIRST_OR_DEFAULT) ?: defaultValue()
    T* result = await_one(source, Mode::kFirstOrDefault, nullptr);
    return result ? *result : default_value();
}

/**
 * Awaits the last value from the given [Observable] without blocking the thread and
 * returns the resulting value, or, if this observable has produced an error, throws the corresponding exception.
 *
 * This suspending function is cancellable.
 * If the [Job] of the current coroutine is cancelled while the suspending function is waiting, this
 * function immediately disposes of its subscription and resumes with [CancellationException].
 *
 * @throws NoSuchElementException if the observable does not emit any value
 */
// @Suppress("UNCHECKED_CAST")
// TODO: implement coroutine suspension
template<typename T>
T await_last(ObservableSource<T>& source) {
    // suspend fun <T> ObservableSource<T & Any>.awaitLast(): T = awaitOne(Mode.LAST) as T
    return await_one(source, Mode::kLast, nullptr);
}

/**
 * Awaits the single value from the given observable without blocking the thread and returns the resulting value, or,
 * if this observable has produced an error, throws the corresponding exception.
 *
 * This suspending function is cancellable.
 * If the [Job] of the current coroutine is cancelled while the suspending function is waiting, this
 * function immediately disposes of its subscription and resumes with [CancellationException].
 *
 * @throws NoSuchElementException if the observable does not emit any value
 * @throws IllegalArgumentException if the observable emits more than one value
 */
// @Suppress("UNCHECKED_CAST")
// TODO: implement coroutine suspension
template<typename T>
T await_single(ObservableSource<T>& source) {
    // suspend fun <T> ObservableSource<T & Any>.awaitSingle(): T = awaitOne(Mode.SINGLE) as T
    return await_one(source, Mode::kSingle, nullptr);
}

// ------------------------ private ------------------------

template<typename Continuation, typename Disposable>
void dispose_on_cancellation(Continuation& cont, Disposable& d) {
    // internal fun CancellableContinuation<*>.disposeOnCancellation(d: Disposable) =
    //     invokeOnCancellation { d.dispose() }
    // TODO: implement invokeOnCancellation
}

// @JvmField
enum class Mode {
    kFirst,           // FIRST("awaitFirst")
    kFirstOrDefault,  // FIRST_OR_DEFAULT("awaitFirstOrDefault")
    kLast,            // LAST("awaitLast")
    kSingle           // SINGLE("awaitSingle")
};

const char* mode_to_string(Mode m) {
    switch (m) {
        case Mode::kFirst: return "awaitFirst";
        case Mode::kFirstOrDefault: return "awaitFirstOrDefault";
        case Mode::kLast: return "awaitLast";
        case Mode::kSingle: return "awaitSingle";
        default: return "unknown";
    }
}

// TODO: implement coroutine suspension
template<typename T>
T* await_one(ObservableSource<T>& source, Mode mode, const T* default_value) {
    // TODO: suspendCancellableCoroutine implementation
    // private suspend fun <T> ObservableSource<T & Any>.awaitOne(
    //     mode: Mode,
    //     default: T? = null
    // ): T? = suspendCancellableCoroutine { cont ->
    //     subscribe(object : Observer<T & Any> {
    //         private lateinit var subscription: Disposable
    //         private var value: T? = null
    //         private var seenValue = false
    //
    //         override fun onSubscribe(sub: Disposable) {
    //             subscription = sub
    //             cont.invokeOnCancellation { sub.dispose() }
    //         }
    //
    //         override fun onNext(t: T & Any) {
    //             when (mode) {
    //                 Mode.FIRST, Mode.FIRST_OR_DEFAULT -> {
    //                     if (!seenValue) {
    //                         seenValue = true
    //                         cont.resume(t)
    //                         subscription.dispose()
    //                     }
    //                 }
    //                 Mode.LAST, Mode.SINGLE -> {
    //                     if (mode == Mode.SINGLE && seenValue) {
    //                         if (cont.isActive)
    //                             cont.resumeWithException(IllegalArgumentException("More than one onNext value for $mode"))
    //                         subscription.dispose()
    //                     } else {
    //                         value = t
    //                         seenValue = true
    //                     }
    //                 }
    //             }
    //         }
    //
    //         @Suppress("UNCHECKED_CAST")
    //         override fun onComplete() {
    //             if (seenValue) {
    //                 if (cont.isActive) cont.resume(value as T)
    //                 return
    //             }
    //             when {
    //                 mode == Mode.FIRST_OR_DEFAULT -> {
    //                     cont.resume(default as T)
    //                 }
    //                 cont.isActive -> {
    //                     cont.resumeWithException(NoSuchElementException("No value received via onNext for $mode"))
    //                 }
    //             }
    //         }
    //
    //         override fun onError(e: Throwable) {
    //             cont.resumeWithException(e)
    //         }
    //     })
    // }
    return nullptr;
}

} // namespace rx3
} // namespace coroutines
} // namespace kotlinx

/*
 * TODO: Semantic Implementation Tasks
 *
 * 1. Implement coroutine suspension mechanism (suspendCancellableCoroutine)
 * 2. Implement RxJava3 integration (CompletableSource, MaybeSource, SingleSource, ObservableSource)
 * 3. Implement Observer pattern adapters (CompletableObserver, MaybeObserver, SingleObserver, Observer)
 * 4. Implement Disposable handling and cancellation
 * 5. Implement continuation resume/resumeWithException mechanisms
 * 6. Implement proper exception handling and propagation
 * 7. Implement await_one template function with full Observer logic
 * 8. Add proper type constraints for template parameters
 * 9. Implement thread-safe state management
 * 10. Add unit tests for all await functions
 */
