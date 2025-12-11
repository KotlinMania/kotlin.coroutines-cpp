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

    /*
     * TODO: MISSING API - Delay.delay() suspend function
     *
     * Kotlin source: Delay.delay() in Delay.kt (internal interface method)
     *
     * What's missing:
     * - Kotlin signature: public abstract suspend fun delay(timeMillis: Long): Unit
     * - This is the Delay interface's method that dispatchers implement
     * - Should integrate with scheduleResumeAfterDelay()
     * - Enables dispatcher-specific delay implementations
     *
     * Current status: Not defined in interface
     * Correct status: Define as suspend function that dispatchers override
     *
     * C++ signature needed: void* delay(long long time_millis, Continuation<void*>* continuation)
     */
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

/*
 * TODO: STUB - delay() blocks thread instead of suspending
 *
 * Kotlin source: delay() in Delay.kt
 *
 * What's missing:
 * - Should be suspend function: suspend fun delay(timeMillis: Long)
 * - Should suspend coroutine without blocking thread
 * - See Delay.cpp for full TODO details
 *
 * Current behavior: Blocks calling thread
 * Correct behavior: Suspend coroutine
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

/*
 * TODO: STUB - awaitCancellation() blocks instead of suspending
 *
 * Kotlin source: awaitCancellation() in Delay.kt
 *
 * What's missing:
 * - Should be suspend function: suspend fun awaitCancellation(): Nothing
 * - Should suspend indefinitely until cancelled
 * - See Delay.cpp for full TODO details
 *
 * Current behavior: Blocks thread forever
 * Correct behavior: Suspend until Job cancelled, throw CancellationException
 */
[[noreturn]] void await_cancellation();

} // namespace coroutines
} // namespace kotlinx
