#pragma once
#include <string>
#include <functional>
#include <memory>
#include <coroutine>
#include "kotlinx/coroutines/Continuation.hpp"
#include "kotlinx/coroutines/Job.hpp"
#include "kotlinx/coroutines/DisposableHandle.hpp"
#include "kotlinx/coroutines/CoroutineContext.hpp"

namespace kotlinx {
namespace coroutines {

class CoroutineDispatcher; 

// Throwable alias standard
using Throwable = std::exception_ptr;

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
template<typename T>
class CancellableContinuation : public Continuation<T> {
public:
    virtual ~CancellableContinuation() = default;

    // Inherited from Continuation
    virtual std::shared_ptr<CoroutineContext> get_context() const = 0;

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
     */
    virtual void* try_resume(T value, void* idempotent = nullptr) = 0;
    
    /**
     * Tries to resume this continuation with the specified [exception] and returns a non-nullptr class token if successful,
     * or `nullptr` otherwise (it was already resumed or cancelled). When a non-nullptr class is returned,
     * [completeResume] must be invoked with it.
     */
    virtual void* try_resume_with_exception(std::exception_ptr exception) = 0;
    
    /**
     * Completes the execution of [tryResume] or [tryResumeWithException] on its non-nullptr result.
     */
    virtual void complete_resume(void* token) = 0;
    
    /**
     * Internal function that setups cancellation behavior in [suspendCancellableCoroutine].
     */
    virtual void init_cancellability() = 0;
    
    /**
     * Cancels this continuation with an optional cancellation `cause`. The result is `true` if this continuation was
     * cancelled as a result of this invocation, and `false` otherwise.
     */
    virtual bool cancel(std::exception_ptr cause = nullptr) = 0;
    
    /**
     * Registers a [handler] to be **synchronously** invoked on [cancellation][cancel] (regular or exceptional) of this continuation.
     */
    virtual void invoke_on_cancellation(std::function<void(std::exception_ptr)> handler) = 0;
    
    /**
     * Resumes this continuation with the specified [value] in the invoker thread without going through
     * the [dispatch][CoroutineDispatcher.dispatch] function of the [CoroutineDispatcher] in the [context].
     * This function is designed to only be used by [CoroutineDispatcher] implementations.
     */
    virtual void resume_undispatched(CoroutineDispatcher* dispatcher, T value) = 0;
    
    /**
     * Resumes this continuation with the specified [exception] in the invoker thread without going through
     * the [dispatch][CoroutineDispatcher.dispatch] function of the [CoroutineDispatcher] in the [context].
     * This function is designed to only be used by [CoroutineDispatcher] implementations.
     */
    virtual void resume_undispatched_with_exception(CoroutineDispatcher* dispatcher, std::exception_ptr exception) = 0;
    
    /**
     * Resumes this continuation with the specified [value] and cancellation handler.
     */
    virtual void resume(T value, std::function<void(std::exception_ptr)> on_cancellation) = 0;
    
    // Convenience helpers
    void resume(T value) {
        resume(value, nullptr);
    }

    void resume_with_exception(std::exception_ptr exception) {
        resume_with(Result<T>::failure(exception));
    }
    
    // Virtual from Continuation
    virtual void resume_with(Result<T> result) override {
        if (result.is_success()) {
            resume(result.get_or_throw(), nullptr);
        } else {
             // Exception case handled by impl or ignored here
        }
    }
};

/**
 * Specialization for void.
 */
template<>
class CancellableContinuation<void> : public Continuation<void> {
public:
    virtual ~CancellableContinuation() = default;

    virtual std::shared_ptr<CoroutineContext> get_context() const = 0;
    virtual bool is_active() const = 0;
    virtual bool is_completed() const = 0;
    virtual bool is_cancelled() const = 0;

    virtual void* try_resume(void* idempotent = nullptr) = 0;
    virtual void* try_resume_with_exception(std::exception_ptr exception) = 0;
    virtual void complete_resume(void* token) = 0;
    virtual void init_cancellability() = 0;
    virtual bool cancel(std::exception_ptr cause = nullptr) = 0;
    
    // Fixed signature: no T value
    virtual void invoke_on_cancellation(std::function<void(std::exception_ptr)> handler) = 0;
    virtual void resume_undispatched(CoroutineDispatcher* dispatcher) = 0; 
    virtual void resume_undispatched_with_exception(CoroutineDispatcher* dispatcher, std::exception_ptr exception) = 0;
    virtual void resume(std::function<void(std::exception_ptr)> on_cancellation) = 0;

    // Convenience
    void resume() {
        resume(nullptr);
    }
    
    void resume_with_exception(std::exception_ptr exception) {
         // How to route this to Impl?
         // Impl will implement resume_with(Result<void>).
         // So we can call that?
         this->resume_with(Result<void>::failure(exception));
    }
    
    template<typename R>
    void* try_resume(
        R value, // Should be ignored or void?
        void* idempotent,
        std::function<void(Throwable, R, std::shared_ptr<CoroutineContext>)> on_cancellation
    ) {
         return nullptr; 
    }
};

/**
 * Suspends the coroutine like [suspendCoroutine], but providing a [CancellableContinuation] to
 * the [block].
 */
// Forward declaration
template <typename T>
class CancellableContinuationImpl;

template <typename T>
class SuspendingCancellableCoroutine {
public:
    std::function<void(CancellableContinuation<T>&)> block;
    std::shared_ptr<CancellableContinuationImpl<T>> impl;

    SuspendingCancellableCoroutine(std::function<void(CancellableContinuation<T>&)> b) : block(b) {}

    bool await_ready() { return false; }

    void await_suspend(std::coroutine_handle<> h);

    T await_resume();
};

template<typename T>
auto suspend_cancellable_coroutine(std::function<void(CancellableContinuation<T>&)> block) {
    return SuspendingCancellableCoroutine<T>(block);
}

void dispose_on_cancellation(CancellableContinuation<void>& cont, DisposableHandle* handle);

} // namespace coroutines
} // namespace kotlinx
