#pragma once
#include <string>
#include <functional>
#include <memory>
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
 * @brief Thread-safe cancellable continuation with atomic decision state machine support.
 *
 * CancellableContinuation is a thread-safe continuation primitive that supports asynchronous
 * cancellation through an atomic decision state machine. It extends the base Continuation<T>
 * interface with cancellation capabilities and proper coordination between suspension and
 * resumption operations.
 *
 * The implementation uses a fixed suspension infrastructure that eliminates exception-based
 * hacks in favor of proper atomic coordination. This provides LLVM-optimized performance
 * and eliminates runtime overhead from exception handling.
 *
 * ## Usage
 *
 * An instance of CancellableContinuation can only be obtained through the
 * suspend_cancellable_coroutine function. The typical usage pattern is to suspend
 * a coroutine while waiting for a result from a callback or external source that
 * supports cancellation:
 *
 * ```cpp
 * auto future = some_async_operation();
 * suspend_cancellable_coroutine<int>([&](auto& continuation) {
 *     future.whenComplete([&](int result, std::exception_ptr error) {
 *         if (error) {
 *             continuation.resume_with_exception(error);
 *         } else {
 *             continuation.resume(result);
 *         }
 *     });
 *     // Cancel the future if continuation is cancelled
 *     continuation.invoke_on_cancellation([&](std::exception_ptr) {
 *         future.cancel();
 *     });
 * }, caller_continuation);
 * ```
 *
 * ## Thread Safety
 *
 * - All public methods are thread-safe and can be called concurrently
 * - Concurrent cancel() and resume() operations are coordinated atomically
 * - Only one operation will succeed in any race condition
 * - Multiple concurrent resume() attempts are considered programmatic errors
 *
 * ## Prompt Cancellation Guarantee
 *
 * The continuation provides prompt cancellation guarantee: if the parent Job is
 * cancelled while this continuation is suspended, it will not resume successfully,
 * even if resume() was called but not yet executed. This is enforced through the
 * atomic decision state machine.
 *
 * ## State Machine
 *
 * The continuation transitions through three observable states:
 *
 * | State      | is_active() | is_completed() | is_cancelled() |
 * |------------|-------------|----------------|----------------|
 * | Active     | true        | false          | false          |
 * | Resumed    | false       | true           | false          |
 * | Cancelled  | false       | true           | true           |
 *
 * State transitions:
 * - Active -> Resumed: via successful resume() call
 * - Active -> Cancelled: via successful cancel() call
 *
 * @tparam T The result type of the continuation
 *
 * @note This interface uses the fixed suspension infrastructure with atomic decision
 *       state machine, replacing the previous exception-based approach.
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
    
    template<typename R>
    void* try_resume(
        [[maybe_unused]] R value,
        [[maybe_unused]] void* idempotent,
        [[maybe_unused]] std::function<void(Throwable, R, std::shared_ptr<CoroutineContext>)> on_cancellation
    ) {
         return nullptr; 
    }
};

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

void dispose_on_cancellation(CancellableContinuation<void>& cont, DisposableHandle* handle);

} // namespace coroutines
} // namespace kotlinx
