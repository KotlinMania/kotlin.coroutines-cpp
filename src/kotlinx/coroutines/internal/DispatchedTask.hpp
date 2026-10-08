#pragma once
// port-lint: source kotlinx-coroutines-core/common/src/internal/DispatchedTask.kt
/**
 * @file DispatchedTask.hpp
 *
 * Transliterated from: kotlinx-coroutines-core/common/src/internal/DispatchedTask.kt
 */

#include <exception>
#include <memory>
#include <string>
#include "kotlinx/coroutines/Runnable.hpp"
#include "kotlinx/coroutines/Continuation.hpp"
#include "kotlinx/coroutines/CoroutineDispatcher.hpp"
#include "kotlinx/coroutines/Job.hpp"
#include "kotlinx/coroutines/Result.hpp"

namespace kotlinx {
namespace coroutines {

/**
 * Non-cancellable dispatch mode.
 *
 * **DO NOT CHANGE THE CONSTANT VALUE**. It might be inlined into legacy user code that was calling
 * inline `suspendAtomicCancellableCoroutine` function and did not support reuse.
 */
// Transliterated from: kotlinx-coroutines-core/common/src/internal/DispatchedTask.kt:13-13
static constexpr int MODE_ATOMIC = 0;

/**
 * Cancellable dispatch mode. It is used by user-facing [suspendCancellableCoroutine].
 * Note, that implementation of cancellability checks mode via [Int.isCancellableMode] extension.
 *
 * **DO NOT CHANGE THE CONSTANT VALUE**. It is being into the user code from [suspendCancellableCoroutine].
 */
// Transliterated from: kotlinx-coroutines-core/common/src/internal/DispatchedTask.kt:22-22
static constexpr int MODE_CANCELLABLE = 1;

/**
 * Cancellable dispatch mode for [suspendCancellableCoroutineReusable].
 * Note, that implementation of cancellability checks mode via [Int.isCancellableMode] extension;
 * implementation of reuse checks mode via [Int.isReusableMode] extension.
 */
// Transliterated from: kotlinx-coroutines-core/common/src/internal/DispatchedTask.kt:29-29
static constexpr int MODE_CANCELLABLE_REUSABLE = 2;

/**
 * Undispatched mode for [CancellableContinuation.resumeUndispatched].
 * It is used when the thread is right, but it needs to be marked with the current coroutine.
 */
// Transliterated from: kotlinx-coroutines-core/common/src/internal/DispatchedTask.kt:35-35
static constexpr int MODE_UNDISPATCHED = 4;

/**
 * Initial mode for [DispatchedContinuation] implementation, should never be used for dispatch, because it is always
 * overwritten when continuation is resumed with the actual resume mode.
 */
// Transliterated from: kotlinx-coroutines-core/common/src/internal/DispatchedTask.kt:41-41
static constexpr int MODE_UNINITIALIZED = -1;

inline bool is_cancellable_mode(int mode) {
    return mode == MODE_CANCELLABLE || mode == MODE_CANCELLABLE_REUSABLE;
}

inline bool is_reusable_mode(int mode) {
    return mode == MODE_CANCELLABLE_REUSABLE;
}

/**
 * A Runnable optimized for dispatcher queues.
 *
 * Kotlin source: internal expect abstract class SchedulerTask : Runnable
 */
class SchedulerTask : public Runnable {
public:
    virtual ~SchedulerTask() = default;
};

/**
 * Internal dispatched task wrapper.
 *
 * This is a direct transliteration of Kotlin's DispatchedTask<T>. The Kotlin
 * `takeState(): Any?` is represented here as a Result<T> bridge.
 */
template<typename T>
class DispatchedTask : public SchedulerTask {
public:
    int resume_mode;

    explicit DispatchedTask(int resume_mode) : resume_mode(resume_mode) {}
    virtual ~DispatchedTask() = default;

    // NOTE(port): Dispatcher queues must retain the task that Kotlin GC owns.
    virtual std::shared_ptr<SchedulerTask> shared_task() = 0;

    // Kotlin: internal abstract val delegate: Continuation<T>
    virtual std::shared_ptr<Continuation<T>> get_delegate() = 0;

    // Kotlin: internal abstract fun takeState(): Any?
    virtual Result<T> take_state() = 0;

    /**
     * Called when this task was cancelled while it was being dispatched.
     */
    // Transliterated from: kotlinx-coroutines-core/common/src/internal/DispatchedTask.kt:56-56
    // NOTE(port): The source default is empty; preserve unused names as comments.
    virtual void cancel_completed_result(Result<T> /* taken_state */, std::exception_ptr /* cause */) {}

    /**
     * There are two implementations of `DispatchedTask`:
     * - [DispatchedContinuation] keeps only simple values as successfully results.
     * - [CancellableContinuationImpl] keeps additional data with values and overrides this method to unwrap it.
     */
    // Transliterated from: kotlinx-coroutines-core/common/src/internal/DispatchedTask.kt:64-65
    template<typename R>
    R get_successful_result(const Result<T>& state) {
        return static_cast<R>(state.get_or_throw());
    }

    /**
     * There are two implementations of `DispatchedTask`:
     * - [DispatchedContinuation] is just an intermediate storage that stores the exception that has its stack-trace
     *   properly recovered and is ready to pass to the [delegate] continuation directly.
     * - [CancellableContinuationImpl] stores raw cause of the failure in its state; when it needs to be dispatched
     *   its stack-trace has to be recovered, so it overrides this method for that purpose.
     */
    // Transliterated from: kotlinx-coroutines-core/common/src/internal/DispatchedTask.kt:74-75
    virtual std::exception_ptr get_exceptional_result(const Result<T>& state) {
        return state.exception_or_null();
    }

    // Transliterated from: kotlinx-coroutines-core/common/src/internal/DispatchedTask.kt:77-109
    void run() override final;

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
    // Transliterated from: kotlinx-coroutines-core/common/src/internal/DispatchedTask.kt:129-133
    void handle_fatal_exception(std::exception_ptr exception);
};

// Kotlin: internal fun <T> DispatchedTask<T>.dispatch(mode: Int)
template<typename T>
void dispatch(DispatchedTask<T>* task, int mode);

// Kotlin: internal fun <T> DispatchedTask<T>.resume(delegate: Continuation<T>, undispatched: Boolean)
template<typename T>
void resume(DispatchedTask<T>* task, std::shared_ptr<Continuation<T>> delegate, bool undispatched);

struct EventLoop;
// Transliterated from: kotlinx-coroutines-core/common/src/internal/DispatchedTask.kt:180-200
template<typename T, typename Block>
inline void run_unconfined_event_loop(DispatchedTask<T>* task, EventLoop& event_loop, Block&& block);

namespace internal {

/**
 * This exception holds an exception raised in [CoroutineDispatcher.dispatch] method.
 * When dispatcher methods fail unexpectedly, it is likely a user-induced programmatic bug,
 * such as calling `executor.close()` prematurely. To avoid reporting such exceptions as fatal errors,
 * we handle them with a separate code path. See also #4091.
 *
 * @see safeDispatch
 */
// Transliterated from: kotlinx-coroutines-core/common/src/internal/DispatchedTask.kt:215-219
class DispatchException : public std::exception {
public:
    std::exception_ptr cause;

    // Transliterated from: kotlinx-coroutines-core/common/src/internal/DispatchedTask.kt:215-219
    DispatchException(std::exception_ptr cause, const CoroutineDispatcher* dispatcher, const CoroutineContext* context);

    // NOTE(port): std::exception transport exposes the constructed source message.
    const char* what() const noexcept override;

private:
    std::string message_;
};

} // namespace internal

} // namespace coroutines
} // namespace kotlinx
