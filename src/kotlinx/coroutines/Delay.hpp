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

#include "kotlinx/coroutines/CancellableContinuationImpl.hpp"
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
        const CoroutineContext& context);

    /**
     * Suspension point: Delays coroutine for a given time without blocking a thread.
     * Use suspend_cancellable_coroutine logic inside.
     */
    virtual void* delay(long long time_millis, Continuation<void*>* continuation);
};

/**
 * Internal DefaultDelay instance (platform-specific).
 *
 * Kotlin source: internal expect val DefaultDelay: Delay
 */
Delay& get_default_delay();

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
 * Delays coroutine for at least the given duration without blocking.
 */
void* delay(long long time_millis, Continuation<void*>* continuation);

/**
 * Delays coroutine for at least the given duration.
 */
void* delay(std::chrono::nanoseconds duration, Continuation<void*>* continuation);

/**
 * Delays coroutine for at least the given duration (milliseconds overload).
 */
void* delay(std::chrono::milliseconds duration, Continuation<void*>* continuation);

/**
 * Suspends the coroutine until cancellation.
 * @param continuation the continuation to suspend
 * @return COROUTINE_SUSPENDED
 */
void* await_cancellation(Continuation<void*>* continuation);

// -----------------------------------------------------------------------------
// shared_ptr Overloads (convenience wrappers)
// -----------------------------------------------------------------------------

void* delay(long long time_millis, std::shared_ptr<Continuation<void*>> continuation);

void* delay(std::chrono::nanoseconds duration, std::shared_ptr<Continuation<void*>> continuation);

void* delay(std::chrono::milliseconds duration, std::shared_ptr<Continuation<void*>> continuation);

void* await_cancellation(std::shared_ptr<Continuation<void*>> continuation);

} // namespace coroutines
} // namespace kotlinx
