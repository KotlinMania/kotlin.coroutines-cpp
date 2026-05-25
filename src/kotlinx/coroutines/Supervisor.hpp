#pragma once
/**
 * Transliterated from: kotlinx-coroutines-core/common/src/Supervisor.kt
 *
 * Kotlin file header (translated):
 *   @file:OptIn(ExperimentalContracts::class)
 *   @file:Suppress("LEAKED_IN_PLACE_LAMBDA", "WRONG_INVOCATION_KIND")
 *   package kotlinx.coroutines
 */

#include "kotlinx/coroutines/CompletableJob.hpp"
#include "kotlinx/coroutines/Continuation.hpp"
#include "kotlinx/coroutines/CoroutineContext.hpp"
#include "kotlinx/coroutines/CoroutineScope.hpp"
#include "kotlinx/coroutines/Job.hpp"
#include "kotlinx/coroutines/JobImpl.hpp"
#include "kotlinx/coroutines/dsl/Suspend.hpp"
#include "kotlinx/coroutines/internal/ScopeCoroutine.hpp"
#include "kotlinx/coroutines/intrinsics/Intrinsics.hpp"

#include <exception>
#include <functional>
#include <memory>

namespace kotlinx::coroutines {

namespace internal_supervisor {

/**
 * Upstream:
 *   private class SupervisorJobImpl(parent: Job?) : JobImpl(parent) {
 *       override fun childCancelled(cause: Throwable): Boolean = false
 *   }
 */
class SupervisorJobImpl : public JobImpl {
public:
    explicit SupervisorJobImpl(std::shared_ptr<Job> parent) : JobImpl(std::move(parent)) {}

    bool child_cancelled(std::exception_ptr /*cause*/) override { return false; }
};

/**
 * Upstream:
 *   private class SupervisorCoroutine<in T>(
 *       context: CoroutineContext,
 *       uCont: Continuation<T>
 *   ) : ScopeCoroutine<T>(context, uCont) {
 *       override fun childCancelled(cause: Throwable): Boolean = false
 *   }
 */
template <typename T>
class SupervisorCoroutine : public internal::ScopeCoroutine<T> {
public:
    SupervisorCoroutine(
        std::shared_ptr<CoroutineContext> context,
        std::shared_ptr<Continuation<T>> u_cont)
        : internal::ScopeCoroutine<T>(std::move(context), std::move(u_cont)) {}

    bool child_cancelled(std::exception_ptr /*cause*/) override { return false; }
};

} // namespace internal_supervisor

/**
 * Creates a _supervisor_ job object in an active state.
 *
 * A failure or cancellation of a child does not cause the supervisor job to fail and does not
 * affect its other children, so a supervisor can implement a custom policy for handling failures
 * of its children. If a [parent] job is specified, then this supervisor job becomes a child job
 * of the [parent] and is cancelled when the parent fails or is cancelled.
 *
 * Upstream:
 *   @Suppress("FunctionName")
 *   public fun SupervisorJob(parent: Job? = null): CompletableJob = SupervisorJobImpl(parent)
 */
inline std::shared_ptr<CompletableJob> SupervisorJob(std::shared_ptr<Job> parent = nullptr) {
    return std::make_shared<internal_supervisor::SupervisorJobImpl>(std::move(parent));
}

/**
 * Binary-compatibility shim for upstream's `SupervisorJob0`.
 *
 * Upstream:
 *   @Deprecated(level = DeprecationLevel.HIDDEN, ...)
 *   @JvmName("SupervisorJob")
 *   public fun SupervisorJob0(parent: Job? = null): Job = SupervisorJob(parent)
 */
inline std::shared_ptr<Job> SupervisorJob0(std::shared_ptr<Job> parent = nullptr) {
    return SupervisorJob(std::move(parent));
}

/**
 * Creates a [CoroutineScope] with [SupervisorJob] and calls the specified suspend [block] with
 * this scope. The provided scope inherits its [coroutineContext][CoroutineScope.coroutineContext]
 * from the outer scope, using the [Job] from that context as the parent for the new
 * [SupervisorJob]. This function returns as soon as the given block and all its child coroutines
 * are completed.
 *
 * Upstream:
 *   public suspend fun <R> supervisorScope(block: suspend CoroutineScope.() -> R): R {
 *       contract { callsInPlace(block, InvocationKind.EXACTLY_ONCE) }
 *       return suspendCoroutineUninterceptedOrReturn { uCont ->
 *           val coroutine = SupervisorCoroutine(uCont.context, uCont)
 *           coroutine.startUndispatchedOrReturn(coroutine, block)
 *       }
 *   }
 */
template <typename R>
[[suspend]]
inline void* supervisor_scope(
    std::function<R(CoroutineScope&, std::shared_ptr<Continuation<void*>>)> block,
    std::shared_ptr<Continuation<void*>> u_cont) {
    auto coroutine = std::make_shared<internal_supervisor::SupervisorCoroutine<R>>(
        u_cont->get_context(),
        std::dynamic_pointer_cast<Continuation<R>>(u_cont));
    return coroutine->start_undispatched_or_return(*coroutine, std::move(block));
}

} // namespace kotlinx::coroutines
