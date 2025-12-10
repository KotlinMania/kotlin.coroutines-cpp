#pragma once
/**
 * @file Delay.hpp
 * @brief Delay interface and functions for coroutine timing
 *
 * Transliterated from: kotlinx-coroutines-core/common/src/Delay.kt
 *
 * This file provides the Delay interface implemented by dispatchers that support
 * scheduled execution, as well as the delay() and await_cancellation() functions.
 */

#include "kotlinx/coroutines/CancellableContinuation.hpp"
#include "kotlinx/coroutines/Runnable.hpp"
#include "kotlinx/coroutines/DisposableHandle.hpp"
#include <memory>
#include <chrono>
#include <string>

namespace kotlinx {
namespace coroutines {

/**
 * This dispatcher feature is implemented by CoroutineDispatcher implementations
 * that natively support scheduled execution of tasks.
 *
 * Implementation of this interface affects operation of delay() and withTimeout() functions.
 *
 * @internal This is an internal API and should not be used from general code.
 */
class Delay {
public:
    virtual ~Delay() = default;

    /**
     * Schedules resume of a specified continuation after a specified delay.
     *
     * Continuation **must be scheduled** to resume even if it is already cancelled,
     * because a cancellation is just an exception that the coroutine that used delay
     * might want to catch and process. It might need to close some resources in its
     * finally blocks, for example.
     *
     * This implementation is supposed to use dispatcher's native ability for scheduled
     * execution in its thread(s).
     *
     * @param time_millis the delay time in milliseconds
     * @param continuation the continuation to resume after the delay
     */
    virtual void schedule_resume_after_delay(long long time_millis, CancellableContinuation<void>& continuation) = 0;

    /**
     * Schedules invocation of a specified block after a specified delay.
     * The resulting DisposableHandle can be used to dispose of this invocation
     * request if it is not needed anymore.
     *
     * @param time_millis the delay time in milliseconds
     * @param block the runnable to execute after the delay
     * @param context the coroutine context
     * @return a DisposableHandle to cancel the scheduled invocation
     */
    virtual std::shared_ptr<DisposableHandle> invoke_on_timeout(
        long long time_millis,
        std::shared_ptr<Runnable> block,
        const CoroutineContext& context) = 0;

    // TODO: MISSING API - kotlinx.coroutines.Delay
    // public abstract suspend fun delay(timeMillis: Long): Unit
    // Delays coroutine for a given time without blocking a thread and resumes it after a specified time.
    // This is a suspending function. If the Job of the current coroutine is cancelled while this
    // suspending function is waiting, this function immediately resumes with CancellationException.
    // Translation: This should be a co-routine suspending function that integrates with our
    // suspend mechanism. Current delay() implementations are blocking.
    // Signature (when suspend macros ready): Awaitable<void> delay(long long time_millis);
};

/**
 * Enhanced Delay interface that provides additional diagnostics for withTimeout.
 *
 * @internal This is an internal API and should not be used from general code.
 */
class DelayWithTimeoutDiagnostics : public Delay {
public:
    /**
     * Returns a string that explains that the timeout has occurred,
     * and explains what can be done about it.
     *
     * @param timeout the timeout duration in nanoseconds
     * @return diagnostic message
     */
    virtual std::string timeout_message(std::chrono::nanoseconds timeout) = 0;
};

// -------------------- Delay functions --------------------

/**
 * Delays coroutine for at least the given time without blocking a thread and
 * resumes it after a specified time. If the given time is non-positive, this
 * function returns immediately.
 *
 * NOTE: In this C++ transliteration, delay is implemented as a blocking sleep.
 * In true Kotlin coroutines, this is a suspending function that doesn't block threads.
 *
 * @param time_millis time in milliseconds
 */
void delay(long time_millis);

/**
 * Delays coroutine for at least the given duration.
 *
 * @param duration the duration to delay
 */
void delay(std::chrono::nanoseconds duration);

/**
 * Delays coroutine for at least the given duration (milliseconds overload).
 *
 * @param duration the duration to delay
 */
void delay(std::chrono::milliseconds duration);

/**
 * Suspends until cancellation, in which case it will throw a CancellationException.
 *
 * This function never returns normally - it either suspends forever or throws
 * when cancelled.
 *
 * NOTE: In this C++ transliteration, this blocks indefinitely since there's
 * no cancellation mechanism outside of true coroutines.
 */
[[noreturn]] void await_cancellation();

} // namespace coroutines
} // namespace kotlinx
