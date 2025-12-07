#pragma once
#include <string>
#include <functional>
#include "kotlinx/coroutines/core_fwd.hpp"
#include "kotlinx/coroutines/Continuation.hpp"
#include "kotlinx/coroutines/Job.hpp"
#include "kotlinx/coroutines/CoroutineDispatcher.hpp"

namespace kotlinx {
namespace coroutines {

/**
 * Cancellable [continuation][Continuation] is a thread-safe continuation primitive with the support of
 * an asynchronous cancellation.
 *
 * Cancellable continuation can be [resumed][Continuation.resumeWith], but unlike regular [Continuation],
 * it also might be [cancelled][CancellableContinuation.cancel] explicitly or [implicitly][Job.cancel] via a parent [job][Job].
 *
 * If the continuation is cancelled successfully, it resumes with a [CancellationException] or
 * the specified cancel cause.
 *
 * ### Usage
 *
 * An instance of `CancellableContinuation` can only be obtained by the [suspendCancellableCoroutine] function.
 * The struct itself is for use and for implementation.
 *
 * A typical usages of this function is to suspend a coroutine while waiting for a result
 * from a callback or an external source of values that optionally supports cancellation:
 *
 * ```
 * fun <T> CompletableFuture<T>.await(): T = suspendCancellableCoroutine { c ->
 *     auto future = this
 *     future.whenComplete { result, throwable ->
 *         if (throwable != nullptr) {
 *             // Resume continuation with an exception if an external source failed
 *             c.resumeWithException(throwable)
 *         } else {
 *             // Resume continuation with a value if it was computed
 *             c.resume(result)
 *         }
 *     }
 *     // Cancel the computation if the continuation itself was cancelled because a caller of 'await' is cancelled
 *     c.invokeOnCancellation { future.cancel(true) }
 * }
 * ```
 *
 * ### Thread-safety
 *
 * Instances of [CancellableContinuation] are thread-safe and can be safely shared across multiple threads.
 * [CancellableContinuation] allows concurrent invocations of the [cancel] and [resume] pair, guaranteeing
 * that only one of these operations will succeed.
 * Concurrent invocations of [resume] methods lead to a [IllegalStateException] and are considered a programmatic error.
 * Concurrent invocations of [cancel] methods is permitted, and at most one of them succeeds.
 *
 * ### Prompt cancellation guarantee
 *
 * A cancellable continuation provides a **prompt cancellation guarantee**.
 *
 * If the [Job] of the coroutine that obtained a cancellable continuation was cancelled while this continuation was suspended it will not resume
 * successfully, even if [CancellableContinuation.resume] was already invoked but not yet executed.
 *
 * The cancellation of the coroutine's job is generally asynchronous with respect to the suspended coroutine.
 * The suspended coroutine is resumed with a call to its [Continuation.resumeWith] member function or to the
 * [resume][Continuation.resume] extension function.
 * However, when the coroutine is resumed, it does not immediately start executing but is passed to its
 * [CoroutineDispatcher] to schedule its execution when the dispatcher's resources become available for execution.
 * The job's cancellation can happen before, after, and concurrently with the call to `resume`. In any
 * case, prompt cancellation guarantees that the coroutine will not resume its code successfully.
 *
 * If the coroutine was resumed with an exception (for example, using the [Continuation.resumeWithException] extension
 * function) and cancelled, then the exception thrown by the `suspendCancellableCoroutine` function is determined
 * by what happened first: exceptional resume or cancellation.
 *
 * ### Resuming with a closeable resource
 *
 * [CancellableContinuation] provides the capability to work with values that represent a resource that should be
 * closed. For that, it provides `resume(value: R, onCancellation: ((cause: Throwable, value: R, context: CoroutineContext) -> Unit)`
 * function that guarantees that either the given `value` will be successfully returned from the corresponding
 * `suspend` function or that `onCancellation` will be invoked with the supplied value:
 *
 * ```
 * continuation.resume(resourceToResumeWith) { _, resourceToClose, _
 *     // Will be invoked if the continuation is cancelled while being dispatched
 *     resourceToClose.close()
 * }
 * ```
 *
 * #### Continuation states
 *
 * A cancellable continuation has three observable states:
 *
 * | **State**                           | [isActive] | [isCompleted] | [isCancelled] |
 * | ----------------------------------- | ---------- | ------------- | ------------- |
 * | _Active_ (initial state)            | `true`     | `false`       | `false`       |
 * | _Resumed_ (final _completed_ state) | `false`    | `true`        | `false`       |
 * | _Canceled_ (final _completed_ state)| `false`    | `true`        | `true`        |
 *
 * For a detailed description of each state, see the corresponding properties' documentation.
 *
 * A successful invocation of [cancel] transitions the continuation from an _active_ to a _cancelled_ state, while
 * an invocation of [Continuation.resume] or [Continuation.resumeWithException] transitions it from
 * an _active_ to _resumed_ state.
 *
 * Possible state transitions diagram:
 * ```
 *    +-----------+   resume    +---------+
 *    |  Active   | ----------> | Resumed |
 *    +-----------+             +---------+
 *          |
 *          | cancel
 *          V
 *    +-----------+
 *    | Cancelled |
 *    +-----------+
 * ```
 */
// @OptIn(ExperimentalSubclassOptIn::class)
// @SubclassOptInRequired(InternalForInheritanceCoroutinesApi::class)
template<typename T>
class CancellableContinuation : public Continuation<T> {
public:
    virtual ~CancellableContinuation() = default;

    /**
     * Returns `true` when this continuation is active -- it was created,
     * but not yet [resumed][Continuation.resumeWith] or [cancelled][CancellableContinuation.cancel].
     *
     * This state implies that [isCompleted] and [isCancelled] are `false`,
     * but this can change immediately after the invocation because of parallel calls to [cancel] and [resume].
     */
    virtual bool is_active() const = 0;

    /**
     * Returns `true` when this continuation was completed -- [resumed][Continuation.resumeWith] or
     * [cancelled][CancellableContinuation.cancel].
     *
     * This state implies that [isActive] is `false`.
     */
    virtual bool is_completed() const = 0;

    /**
     * Returns `true` if this continuation was [cancelled][CancellableContinuation.cancel].
     *
     * It implies that [isActive] is `false` and [isCompleted] is `true`.
     */
    virtual bool is_cancelled() const = 0;

    /**
     * Tries to resume this continuation with the specified [value] and returns a non-nullptr class token if successful,
     * or `nullptr` otherwise (it was already resumed or cancelled). When a non-nullptr class is returned,
     * [completeResume] must be invoked with it.
     *
     * When [idempotent] is not `nullptr`, this function performs an _idempotent_ operation, so that
     * further invocations with the same non-nullptr reference produce the same result.
     *
     * @suppress **This is unstable API and it is subject to change.**
     */
    // @InternalCoroutinesApi
    virtual void* try_resume(T value, void* idempotent = nullptr) = 0;

    /**
     * Same as [tryResume] but with an [onCancellation] handler that is called if and only if the value is not
     * delivered to the caller because of the dispatch in the process.
     *
     * The purpose of this function is to enable atomic delivery guarantees: either resumption succeeded, passing
     * the responsibility for [value] to the continuation, or the [onCancellation] block will be invoked,
     * allowing one to free the resources in [value].
     *
     * Implementation note: current implementation always returns RESUME_TOKEN or `nullptr`
     *
     * @suppress  **This is unstable API and it is subject to change.**
     */
    // @InternalCoroutinesApi
    template<typename R>
    void* try_resume(
        R value,
        void* idempotent,
        std::function<void(Throwable*, R, CoroutineContext)> on_cancellation
    ) {
        // Default impl or pure virtual? The snippet had abstract in one and impl in another
        return nullptr; // Stub for now as interface
    }

    /**
     * Tries to resume this continuation with the specified [exception] and returns a non-nullptr class token if successful,
     * or `nullptr` otherwise (it was already resumed or cancelled). When a non-nullptr class is returned,
     * [completeResume] must be invoked with it.
     *
     * @suppress **This is unstable API and it is subject to change.**
     */
    // @InternalCoroutinesApi
    virtual void* try_resume_with_exception(std::exception_ptr exception) = 0;

    /**
     * Completes the execution of [tryResume] or [tryResumeWithException] on its non-nullptr result.
     *
     * @suppress **This is unstable API and it is subject to change.**
     */
    // @InternalCoroutinesApi
    virtual void complete_resume(void* token) = 0;

    /**
     * Internal function that setups cancellation behavior in [suspendCancellableCoroutine].
     * It's illegal to call this function in any non-`kotlinx.coroutines` code and
     * such calls lead to undefined behaviour.
     * Exposed in our ABI since 1.0.0 within `suspendCancellableCoroutine` body.
     *
     * @suppress **This is unstable API and it is subject to change.**
     */
    // @InternalCoroutinesApi
    virtual void init_cancellability() = 0;

    /**
     * Cancels this continuation with an optional cancellation `cause`. The result is `true` if this continuation was
     * cancelled as a result of this invocation, and `false` otherwise.
     * [cancel] might return `false` when the continuation was either [resumed][resume] or already [cancelled][cancel].
     */
    virtual bool cancel(std::exception_ptr cause = nullptr) = 0;

    /**
     * Registers a [handler] to be **synchronously** invoked on [cancellation][cancel] (regular or exceptional) of this continuation.
     * When the continuation is already cancelled, the handler is immediately invoked with the cancellation exception.
     * Otherwise, the handler will be invoked as soon as this continuation is cancelled.
     *
     * The installed [handler] should not throw any exceptions.
     * If it does, they will get caught, wrapped into a `CompletionHandlerException` and
     * processed as an uncaught exception in the context of the current coroutine
     * (see [CoroutineExceptionHandler]).
     *
     * At most one [handler] can be installed on a continuation.
     * Attempting to call `invokeOnCancellation` a second time produces an [IllegalStateException].
     *
     * This handler is also called when this continuation [resumes][Continuation.resume] normally (with a value) and then
     * is cancelled while waiting to be dispatched. More generally speaking, this handler is called whenever
     * the caller of [suspendCancellableCoroutine] is getting a [CancellationException].
     *
     * A typical example of `invokeOnCancellation` usage is given in
     * the documentation for the [suspendCancellableCoroutine] function.
     *
     * **Note**: Implementations of [CompletionHandler] must be fast, non-blocking, and thread-safe.
     * This [handler] can be invoked concurrently with the surrounding code.
     * There is no guarantee on the execution context in which the [handler] will be invoked.
     */
    virtual void invoke_on_cancellation(std::function<void(std::exception_ptr)> handler) = 0;

    /**
     * Resumes this continuation with the specified [value] in the invoker thread without going through
     * the [dispatch][CoroutineDispatcher.dispatch] function of the [CoroutineDispatcher] in the [context].
     * This function is designed to only be used by [CoroutineDispatcher] implementations.
     * **It should not be used in general code**.
     *
     * **Note: This function is experimental.** Its signature general code may be changed in the future.
     */
    // @ExperimentalCoroutinesApi
    virtual void resume_undispatched(CoroutineDispatcher* dispatcher, T value) = 0;

    /**
     * Resumes this continuation with the specified [exception] in the invoker thread without going through
     * the [dispatch][CoroutineDispatcher.dispatch] function of the [CoroutineDispatcher] in the [context].
     * This function is designed to only be used by [CoroutineDispatcher] implementations.
     * **It should not be used in general code**.
     *
     * **Note: This function is experimental.** Its signature general code may be changed in the future.
     */
    // @ExperimentalCoroutinesApi
    virtual void resume_undispatched_with_exception(CoroutineDispatcher* dispatcher, std::exception_ptr exception) = 0;

    /** @suppress */
    // @Deprecated("Use the overload that also accepts the `value` and the coroutine context in lambda")
    virtual void resume(T value, std::function<void(std::exception_ptr)> on_cancellation) = 0;
    
    // Internal helper for resume_undispatched_impl logic if needed
    // virtual void resume_undispatched_impl(CoroutineDispatcher* dispatcher, T value) = 0;
};

// Forward declaration of CancellableContinuationImpl for the reusable function
template<typename T>
class CancellableContinuationImpl;

/**
 * Suspends the coroutine like [suspendCoroutine], but providing a [CancellableContinuation] to
 * the [block].
 */
template<typename T>
T suspend_cancellable_coroutine(std::function<void(CancellableContinuation<T>&)> block) {
    // Stub
    return T();
}

/**
 * Suspends the coroutine similar to [suspendCancellableCoroutine], but an instance of
 * [CancellableContinuationImpl] is reused.
 */
template<typename T>
T suspend_cancellable_coroutine_reusable(std::function<void(CancellableContinuationImpl<T>&)> block) {
    // Stub
    return T();
}

template<typename T>
CancellableContinuationImpl<T>* get_or_create_cancellable_continuation(Continuation<T>* delegate) {
    return nullptr;
}

/**
 * Disposes the specified [handle] when this continuation is cancelled.
 */
void dispose_on_cancellation(CancellableContinuation<void>& cont, DisposableHandle* handle);

} // namespace coroutines
} // namespace kotlinx
