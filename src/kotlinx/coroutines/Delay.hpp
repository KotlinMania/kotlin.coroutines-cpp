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
#include "kotlin/time/Duration.hpp"

#include <string>

namespace kotlinx {
namespace coroutines {

/**
 * This dispatcher feature is implemented by CoroutineDispatcher implementations
 * that natively support scheduled execution of tasks.
 *
 * Implementation of this interface affects operation of delay() and with_timeout() functions.
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
    virtual void* delay(long long time_millis, std::shared_ptr<Continuation<void*>> continuation);
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

/**
 * Convert this duration to its millisecond value. Durations which have a nanosecond component less than
 * a single millisecond will be rounded up to the next largest millisecond.
 *
 * Transliterated from: kotlinx-coroutines-core/common/src/Delay.kt:155-158
 */
long long to_delay_millis(kotlin::time::Duration duration);

/**
 * Enhanced Delay interface that provides additional diagnostics for with_timeout.
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
     * @param timeout the timeout duration
     * @return diagnostic message
     *
     * Transliterated from: kotlinx-coroutines-core/common/src/Delay.kt:66
     */
    virtual std::string timeout_message(kotlin::time::Duration timeout) = 0;
};

// Transliterated from: kotlinx-coroutines-core/common/src/Delay.kt:146
[[clang::annotate("suspend")]]
void* delay(kotlin::time::Duration duration, Continuation<void*>* continuation);
// Transliterated from: kotlinx-coroutines-core/common/src/Delay.kt:146
[[clang::annotate("suspend")]]
void* delay(kotlin::time::Duration duration, std::shared_ptr<Continuation<void*>> continuation);
// Transliterated from: kotlinx-coroutines-core/common/src/Delay.kt:146
[[clang::annotate("suspend"), clang::annotate("kxs_implicit_continuation")]]
__attribute__((error("delay(duration) requires suspend lowering; use delay(duration, completion) at an explicit ABI boundary")))
void delay(kotlin::time::Duration duration);

// -------------------- Delay functions --------------------

/**
 * Delays coroutine for at least the given duration without blocking.
 *
 * Transliterated from: kotlinx-coroutines-core/common/src/Delay.kt:121-129
 */
[[clang::annotate("suspend")]]
void* delay(long long time_millis, Continuation<void*>* continuation);

/**
 * Suspends the coroutine until cancellation.
 * @param continuation the continuation to suspend
 * @return COROUTINE_SUSPENDED
 *
 * Transliterated from: kotlinx-coroutines-core/common/src/Delay.kt:103
 */
void* await_cancellation(Continuation<void*>* continuation);

// -----------------------------------------------------------------------------
// Owned continuation ABI entries
// -----------------------------------------------------------------------------

// Transliterated from: kotlinx-coroutines-core/common/src/Delay.kt:121-129
[[clang::annotate("suspend")]]
void* delay(long long time_millis, std::shared_ptr<Continuation<void*>> continuation);

// Transliterated from: kotlinx-coroutines-core/common/src/Delay.kt:103
void* await_cancellation(std::shared_ptr<Continuation<void*>> continuation);

// Transliterated from: kotlinx-coroutines-core/common/src/Delay.kt:121-129
// NOTE(port): Authoring intrinsic supplies the current frame at compile time.
[[clang::annotate("suspend"), clang::annotate("kxs_implicit_continuation")]]
__attribute__((error("delay(time_millis) requires suspend lowering; use delay(time_millis, completion) at an explicit ABI boundary")))
void delay(long long time_millis);

} // namespace coroutines
} // namespace kotlinx
