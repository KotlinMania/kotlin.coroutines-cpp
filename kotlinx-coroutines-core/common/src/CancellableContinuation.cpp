#include <string>
// Transliterated from: kotlinx-coroutines-core/common/src/CancellableContinuation.kt
//
// TODO: Semantic implementation required
// - Implement coroutine suspension mechanisms
// - Implement cancellation infrastructure
// - Add proper exception handling
// - Implement thread-safety guarantees
// - Add memory management for continuation state
// - Implement prompt cancellation guarantee
// - Add resource cleanup for closeable resources

#include "kotlinx/coroutines/core_fwd.hpp"
#include <functional>

// TODO: #include equivalent
// #include <kotlinx/coroutines/internal/...>
// #include <kotlin/coroutines/Continuation.hpp>
// #include <kotlin/coroutines/intrinsics/...>

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
class CancellableContinuation : Continuation<T> {
public:
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
    virtual void* try_resume(
        R value,
        void* idempotent,
        std::function<void(Throwable*, R, CoroutineContext)> on_cancellation
    ) = 0;

    /**
     * Tries to resume this continuation with the specified [exception] and returns a non-nullptr class token if successful,
     * or `nullptr` otherwise (it was already resumed or cancelled). When a non-nullptr class is returned,
     * [completeResume] must be invoked with it.
     *
     * @suppress **This is unstable API and it is subject to change.**
     */
    // @InternalCoroutinesApi
    virtual void* try_resume_with_exception(Throwable* exception) = 0;

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
    virtual bool cancel(Throwable* cause = nullptr) = 0;

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
    virtual void invoke_on_cancellation(CompletionHandler handler) = 0;

    /**
     * Resumes this continuation with the specified [value] in the invoker thread without going through
     * the [dispatch][CoroutineDispatcher.dispatch] function of the [CoroutineDispatcher] in the [context].
     * This function is designed to only be used by [CoroutineDispatcher] implementations.
     * **It should not be used in general code**.
     *
     * **Note: This function is experimental.** Its signature general code may be changed in the future.
     */
    // @ExperimentalCoroutinesApi
    virtual void resume_undispatched(CoroutineDispatcher& dispatcher, T value) = 0;

    /**
     * Resumes this continuation with the specified [exception] in the invoker thread without going through
     * the [dispatch][CoroutineDispatcher.dispatch] function of the [CoroutineDispatcher] in the [context].
     * This function is designed to only be used by [CoroutineDispatcher] implementations.
     * **It should not be used in general code**.
     *
     * **Note: This function is experimental.** Its signature general code may be changed in the future.
     */
    // @ExperimentalCoroutinesApi
    virtual void resume_undispatched_with_exception(CoroutineDispatcher& dispatcher, Throwable* exception) = 0;

    /** @suppress */
    // @Deprecated("Use the overload that also accepts the `value` and the coroutine context in lambda")
    virtual void resume(T value, std::function<void(Throwable*)> on_cancellation) = 0;

    /**
     * Resumes this continuation with the specified [value], calling the specified [onCancellation] if and only if
     * the [value] was not successfully used to resume the continuation.
     *
     * The [value] can be rejected in two cases (in both of which [onCancellation] will be called):
     * - Cancellation happened before the handler was resumed;
     * - The continuation was resumed successfully (before cancellation), but the coroutine's job was cancelled before
     *   it had a chance to run in its dispatcher, and so the suspended function threw an exception instead of returning
     *   this value.
     *
     * The installed [onCancellation] handler should not throw any exceptions.
     * If it does, they will get caught, wrapped into a `CompletionHandlerException`, and
     * processed as an uncaught exception in the context of the current coroutine
     * (see [CoroutineExceptionHandler]).
     *
     * With this version of [resume], it's possible to pass resources that can not simply be left for the garbage
     * collector (like file handles, sockets, etc.) and need to be closed explicitly:
     *
     * ```
     * continuation.resume(resourceToResumeWith) { _, resourceToClose, _ ->
     *     resourceToClose.close()
     * }
     * ```
     *
     * [onCancellation] accepts three arguments:
     *
     * - `cause: Throwable` is the exception with which the continuation was cancelled.
     * - `value` is exactly the same as the [value] passed to [resume] itself.
     *   In the example above, `resourceToResumeWith` is exactly the same as `resourceToClose`; in particular,
     *   one could call `resourceToResumeWith.close()` in the lambda for the same effect.
     *   The reason to reference `resourceToClose` anyway is to avoid a memory allocation due to the lambda
     *   capturing the `resourceToResumeWith` reference.
     * - `context` is the [context] of this continuation.
     *   Like with `value`, the reason this is available as a lambda parameter, even though it is always possible to
     *   call [context] from the lambda instead, is to allow lambdas to capture less of their environment.
     *
     * A more complete example and further details are given in
     * the documentation for the [suspendCancellableCoroutine] function.
     *
     * **Note**: The [onCancellation] handler must be fast, non-blocking, and thread-safe.
     * It can be invoked concurrently with the surrounding code.
     * There is no guarantee on the execution context of its invocation.
     */
    template<typename R>
    void resume(
        R value,
        std::function<void(Throwable*, R, CoroutineContext)> on_cancellation
    );
};

/**
 * A version of `invokeOnCancellation` that accepts a class as a handler instead of a lambda, but identical otherwise.
 * This allows providing a custom [tostd::string] instance that will look better during debugging.
 */
template<typename T>
void invoke_on_cancellation(CancellableContinuation<T>& cont, CancelHandler* handler);
// TODO: implement coroutine suspension

/**
 * Suspends the coroutine like [suspendCoroutine], but providing a [CancellableContinuation] to
 * the [block]. This function throws a [CancellationException] if the [Job] of the coroutine is
 * cancelled or completed while it is suspended, or if [CancellableContinuation.cancel] is invoked.
 *
 * A typical use of this function is to suspend a coroutine while waiting for a result
 * from a single-shot callback API and to return the result to the caller.
 * For multi-shot callback APIs see [callbackFlow][kotlinx.coroutines.flow.callbackFlow].
 *
 * ```
 * auto await_callback(): T = suspendCancellableCoroutine { continuation ->
 *     auto callback = class : Callback { // Implementation of some callback interface
 *         virtual auto on_completed(value: T) {
 *             // Resume coroutine with a value provided by the callback
 *             continuation.resume(value)
 *         }
 *         virtual auto on_api_error(cause: Throwable) {
 *             // Resume coroutine with an exception provided by the callback
 *             continuation.resumeWithException(cause)
 *         }
 *     }
 *     // Register callback with an API
 *     api.register(callback)
 *     // Remove callback on cancellation
 *     continuation.invokeOnCancellation { api.unregister(callback) }
 *     // At this point the coroutine is suspended by suspendCancellableCoroutine until callback fires
 * }
 * ```
 *
 * > The callback `register`/`unregister` methods provided by an external API must be thread-safe, because
 * > `invokeOnCancellation` block can be called at any time due to asynchronous nature of cancellation, even
 * > concurrently with the call of the callback.
 *
 * ### Prompt cancellation guarantee
 *
 * This function provides **prompt cancellation guarantee**.
 * If the [Job] of the current coroutine was cancelled while this function was suspended it will not resume
 * successfully, even if [CancellableContinuation.resume] was already invoked.
 *
 * The cancellation of the coroutine's job is generally asynchronous with respect to the suspended coroutine.
 * The suspended coroutine is resumed with a call to its [Continuation.resumeWith] member function or to the
 * [resume][Continuation.resume] extension function.
 * However, when coroutine is resumed, it does not immediately start executing, but is passed to its
 * [CoroutineDispatcher] to schedule its execution when dispatcher's resources become available for execution.
 * The job's cancellation can happen before, after, and concurrently with the call to `resume`. In any
 * case, prompt cancellation guarantees that the coroutine will not resume its code successfully.
 *
 * If the coroutine was resumed with an exception (for example, using [Continuation.resumeWithException] extension
 * function) and cancelled, then the exception thrown by the `suspendCancellableCoroutine` function is determined
 * by what happened first: exceptional resume or cancellation.
 *
 * ### Returning resources from a suspended coroutine
 *
 * As a result of the prompt cancellation guarantee, when a closeable resource
 * (like open file or a handle to another native resource) is returned from a suspended coroutine as a value,
 * it can be lost when the coroutine is cancelled. To ensure that the resource can be properly closed
 * in this case, the [CancellableContinuation] struct provides two functions.
 *
 * - [invokeOnCancellation][CancellableContinuation.invokeOnCancellation] installs a handler that is called
 *   whenever a suspend coroutine is being cancelled. In addition to the example at the beginning, it can be
 *   used to ensure that a resource that was opened before the call to
 *   `suspendCancellableCoroutine` or in its body is closed in case of cancellation.
 *
 * ```
 * suspendCancellableCoroutine { continuation ->
 *     auto resource = openResource() // Opens some resource
 *     continuation.invokeOnCancellation {
 *         resource.close() // Ensures the resource is closed on cancellation
 *     }
 *     // ...
 * }
 * ```
 *
 * - [resume(value) { ... }][CancellableContinuation.resume] method on a [CancellableContinuation] takes
 *   an optional `onCancellation` block. It can be used when resuming with a resource that must be closed by
 *   the code that called the corresponding suspending function.
 *
 * ```
 * suspendCancellableCoroutine { continuation ->
 *     auto callback = class : Callback { // Implementation of some callback interface
 *         // A callback provides a reference to some closeable resource
 *         virtual auto on_completed(resource: T) {
 *             // Resume coroutine with a value provided by the callback and ensure the resource is closed in case
 *             // when the coroutine is cancelled before the caller gets a reference to the resource.
 *             continuation.resume(resource) { cause, resourceToClose, context ->
 *                 resourceToClose.close() // Close the resource on cancellation
 *                 // If we used `resource` instead of `resourceToClose`, this lambda would need to allocate a closure,
 *                 // but with `resourceToClose`, the lambda does not capture any of its environment.
 *             }
 *         }
 *     // ...
 * }
 * ```
 *
 * ### Implementation details and custom continuation interceptors
 *
 * The prompt cancellation guarantee is the result of a coordinated implementation inside `suspendCancellableCoroutine`
 * function and the [CoroutineDispatcher] class. The coroutine dispatcher checks for the status of the [Job] immediately
 * before continuing its normal execution and aborts this normal execution, calling all the corresponding
 * cancellation handlers, if the job was cancelled.
 *
 * If a custom implementation of [ContinuationInterceptor] is used in a coroutine's context that does not extend
 * [CoroutineDispatcher] class, then there is no prompt cancellation guarantee. A custom continuation interceptor
 * can resume execution of a previously suspended coroutine even if its job was already cancelled.
 */
template<typename T>
T suspend_cancellable_coroutine(std::function<void(CancellableContinuation<T>&)> block);
// TODO: implement coroutine suspension

/**
 * Suspends the coroutine similar to [suspendCancellableCoroutine], but an instance of
 * [CancellableContinuationImpl] is reused.
 */
template<typename T>
T suspend_cancellable_coroutine_reusable(std::function<void(CancellableContinuationImpl<T>&)> block);
// TODO: implement coroutine suspension

template<typename T>
CancellableContinuationImpl<T>* get_or_create_cancellable_continuation(Continuation<T>* delegate);
// TODO: implement coroutine suspension

/**
 * Disposes the specified [handle] when this continuation is cancelled.
 *
 * This is a shortcut for the following code with slightly more efficient implementation (one fewer class created):
 * ```
 * invokeOnCancellation { handle.dispose() }
 * ```
 *
 * @suppress **This an API and should not be used from general code.**
 */
// @InternalCoroutinesApi
void dispose_on_cancellation(CancellableContinuation<void>& cont, DisposableHandle* handle);

class DisposeOnCancel : CancelHandler {
private:
    DisposableHandle* handle;

public:
    explicit DisposeOnCancel(DisposableHandle* handle) : handle(handle) {}

    void invoke(Throwable* cause) override {
        handle->dispose();
    }

    std::string to_string() const override {
        return "DisposeOnCancel[" + handle->to_string() + "]";
    }
};

} // namespace coroutines
} // namespace kotlinx
