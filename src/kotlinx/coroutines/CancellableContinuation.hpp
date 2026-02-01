#pragma once
// port-lint: source CancellableContinuation.kt
/**
 * @file CancellableContinuation.hpp
 * @brief Cancellable continuation interface with prompt cancellation guarantee
 *
 * Transliterated from: kotlinx-coroutines-core/common/src/CancellableContinuation.kt
 */

#include <string>
#include <functional>
#include <memory>
#include "kotlinx/coroutines/Continuation.hpp"
#include "kotlinx/coroutines/Job.hpp"
#include "kotlinx/coroutines/DisposableHandle.hpp"
#include "kotlinx/coroutines/CoroutineContext.hpp"
#include "kotlinx/coroutines/internal/Symbol.hpp"
#include "kotlinx/coroutines/internal/Concurrent.hpp"

namespace kotlinx {
namespace coroutines {

class CoroutineDispatcher;

// Throwable alias standard
using Throwable = std::exception_ptr;

/**
 * Cancellable [continuation][Continuation] is a thread-safe continuation primitive with the support of
 * an asynchronous cancellation.
 *
 * Cancellable continuation can be [resumed][Continuation::resume_with], but unlike regular [Continuation],
 * it also might be [cancelled][CancellableContinuation::cancel] explicitly or [implicitly][Job::cancel] via a parent [job][Job].
 *
 * If the continuation is cancelled successfully, it resumes with a [CancellationException] or
 * the specified cancel cause.
 *
 * ### Usage
 *
 * An instance of `CancellableContinuation` can only be obtained by the [suspend_cancellable_coroutine] function.
 * The interface itself is public for use and private for implementation.
 *
 * A typical usages of this function is to suspend a coroutine while waiting for a result
 * from a callback or an external source of values that optionally supports cancellation:
 *
 * ```cpp
 * template<typename T>
 * void* await(CompletableFuture<T>& future, Continuation<void*>* cont) {
 *     return suspend_cancellable_coroutine<T>([&future](CancellableContinuation<T>& c) {
 *         future.when_complete([&c](T result, std::exception_ptr error) {
 *             if (error) {
 *                 // Resume continuation with an exception if an external source failed
 *                 c.resume_with_exception(error);
 *             } else {
 *                 // Resume continuation with a value if it was computed
 *                 c.resume(result);
 *             }
 *         });
 *         // Cancel the computation if the continuation itself was cancelled
 *         c.invoke_on_cancellation([&future](std::exception_ptr) {
 *             future.cancel(true);
 *         });
 *     }, cont);
 * }
 * ```
 *
 * ### Thread-safety
 *
 * Instances of [CancellableContinuation] are thread-safe and can be safely shared across multiple threads.
 * [CancellableContinuation] allows concurrent invocations of the [cancel] and [resume] pair, guaranteeing
 * that only one of these operations will succeed.
 * Concurrent invocations of [resume] methods lead to a [std::logic_error] and are considered a programmatic error.
 * Concurrent invocations of [cancel] methods is permitted, and at most one of them succeeds.
 *
 * ### Prompt cancellation guarantee
 *
 * A cancellable continuation provides a **prompt cancellation guarantee**.
 *
 * If the [Job] of the coroutine that obtained a cancellable continuation was cancelled while this continuation was suspended it will not resume
 * successfully, even if [CancellableContinuation::resume] was already invoked but not yet executed.
 *
 * The cancellation of the coroutine's job is generally asynchronous with respect to the suspended coroutine.
 * The suspended coroutine is resumed with a call to its [Continuation::resume_with] member function or to the
 * [resume][Continuation::resume] extension function.
 * However, when the coroutine is resumed, it does not immediately start executing but is passed to its
 * [CoroutineDispatcher] to schedule its execution when the dispatcher's resources become available for execution.
 * The job's cancellation can happen before, after, and concurrently with the call to `resume`. In any
 * case, prompt cancellation guarantees that the coroutine will not resume its code successfully.
 *
 * If the coroutine was resumed with an exception (for example, using the [Continuation::resume_with_exception] extension
 * function) and cancelled, then the exception thrown by the `suspend_cancellable_coroutine` function is determined
 * by what happened first: exceptional resume or cancellation.
 *
 * ### Resuming with a closeable resource
 *
 * [CancellableContinuation] provides the capability to work with values that represent a resource that should be
 * closed. For that, it provides `resume(value, on_cancellation)` function that guarantees that either the given
 * `value` will be successfully returned from the corresponding `suspend` function or that `on_cancellation` will
 * be invoked with the supplied value:
 *
 * ```cpp
 * continuation.resume(resource_to_resume_with, [](std::exception_ptr, auto resource) {
 *     // Will be invoked if the continuation is cancelled while being dispatched
 *     resource.close();
 * });
 * ```
 *
 * #### Continuation states
 *
 * A cancellable continuation has three observable states:
 *
 * | **State**                           | [is_active] | [is_completed] | [is_cancelled] |
 * | ----------------------------------- | ----------- | -------------- | -------------- |
 * | _Active_ (initial state)            | `true`      | `false`        | `false`        |
 * | _Resumed_ (final _completed_ state) | `false`     | `true`         | `false`        |
 * | _Canceled_ (final _completed_ state)| `false`     | `true`         | `true`         |
 *
 * For a detailed description of each state, see the corresponding properties' documentation.
 *
 * A successful invocation of [cancel] transitions the continuation from an _active_ to a _cancelled_ state, while
 * an invocation of [Continuation::resume] or [Continuation::resume_with_exception] transitions it from
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
 *
 * Transliterated from:
 * public interface CancellableContinuation<in T> : Continuation<T>
 */
template<typename T>
class CancellableContinuation : public Continuation<T> {
public:
    virtual ~CancellableContinuation() = default;

    // Inherited from Continuation
    std::shared_ptr<CoroutineContext> get_context() const override = 0;

    /**
     * Returns `true` when this continuation is active -- it was created,
     * but not yet [resumed][Continuation.resume_with] or [cancelled][CancellableContinuation.cancel].
     *
     * This state implies that [isCompleted] and [isCancelled] are `false`,
     * but this can change immediately after the invocation because of parallel calls to [cancel] and [resume].
     */
    virtual bool is_active() const = 0;

    /**
     * Returns `true` when this continuation was completed -- [resumed][Continuation.resume_with] or
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

    std::shared_ptr<CoroutineContext> get_context() const override = 0;
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
    
    // Kotlin: tryResume(value: R, idempotent: Any?, onCancellation: ...)
    // For void specialization, no value parameter needed
    virtual void* try_resume(
        void* idempotent,
        std::function<void(std::exception_ptr, void*, std::shared_ptr<CoroutineContext>)> on_cancellation
    ) = 0;
};

/**
 * Kotlin extension function from Select.kt lines 865-872:
 *   private fun CancellableContinuation<Unit>.tryResume(onCancellation: ...): Boolean
 *
 * Convenience wrapper that calls try_resume(Unit, null, onCancellation), then complete_resume.
 * Returns true if resumption succeeded, false if already cancelled.
 */
inline bool try_resume_with_on_cancellation(
    CancellableContinuation<void>* cont,
    std::function<void(std::exception_ptr, void*, std::shared_ptr<CoroutineContext>)> on_cancellation
) {
    if (!cont) return false;
    void* token = cont->try_resume(nullptr, on_cancellation);
    if (!token) return false;
    cont->complete_resume(token);
    return true;
}

/**
 * Suspends the coroutine like [suspendCoroutine], but providing a [CancellableContinuation] to
 * the [block].
 *
 * In Kotlin, this is: suspend fun <T> suspendCancellableCoroutine(block: (CancellableContinuation<T>) -> Unit): T
 *
 * In C++, this is implemented as a suspend function that takes a Continuation<void*>* parameter
 * following the Kotlin Native compilation pattern.
 */
// Forward declaration
template <typename T>
class CancellableContinuationImpl;

/**
 * suspend_cancellable_coroutine - suspend function following Kotlin's pattern
 *
 * @param block The block to execute with the CancellableContinuation
 * @param continuation The continuation for resumption (from caller's state machine)
 * @return void* - either COROUTINE_SUSPENDED or the result pointer
 *
 * Usage in invoke_suspend():
 *   case 0:
 *       _label = 1;
 *       result = suspend_cancellable_coroutine<int>([](auto& cont) {
 *           // Register callbacks, etc.
 *           cont.resume(42);
 *       }, this);
 *       if (result == COROUTINE_SUSPENDED) return COROUTINE_SUSPENDED;
 *       [[fallthrough]];
 *   case 1:
 *       value = *reinterpret_cast<int*>(result);
 *       // continue...
 */
template<typename T>
void* suspend_cancellable_coroutine(
    std::function<void(CancellableContinuation<T>&)> block,
    Continuation<void*>* continuation
);

/**
 * Kotlin: (CancellableContinuation<*>).disposeOnCancellation(handle)
 *
 * Registers [handle] to be disposed when [cont] is cancelled.
 */
template<typename T>
inline void dispose_on_cancellation(CancellableContinuation<T>& cont, DisposableHandle* handle) {
    cont.invoke_on_cancellation([handle](std::exception_ptr) {
        handle->dispose();
    });
}

void dispose_on_cancellation(CancellableContinuation<void>& cont, DisposableHandle* handle);

} // namespace coroutines
} // namespace kotlinx
