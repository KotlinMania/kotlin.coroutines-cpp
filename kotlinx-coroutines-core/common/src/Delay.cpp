#include <string>
#include "kotlinx/coroutines/core_fwd.hpp"
// Transliterated from Kotlin to C++ (first-pass, mechanical syntax mapping)
// Original: kotlinx-coroutines-core/common/src/Delay.kt
//
// TODO:
// - suspend functions need coroutine infrastructure
// - kotlin.time.Duration needs C++ chrono or custom duration type
// - CancellableContinuation template infrastructure
// - CoroutineContext and CoroutineDispatcher integration
// - Select infrastructure
// - Extension properties (CoroutineContext.delay) need separate implementation

#include <functional>
#include <chrono>

namespace kotlinx {
namespace coroutines {

// Forward declarations
class CoroutineContext;
class CoroutineDispatcher;
class DisposableHandle;
class Runnable;
template<typename T> class CancellableContinuation;

/**
 * This dispatcher _feature_ is implemented by [CoroutineDispatcher] implementations that natively support
 * scheduled execution of tasks.
 *
 * Implementation of this struct affects operation of
 * [delay][kotlinx.coroutines.delay] and [withTimeout] functions.
 *
 * @suppress **This an API and should not be used from general code.**
 */
// @InternalCoroutinesApi
class Delay {
public:
    /** @suppress **/
    // @Deprecated(
    //     message = "Deprecated without replacement as an method never intended for use",
    //     level = DeprecationLevel.ERROR
    // ) // Error since 1.6.0
    // TODO: suspend function - coroutine semantics not implemented
    virtual void delay(long time) {
        if (time <= 0) return; // don't delay
        // return suspendCancellableCoroutine { scheduleResumeAfterDelay(time, it) }
    }

    /**
     * Schedules resume of a specified [continuation] after a specified delay [timeMillis].
     *
     * Continuation **must be scheduled** to resume even if it is already cancelled, because a cancellation is just
     * an exception that the coroutine that used `delay` might wanted to catch and process. It might
     * need to close some resources in its `finally` blocks, for example.
     *
     * This implementation is supposed to use dispatcher's native ability for scheduled execution in its thread(s).
     * In order to avoid an extra delay of execution, the following code shall be used to resume this
     * [continuation] when the code is already executing in the appropriate thread:
     *
     * ```kotlin
     * with(continuation) { resumeUndispatchedWith(Unit) }
     * ```
     */
    virtual void schedule_resume_after_delay(long time_millis, CancellableContinuation<void>& continuation) = 0;

    /**
     * Schedules invocation of a specified [block] after a specified delay [timeMillis].
     * The resulting [DisposableHandle] can be used to [dispose][DisposableHandle.dispose] of this invocation
     * request if it is not needed anymore.
     */
    virtual DisposableHandle* invoke_on_timeout(long time_millis, Runnable& block, CoroutineContext& context);

    virtual ~Delay() = default;
};

/**
 * Enhanced [Delay] struct that provides additional diagnostics for [withTimeout].
 * Is going to be removed once there is proper JVM-default support.
 * Then we'll be able put this function into [Delay] without breaking binary compatibility.
 */
// @InternalCoroutinesApi
class DelayWithTimeoutDiagnostics : Delay {
public:
    /**
     * Returns a string that explains that the timeout has occurred, and explains what can be done about it.
     */
    virtual std::string timeout_message(std::chrono::nanoseconds timeout) = 0;
};

/**
 * Suspends until cancellation, in which case it will throw a [CancellationException].
 *
 * This function returns [Nothing], so it can be used in any coroutine,
 * regardless of the required return type.
 *
 * Usage example in callback adapting code:
 *
 * ```kotlin
 * auto current_temperature(): Flow<Temperature> = callbackFlow {
 *     auto callback = SensorCallback { degreesCelsius: Double ->
 *         trySend(Temperature.celsius(degreesCelsius))
 *     }
 *     try {
 *         registerSensorCallback(callback)
 *         awaitCancellation() // Suspends to keep getting updates until cancellation.
 *     } finally {
 *         unregisterSensorCallback(callback)
 *     }
 * }
 * ```
 *
 * Usage example in (non declarative) UI code:
 *
 * ```kotlin
 * auto show_stuff_until_cancelled(content: Stuff): Nothing {
 *     someSubView.text = content.title
 *     anotherSubView.text = content.description
 *     someView.visibleInScope {
 *         awaitCancellation() // Suspends so the view stays visible.
 *     }
 * }
 * ```
 */
// TODO: suspend function returning Nothing - coroutine semantics not implemented
[[noreturn]] void await_cancellation(); // = suspendCancellableCoroutine {}

/**
 * Delays coroutine for at least the given time without blocking a thread and resumes it after a specified time.
 * If the given [timeMillis] is non-positive, this function returns immediately.
 *
 * This suspending function is cancellable: if the [Job] of the current coroutine is cancelled while this
 * suspending function is waiting, this function immediately resumes with [CancellationException].
 * There is a **prompt cancellation guarantee**: even if this function is ready to return the result, but was cancelled
 * while suspended, [CancellationException] will be thrown. See [suspendCancellableCoroutine] for low-level details.
 *
 * If you want to delay forever (until cancellation), consider using [awaitCancellation] instead.
 *
 * Note that delay can be used in [select] invocation with [onTimeout][SelectBuilder.onTimeout] clause.
 *
 * Implementation note: how exactly time is tracked is an implementation detail of [CoroutineDispatcher] in the context.
 * @param timeMillis time in milliseconds.
 */
// TODO: suspend function - coroutine semantics not implemented
void delay(long time_millis);

/**
 * Delays coroutine for at least the given [duration] without blocking a thread and resumes it after the specified time.
 * If the given [duration] is non-positive, this function returns immediately.
 *
 * This suspending function is cancellable: if the [Job] of the current coroutine is cancelled while this
 * suspending function is waiting, this function immediately resumes with [CancellationException].
 * There is a **prompt cancellation guarantee**: even if this function is ready to return the result, but was cancelled
 * while suspended, [CancellationException] will be thrown. See [suspendCancellableCoroutine] for low-level details.
 *
 * If you want to delay forever (until cancellation), consider using [awaitCancellation] instead.
 *
 * Note that delay can be used in [select] invocation with [onTimeout][SelectBuilder.onTimeout] clause.
 *
 * Implementation note: how exactly time is tracked is an implementation detail of [CoroutineDispatcher] in the context.
 */
// TODO: suspend function - coroutine semantics not implemented
void delay(std::chrono::nanoseconds duration);

/** Returns [Delay] implementation of the given context */
// TODO: Extension property - needs implementation
// auto CoroutineContext.delay: Delay get() { return get(ContinuationInterceptor) as* Delay ?: DefaultDelay; }

/**
 * Convert this duration to its millisecond value. Durations which have a nanosecond component less than
 * a single millisecond will be rounded up to the next largest millisecond.
 */
// TODO: Extension function on Duration
// auto Duration__dot__toDelayMillis(): Long = when (isPositive()) {
//     true -> plus(999_999L.nanoseconds).inWholeMilliseconds
//     false -> 0L
// }

} // namespace coroutines
} // namespace kotlinx
