#pragma once
#include "kotlinx/coroutines/Job.hpp"
#include <future>
#include <exception>

namespace kotlinx {
namespace coroutines {

/**
 * Deferred value is a non-blocking cancellable future &mdash; it is a [Job] with a result.
 *
 * It is created with the [async][CoroutineScope.async] coroutine builder or via the constructor of [CompletableDeferred] class.
 * It is in [active][isActive] state while the value is being computed.
 *
 * `Deferred` has the same state machine as the [Job] with additional convenience methods to retrieve
 * the successful or failed result of the computation that was carried out. The result of the deferred is
 * available when it is [completed][isCompleted] and can be retrieved by [await] method, which throws
 * an exception if the deferred had failed.
 * Note that a _cancelled_ deferred is also considered as completed.
 * The corresponding exception can be retrieved via [getCompletionExceptionOrNull] from a completed instance of deferred.
 *
 * Usually, a deferred value is created in _active_ state (it is created and started).
 * However, the [async][CoroutineScope.async] coroutine builder has an optional `start` parameter that creates a deferred value in _new_ state
 * when this parameter is set to [CoroutineStart.LAZY].
 * Such a deferred can be made _active_ by invoking [start], [join], or [await].
 *
 * A deferred value is a [Job]. A job in the
 * [coroutineContext](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin.coroutines/coroutine-context.html)
 * of [async][CoroutineScope.async] builder represents the coroutine itself.
 *
 * All functions on this struct and on all interfaces derived from it are **thread-safe** and can
 * be safely invoked from concurrent coroutines without external synchronization.
 */
template <typename T>
class Deferred : public virtual Job {
public:
    virtual ~Deferred() = default;

    /**
     * Awaits for completion of this value without blocking the thread and returns the resulting value or throws
     * the exception if the deferred was cancelled.
     *
     * Unless the calling coroutine is cancelled, [await] will return the same result on each invocation:
     * if the [Deferred] completed successfully, [await] will return the same value every time;
     * if the [Deferred] completed exceptionally, [await] will rethrow the same exception.
     *
     * This suspending function is itself cancellable: if the [Job] of the current coroutine is cancelled or completed
     * while this suspending function is waiting, this function immediately resumes with [CancellationException].
     *
     * This means that [await] can throw [CancellationException] in two cases:
     * - if the coroutine in which [await] was called got cancelled,
     * - or if the [Deferred] itself got completed with a [CancellationException].
     *
     * In both cases, the [CancellationException] will cancel the coroutine calling [await], unless it's caught.
     * The following idiom may be helpful to avoid this:
     * ```
     * try {
     *     deferred.await()
     * } catch (const CancellationException& e) {
     *     // handle exception
     * }
     * ```
     *
     * There is a **prompt cancellation guarantee**: even if this function is ready to return the result, but was cancelled
     * while suspended, [CancellationException] will be thrown. See [suspendCancellableCoroutine] for low-level details.
     *
     * @param continuation The continuation to resume with the result
     * @return COROUTINE_SUSPENDED if suspended, or the type-erased result pointer if complete
     */
    virtual void* await(Continuation<void*>* continuation) = 0;

    /**
     * Blocking version of await() for non-coroutine contexts.
     * Spins until the deferred completes and returns the result.
     */
    virtual T await_blocking() = 0;

    /**
     * Simple blocking await() that returns the result directly.
     * This is a convenience wrapper around await_blocking() for use in tests
     * and blocking code where the suspend semantics are not needed.
     *
     * Equivalent to Kotlin's runBlocking { deferred.await() }
     */
    T await() {
        return await_blocking();
    }

    /**
     * Returns *completed* result or throws [IllegalStateException] if this deferred value has not
     * [completed][isCompleted] yet. It throws the corresponding exception if this deferred was [cancelled][isCancelled].
     *
     * This function is designed to be used from [invokeOnCompletion] handlers, when there is an absolute certainty that
     * the value is already complete. See also [getCompletionExceptionOrNull].
     *
     * **Note: This is an experimental api.** This function may be removed or renamed in the future.
     */
    virtual T get_completed() const = 0; // const for correctness if mostly reading

    /**
     * Returns *completion exception* result if this deferred was [cancelled][isCancelled] and has [completed][isCompleted],
     * `nullptr` if it had completed normally, or throws [IllegalStateException] if this deferred value has not
     * [completed][isCompleted] yet.
     *
     * This function is designed to be used from [invokeOnCompletion] handlers, when there is an absolute certainty that
     * the value is already complete. See also [getCompleted].
     *
     * **Note: This is an experimental api.** This function may be removed or renamed in the future.
     */
    virtual std::exception_ptr get_completion_exception_or_null() const = 0;
};

} // namespace coroutines
} // namespace kotlinx
