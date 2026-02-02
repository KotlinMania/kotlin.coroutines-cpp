#pragma once
// port-lint: source internal/DispatchedTask.kt
/**
 * @file DispatchedTask.hpp
 *
 * Transliterated from: kotlinx-coroutines-core/common/src/internal/DispatchedTask.kt
 */

#include <exception>
#include <memory>
#include <string>
#include <typeinfo>
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
 * **DO NOT CHANGE THE CONSTANT VALUE** — matches Kotlin's MODE_ATOMIC.
 */
static constexpr int MODE_ATOMIC = 0;

/**
 * Cancellable dispatch mode for suspend_cancellable_coroutine.
 *
 * **DO NOT CHANGE THE CONSTANT VALUE** — matches Kotlin's MODE_CANCELLABLE.
 */
static constexpr int MODE_CANCELLABLE = 1;

/** Cancellable + reusable mode. */
static constexpr int MODE_CANCELLABLE_REUSABLE = 2;

/** Undispatched mode. */
static constexpr int MODE_UNDISPATCHED = 4;

/** Initial mode for DispatchedContinuation, should never be dispatched. */
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

    // Kotlin: internal abstract val delegate: Continuation<T>
    virtual std::shared_ptr<Continuation<T>> get_delegate() = 0;

    // Kotlin: internal abstract fun takeState(): Any?
    virtual Result<T> take_state() = 0;

    // Kotlin: internal open fun cancelCompletedResult(...)
    virtual void cancel_completed_result(Result<T> taken_state, std::exception_ptr cause) {}

    // Kotlin: internal open fun <T> getSuccessfulResult(state: Any?): T
    template<typename R>
    R get_successful_result(const Result<T>& state) {
        return static_cast<R>(state.get_or_throw());
    }

    // Kotlin: internal open fun getExceptionalResult(state: Any?): Throwable?
    std::exception_ptr get_exceptional_result(const Result<T>& state) {
        return state.exception_or_null();
    }

    // Kotlin: final override fun run()
    void run() override;

    // Kotlin: internal fun handleFatalException(exception: Throwable)
    void handle_fatal_exception(std::exception_ptr exception);
};

// Kotlin: internal fun <T> DispatchedTask<T>.dispatch(mode: Int)
template<typename T>
void dispatch(DispatchedTask<T>* task, int mode);

// Kotlin: internal fun <T> DispatchedTask<T>.resume(delegate: Continuation<T>, undispatched: Boolean)
template<typename T>
void resume(DispatchedTask<T>* task, std::shared_ptr<Continuation<T>> delegate, bool undispatched);

namespace internal {

/**
 * Kotlin: internal class DispatchException(...)
 */
class DispatchException : public std::exception {
public:
    std::exception_ptr cause;

    DispatchException(std::exception_ptr cause_, const CoroutineDispatcher* dispatcher, const CoroutineContext* context)
        : cause(cause_) {
        message_ = std::string("Coroutine dispatcher ") +
            (dispatcher ? dispatcher->to_string() : "<null>") +
            " threw an exception, context = " +
            (context ? typeid(*context).name() : "<null>");
    }

    const char* what() const noexcept override { return message_.c_str(); }

private:
    std::string message_;
};

} // namespace internal

} // namespace coroutines
} // namespace kotlinx
