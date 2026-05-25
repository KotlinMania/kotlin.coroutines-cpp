#pragma once
/**
 * Transliterated from: kotlinx-coroutines-core/common/src/Deferred.kt
 *
 * Kotlin file header (translated):
 *   package kotlinx.coroutines
 */

#include "kotlinx/coroutines/Continuation.hpp"
#include "kotlinx/coroutines/Job.hpp"
#include "kotlinx/coroutines/selects/Select.hpp"

#include <exception>

namespace kotlinx::coroutines {

/**
 * Deferred value is a non-blocking cancellable future &mdash; it is a [Job] with a result.
 *
 * It is created with the [async][CoroutineScope.async] coroutine builder or via the constructor
 * of [CompletableDeferred] class. It is in [active][isActive] state while the value is being
 * computed.
 *
 * `Deferred` has the same state machine as the [Job] with additional convenience methods to
 * retrieve the successful or failed result of the computation that was carried out. The result
 * of the deferred is available when it is [completed][isCompleted] and can be retrieved by
 * [await] method, which throws an exception if the deferred had failed. Note that a
 * _cancelled_ deferred is also considered as completed. The corresponding exception can be
 * retrieved via [get_completion_exception_or_null] from a completed instance of deferred.
 *
 * All functions on this interface are **thread-safe** and can be safely invoked from concurrent
 * coroutines without external synchronization.
 *
 * Upstream:
 *   @OptIn(ExperimentalSubclassOptIn::class)
 *   @SubclassOptInRequired(InternalForInheritanceCoroutinesApi::class)
 *   public interface Deferred<out T> : Job {
 *       public suspend fun await(): T
 *       public val onAwait: SelectClause1<T>
 *       @ExperimentalCoroutinesApi public fun getCompleted(): T
 *       @ExperimentalCoroutinesApi public fun getCompletionExceptionOrNull(): Throwable?
 *   }
 */
template <typename T>
class Deferred : public virtual Job {
public:
    ~Deferred() override = default;

    /**
     * Upstream: public suspend fun await(): T
     *
     * Awaits for completion of this value without blocking the thread and returns the
     * resulting value or throws the exception if the deferred was cancelled.
     *
     * Suspend translation: signature carries an explicit `Continuation<void*>*` and returns
     * `void*`; `COROUTINE_SUSPENDED` means the function suspended.
     */
    virtual void* await(Continuation<void*>* continuation) = 0;

    /** Upstream: public val onAwait: SelectClause1<T> */
    virtual selects::SelectClause1<T>& on_await() = 0;

    /**
     * Upstream:
     *   @ExperimentalCoroutinesApi
     *   public fun getCompleted(): T
     */
    virtual T get_completed() const = 0;

    /**
     * Upstream:
     *   @ExperimentalCoroutinesApi
     *   public fun getCompletionExceptionOrNull(): Throwable?
     */
    virtual std::exception_ptr get_completion_exception_or_null() const = 0;
};

} // namespace kotlinx::coroutines
