// Transliterated from Kotlin to C++
// Original: kotlinx-coroutines-core/common/src/internal/DispatchedTask.kt
//
// TODO: This is a mechanical transliteration - semantics not fully implemented
// TODO: Continuation<T>, CoroutineContext, Throwable need C++ equivalents
// TODO: Result type needs C++ implementation
// TODO: @JvmField, @PublishedApi annotations - JVM-specific, translate to comments
// TODO: Extension functions need free function implementations
// TODO: MODE constants and helper functions need proper implementation

#include <exception>

namespace kotlinx {
namespace coroutines {

// Forward declarations
template<typename T> class Continuation;
class CoroutineContext;
class SchedulerTask;
template<typename T> class DispatchedContinuation;
class CompletedExceptionally;
class Job;
class CoroutineDispatcher;
class EventLoop;
class StackTraceElement;

/**
 * Non-cancellable dispatch mode.
 *
 * **DO NOT CHANGE THE CONSTANT VALUE**. It might be inlined into legacy user code that was calling
 * inline `suspendAtomicCancellableCoroutine` function and did not support reuse.
 */
constexpr int MODE_ATOMIC = 0;

/**
 * Cancellable dispatch mode. It is used by user-facing [suspendCancellableCoroutine].
 * Note, that implementation of cancellability checks mode via [Int.isCancellableMode] extension.
 *
 * **DO NOT CHANGE THE CONSTANT VALUE**. It is being into the user code from [suspendCancellableCoroutine].
 */
constexpr int MODE_CANCELLABLE = 1;

/**
 * Cancellable dispatch mode for [suspendCancellableCoroutineReusable].
 * Note, that implementation of cancellability checks mode via [Int.isCancellableMode] extension;
 * implementation of reuse checks mode via [Int.isReusableMode] extension.
 */
constexpr int MODE_CANCELLABLE_REUSABLE = 2;

/**
 * Undispatched mode for [CancellableContinuation.resumeUndispatched].
 * It is used when the thread is right, but it needs to be marked with the current coroutine.
 */
constexpr int MODE_UNDISPATCHED = 4;

/**
 * Initial mode for [DispatchedContinuation] implementation, should never be used for dispatch, because it is always
 * overwritten when continuation is resumed with the actual resume mode.
 */
constexpr int MODE_UNINITIALIZED = -1;

inline bool is_cancellable_mode(int mode) {
    return mode == MODE_CANCELLABLE || mode == MODE_CANCELLABLE_REUSABLE;
}

inline bool is_reusable_mode(int mode) {
    return mode == MODE_CANCELLABLE_REUSABLE;
}

template<typename T>
class DispatchedTask : public SchedulerTask {
public:
    int resume_mode;

    explicit DispatchedTask(int resume_mode) : resume_mode(resume_mode) {}

    virtual Continuation<T>* get_delegate() = 0;
    virtual void* take_state() = 0;

    /**
     * Called when this task was cancelled while it was being dispatched.
     */
    virtual void cancel_completed_result(void* taken_state, std::exception* cause) {}

    /**
     * There are two implementations of `DispatchedTask`:
     * - [DispatchedContinuation] keeps only simple values as successfully results.
     * - [CancellableContinuationImpl] keeps additional data with values and overrides this method to unwrap it.
     */
    template<typename U>
    virtual U get_successful_result(void* state) {
        return static_cast<U>(state);
    }

    /**
     * There are two implementations of `DispatchedTask`:
     * - [DispatchedContinuation] is just an intermediate storage that stores the exception that has its stack-trace
     *   properly recovered and is ready to pass to the [delegate] continuation directly.
     * - [CancellableContinuationImpl] stores raw cause of the failure in its state; when it needs to be dispatched
     *   its stack-trace has to be recovered, so it overrides this method for that purpose.
     */
    virtual std::exception* get_exceptional_result(void* state) {
        // TODO: (state as? CompletedExceptionally)?.cause
        return nullptr; // placeholder
    }

    void run() override {
        // TODO: assert { resume_mode != MODE_UNINITIALIZED } // should have been set before dispatching
        try {
            Continuation<T>* delegate = get_delegate();
            auto* dispatched = dynamic_cast<DispatchedContinuation<T>*>(delegate);
            Continuation<T>* continuation = dispatched->continuation;
            // TODO: with_continuation_context(continuation, dispatched->count_or_element) {
            //     val context = continuation.context
            //     val state = take_state() // NOTE: Must take state in any case, even if cancelled
            //     val exception = get_exceptional_result(state)
            //     /*
            //      * Check whether continuation was originally resumed with an exception.
            //      * If so, it dominates cancellation, otherwise the original exception
            //      * will be silently lost.
            //      */
            //     val job = if (exception == null && resume_mode.is_cancellable_mode) context[Job] else null
            //     if (job != null && !job.isActive) {
            //         val cause = job.getCancellationException()
            //         cancel_completed_result(state, cause)
            //         continuation.resumeWithStackTrace(cause)
            //     } else {
            //         if (exception != null) {
            //             continuation.resumeWithException(exception)
            //         } else {
            //             continuation.resume(get_successful_result(state))
            //         }
            //     }
            // }
        } catch (const DispatchException& e) {
            // TODO: handle_coroutine_exception(delegate.context, e.cause)
        } catch (const std::exception& e) {
            handle_fatal_exception(e);
        }
    }

    /**
     * Machinery that handles fatal exceptions in kotlinx.coroutines.
     * There are two kinds of fatal exceptions:
     *
     * 1) Exceptions from kotlinx.coroutines code. Such exceptions indicate that either
     *    the library or the compiler has a bug that breaks internal invariants.
     *    They usually have specific workarounds, but require careful study of the cause and should
     *    be reported to the maintainers and fixed on the library's side anyway.
     *
     * 2) Exceptions from [ThreadContextElement.updateThreadContext] and [ThreadContextElement.restoreThreadContext].
     *    While a user code can trigger such exception by providing an improper implementation of [ThreadContextElement],
     *    we can't ignore it because it may leave coroutine in the inconsistent state.
     *    If you encounter such exception, you can either disable this context element or wrap it into
     *    another context element that catches all exceptions and handles it in the application specific manner.
     *
     * Fatal exception handling can be intercepted with [CoroutineExceptionHandler] element in the context of
     * a failed coroutine, but such exceptions should be reported anyway.
     */
    void handle_fatal_exception(const std::exception& exception) {
        // TODO: val reason = CoroutinesInternalError("Fatal exception in coroutines machinery for $this. " +
        //         "Please read KDoc to 'handleFatalException' method and report this incident to maintainers", exception)
        // TODO: handle_coroutine_exception(this->get_delegate()->context, reason)
    }
};

template<typename T>
void dispatch(DispatchedTask<T>* task, int mode) {
    // TODO: assert { mode != MODE_UNINITIALIZED } // invalid mode value for this method
    Continuation<T>* delegate = task->get_delegate();
    bool undispatched = (mode == MODE_UNDISPATCHED);
    if (!undispatched /* && delegate is DispatchedContinuation && mode.is_cancellable_mode == resume_mode.is_cancellable_mode */) {
        // dispatch directly using this instance's Runnable implementation
        // TODO: val dispatcher = delegate.dispatcher
        // TODO: val context = delegate.context
        // TODO: if (dispatcher.safeIsDispatchNeeded(context)) {
        //     dispatcher.safeDispatch(context, this)
        // } else {
        //     resume_unconfined()
        // }
    } else {
        // delegate is coming from 3rd-party interceptor implementation (and does not support cancellation)
        // or undispatched mode was requested
        // TODO: resume(delegate, undispatched)
    }
}

template<typename T>
void resume(DispatchedTask<T>* task, Continuation<T>* delegate, bool undispatched) {
    // This resume is never cancellable. The result is always delivered to delegate continuation.
    void* state = task->take_state();
    std::exception* exception = task->get_exceptional_result(state);
    // TODO: val result = if (exception != null) Result.failure(exception) else Result.success(get_successful_result<T>(state))
    // TODO: when {
    //     undispatched -> (delegate as DispatchedContinuation).resume_undispatched_with(result)
    //     else -> delegate.resumeWith(result)
    // }
}

// TODO: Additional helper functions need implementation
// - resume_unconfined
// - run_unconfined_event_loop
// - resume_with_stack_trace
// - DispatchException class

class DispatchException : public std::exception {
public:
    std::exception* cause;
    CoroutineDispatcher* dispatcher;
    CoroutineContext* context;

    DispatchException(std::exception* cause, CoroutineDispatcher* dispatcher, CoroutineContext* context)
        : cause(cause), dispatcher(dispatcher), context(context) {}
};

} // namespace coroutines
} // namespace kotlinx
