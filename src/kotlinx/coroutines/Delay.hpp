#pragma once
// port-lint: source kotlinx-coroutines-core/common/src/Delay.kt
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
#include "kotlinx/coroutines/intrinsics/Intrinsics.hpp"
#include "kotlinx/coroutines/internal/CurrentRunningCoroutine.hpp"
#include <chrono>
#include <string>
#include <thread>
#include <limits>

namespace kotlinx {
namespace coroutines {

/**
 * This dispatcher feature is implemented by CoroutineDispatcher implementations
 * that natively support scheduled execution of tasks.
 *
 * Implementation of this interface affects operation of delay() and withTimeout() functions.
 *
 * @internal This is an internal API and should not be used from general code.
 *
 * Transliterated from: kotlinx-coroutines-core/common/src/Delay.kt:18-54
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
     *
     * Transliterated from: kotlinx-coroutines-core/common/src/Delay.kt:45
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
     *
     * Transliterated from: kotlinx-coroutines-core/common/src/Delay.kt:52-54
     */
    virtual std::shared_ptr<DisposableHandle> invoke_on_timeout(
        long long time_millis,
        std::shared_ptr<Runnable> block,
        const CoroutineContext& context);

    /**
     * Suspension point: Delays coroutine for a given time without blocking a thread.
     *
     * Transliterated from: kotlinx-coroutines-core/common/src/Delay.kt:25-28
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
 * Returns Delay implementation of the given context.
 *
 * Transliterated from: kotlinx-coroutines-core/common/src/Delay.kt:149
 */
Delay& get_delay(const CoroutineContext& context);
Delay& get_delay(const std::shared_ptr<CoroutineContext>& context);

/**
 * Convert this duration to its millisecond value. Durations which have a nanosecond component less than
 * a single millisecond will be rounded up to the next largest millisecond.
 *
 * Transliterated from: kotlinx-coroutines-core/common/src/Delay.kt:155-158
 */
inline long long to_delay_millis(std::chrono::nanoseconds duration) {
    if (duration.count() > 0) {
        auto rounded = duration + std::chrono::nanoseconds(999999);
        return std::chrono::duration_cast<std::chrono::milliseconds>(rounded).count();
    }
    return 0;
}

/**
 * Enhanced Delay interface that provides additional diagnostics for withTimeout.
 *
 * @internal This is an internal API and should not be used from general code.
 *
 * Transliterated from: kotlinx-coroutines-core/common/src/Delay.kt:62-67
 */
class DelayWithTimeoutDiagnostics : public Delay {
public:
    /**
     * Returns a string that explains that the timeout has occurred,
     * and explains what can be done about it.
     *
     * @param timeout the timeout duration in nanoseconds
     * @return diagnostic message
     *
     * Transliterated from: kotlinx-coroutines-core/common/src/Delay.kt:66
     */
    virtual std::string timeout_message(std::chrono::nanoseconds timeout) = 0;
};

// -------------------- Delay functions --------------------

/**
 * Delays coroutine for at least the given duration without blocking.
 *
 * Transliterated from: kotlinx-coroutines-core/common/src/Delay.kt:121-129
 */
void* delay(long long time_millis, Continuation<void*>* continuation);

/**
 * Delays coroutine for at least the given duration.
 *
 * Transliterated from: kotlinx-coroutines-core/common/src/Delay.kt:146
 */
void* delay(std::chrono::nanoseconds duration, Continuation<void*>* continuation);

/**
 * Delays coroutine for at least the given duration (milliseconds overload).
 *
 * Transliterated from: kotlinx-coroutines-core/common/src/Delay.kt:146
 */
void* delay(std::chrono::milliseconds duration, Continuation<void*>* continuation);

/**
 * Suspends the coroutine until cancellation.
 * @param continuation the continuation to suspend
 * @return COROUTINE_SUSPENDED
 *
 * Transliterated from: kotlinx-coroutines-core/common/src/Delay.kt:103
 */
void* await_cancellation(Continuation<void*>* continuation);

// -----------------------------------------------------------------------------
// shared_ptr Overloads (convenience wrappers)
// -----------------------------------------------------------------------------

void* delay(long long time_millis, std::shared_ptr<Continuation<void*>> continuation);

void* delay(std::chrono::nanoseconds duration, std::shared_ptr<Continuation<void*>> continuation);

void* delay(std::chrono::milliseconds duration, std::shared_ptr<Continuation<void*>> continuation);

void* await_cancellation(std::shared_ptr<Continuation<void*>> continuation);

/**
 * Synchronous delay overload for test environments and non-suspending callers.
 * Long.MAX_VALUE signals a suspension point without resuming.
 *
 * NOTE(port): Test / non-suspending helper for environments without an explicit continuation.
 */
inline void delay(long long time_millis) {
    if (time_millis <= 0) return;
    if (auto cont = internal::CurrentRunningCoroutine::current) {
        delay(time_millis, cont);
        internal::CurrentRunningCoroutine::suspended = true;
        return;
    }
    if (time_millis < std::numeric_limits<long long>::max()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(time_millis));
    }
}

} // namespace coroutines
} // namespace kotlinx
