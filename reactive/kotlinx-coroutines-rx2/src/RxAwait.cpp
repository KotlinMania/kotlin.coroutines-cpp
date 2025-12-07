// Transliterated from: reactive/kotlinx-coroutines-rx2/src/RxAwait.kt

// TODO: #include <io/reactivex/CompletableSource.hpp>
// TODO: #include <io/reactivex/MaybeSource.hpp>
// TODO: #include <io/reactivex/SingleSource.hpp>
// TODO: #include <io/reactivex/ObservableSource.hpp>
// TODO: #include <io/reactivex/disposables/Disposable.hpp>
// TODO: #include <kotlinx/coroutines/CancellableContinuation.hpp>
// TODO: #include <kotlinx/coroutines/CancellationException.hpp>
// TODO: #include <kotlinx/coroutines/Job.hpp>
// TODO: #include <kotlinx/coroutines/suspendCancellableCoroutine.hpp>
// TODO: #include <kotlin/coroutines/...>

namespace kotlinx {
namespace coroutines {
namespace rx2 {

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
// suspend fun CompletableSource.await(): Unit
template<typename CompletableSource>
void await(CompletableSource& source) {
    // TODO: implement suspendCancellableCoroutine
    // subscribe(object : CompletableObserver {
    //     override fun onSubscribe(d: Disposable) {
    //         cont.disposeOnCancellation(d)
    //     }
    //     override fun onComplete() {
    //         cont.resume(Unit)
    //     }
    //     override fun onError(e: Throwable) {
    //         cont.resumeWithException(e)
    //     }
    // })
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
// suspend fun <T> MaybeSource<T>.awaitSingleOrNull(): T?
template<typename T, typename MaybeSource>
T* await_single_or_null(MaybeSource<T>& source) {
    // TODO: implement suspendCancellableCoroutine
    // subscribe(object : MaybeObserver<T> {
    //     override fun onSubscribe(d: Disposable) {
    //         cont.disposeOnCancellation(d)
    //     }
    //     override fun onComplete() {
    //         cont.resume(null)
    //     }
    //     override fun onSuccess(t: T & Any) {
    //         cont.resume(t)
    //     }
    //     override fun onError(error: Throwable) {
    //         cont.resumeWithException(error)
    //     }
    // })
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
// suspend fun <T> MaybeSource<T>.awaitSingle(): T
template<typename T, typename MaybeSource>
T await_single(MaybeSource<T>& source) {
    // return awaitSingleOrNull() ?: throw NoSuchElementException()
    T* result = await_single_or_null(source);
    if (result == nullptr) {
        throw std::runtime_error("NoSuchElementException"); // TODO: proper exception type
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
 * @suppress
 */
// @Deprecated(
//     message = "Deprecated in favor of awaitSingleOrNull()",
//     level = DeprecationLevel.HIDDEN,
//     replaceWith = ReplaceWith("this.awaitSingleOrNull()")
// ) // Warning since 1.5, error in 1.6, hidden in 1.7
// TODO: implement coroutine suspension
// suspend fun <T> MaybeSource<T>.await(): T?
template<typename T, typename MaybeSource>
[[deprecated("Deprecated in favor of awaitSingleOrNull()")]]
T* await(MaybeSource<T>& source) {
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
 * @suppress
 */
// @Deprecated(
//     message = "Deprecated in favor of awaitSingleOrNull()",
//     level = DeprecationLevel.HIDDEN,
//     replaceWith = ReplaceWith("this.awaitSingleOrNull() ?: default")
// ) // Warning since 1.5, error in 1.6, hidden in 1.7
// TODO: implement coroutine suspension
// suspend fun <T> MaybeSource<T>.awaitOrDefault(default: T): T
template<typename T, typename MaybeSource>
[[deprecated("Deprecated in favor of awaitSingleOrNull()")]]
T await_or_default(MaybeSource<T>& source, const T& default_value) {
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
// suspend fun <T> SingleSource<T>.await(): T
template<typename T, typename SingleSource>
T await(SingleSource<T>& source) {
    // TODO: implement suspendCancellableCoroutine
    // subscribe(object : SingleObserver<T> {
    //     override fun onSubscribe(d: Disposable) {
    //         cont.disposeOnCancellation(d)
    //     }
    //     override fun onSuccess(t: T & Any) {
    //         cont.resume(t)
    //     }
    //     override fun onError(error: Throwable) {
    //         cont.resumeWithException(error)
    //     }
    // })
    return T{};
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
// TODO: implement coroutine suspension
// suspend fun <T> ObservableSource<T>.awaitFirst(): T
template<typename T, typename ObservableSource>
T await_first(ObservableSource<T>& source) {
    // return awaitOne(Mode.FIRST)
    return T{};
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
// TODO: implement coroutine suspension
// suspend fun <T> ObservableSource<T>.awaitFirstOrDefault(default: T): T
template<typename T, typename ObservableSource>
T await_first_or_default(ObservableSource<T>& source, const T& default_value) {
    // return awaitOne(Mode.FIRST_OR_DEFAULT, default)
    return default_value;
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
// suspend fun <T> ObservableSource<T>.awaitFirstOrNull(): T?
template<typename T, typename ObservableSource>
T* await_first_or_null(ObservableSource<T>& source) {
    // return awaitOne(Mode.FIRST_OR_DEFAULT)
    return nullptr;
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
// suspend fun <T> ObservableSource<T>.awaitFirstOrElse(defaultValue: () -> T): T
template<typename T, typename ObservableSource, typename DefaultValueFunc>
T await_first_or_else(ObservableSource<T>& source, DefaultValueFunc&& default_value_func) {
    // return awaitOne(Mode.FIRST_OR_DEFAULT) ?: defaultValue()
    T* result = await_first_or_null(source);
    return result ? *result : default_value_func();
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
// TODO: implement coroutine suspension
// suspend fun <T> ObservableSource<T>.awaitLast(): T
template<typename T, typename ObservableSource>
T await_last(ObservableSource<T>& source) {
    // return awaitOne(Mode.LAST)
    return T{};
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
// TODO: implement coroutine suspension
// suspend fun <T> ObservableSource<T>.awaitSingle(): T
template<typename T, typename ObservableSource>
T await_single(ObservableSource<T>& source) {
    // return awaitOne(Mode.SINGLE)
    return T{};
}

// ------------------------ private ------------------------

// internal fun CancellableContinuation<*>.disposeOnCancellation(d: Disposable)
template<typename CancellableContinuation, typename Disposable>
void dispose_on_cancellation(CancellableContinuation& cont, Disposable& d) {
    // invokeOnCancellation { d.dispose() }
    // TODO: implement invokeOnCancellation
}

// private enum class Mode(val s: String)
enum class Mode {
    kFirst,           // "awaitFirst"
    kFirstOrDefault,  // "awaitFirstOrDefault"
    kLast,            // "awaitLast"
    kSingle           // "awaitSingle"
};

// TODO: toString() for Mode enum

// private suspend fun <T> ObservableSource<T>.awaitOne(mode: Mode, default: T? = null): T
template<typename T, typename ObservableSource>
T await_one(ObservableSource<T>& source, Mode mode, T* default_value = nullptr) {
    // TODO: implement suspendCancellableCoroutine
    // subscribe(object : Observer<T> {
    //     private lateinit var subscription: Disposable
    //     private var value: T? = null
    //     private var seenValue = false
    //     override fun onSubscribe(sub: Disposable) { ... }
    //     override fun onNext(t: T & Any) { ... }
    //     override fun onComplete() { ... }
    //     override fun onError(e: Throwable) { ... }
    // })
    return T{};
}

} // namespace rx2
} // namespace coroutines
} // namespace kotlinx

/*
 * TODO List for semantic implementation:
 *
 * 1. Implement coroutine suspension mechanism (suspendCancellableCoroutine equivalent)
 * 2. Implement RxJava2 observer classes (CompletableObserver, MaybeObserver, SingleObserver, Observer)
 * 3. Implement Disposable interface and disposeOnCancellation mechanism
 * 4. Implement CancellableContinuation with resume/resumeWithException
 * 5. Implement proper exception types (NoSuchElementException, IllegalArgumentException, CancellationException)
 * 6. Implement subscription management and cancellation handling
 * 7. Implement Mode enum with proper toString() for error messages
 * 8. Implement awaitOne state machine for Observable (handling FIRST, FIRST_OR_DEFAULT, LAST, SINGLE modes)
 * 9. Handle T & Any intersection types (non-nullable guarantees)
 * 10. Implement template specialization for nullable/non-nullable return types
 */
