// port-lint: source kotlinx-coroutines-core/common/src/Supervisor.kt
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
 *   private class SupervisorCoroutine<in T>(
 *       context: CoroutineContext,
 *       uCont: Continuation<T>
 *   ) : ScopeCoroutine<T>(context, uCont) {
 *       override fun childCancelled(cause: Throwable): Boolean = false
 *   }
 */
// Transliterated from: kotlinx-coroutines-core/common/src/Supervisor.kt:64-69
template <typename T>
class SupervisorCoroutine : public internal::ScopeCoroutine<T> {
public:
    // Transliterated from: kotlinx-coroutines-core/common/src/Supervisor.kt:64-67
    SupervisorCoroutine(
        std::shared_ptr<CoroutineContext> context,
        std::shared_ptr<Continuation<T>> u_cont)
        : internal::ScopeCoroutine<T>(std::move(context), std::move(u_cont)) {}

    // Transliterated from: kotlinx-coroutines-core/common/src/Supervisor.kt:68-68
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
// Transliterated from: kotlinx-coroutines-core/common/src/Supervisor.kt:27-27
std::shared_ptr<CompletableJob> SupervisorJob(std::shared_ptr<Job> parent = nullptr);

/**
 * Binary-compatibility shim for upstream's `SupervisorJob0`.
 *
 * Upstream:
 *   @Deprecated(level = DeprecationLevel.HIDDEN, ...)
 *   @JvmName("SupervisorJob")
 *   public fun SupervisorJob0(parent: Job? = null): Job = SupervisorJob(parent)
 */
// Transliterated from: kotlinx-coroutines-core/common/src/Supervisor.kt:33-33
inline std::shared_ptr<Job> SupervisorJob0(std::shared_ptr<Job> parent = nullptr) {
    return SupervisorJob(std::move(parent));
}

/**
 * Creates a [CoroutineScope] with [SupervisorJob] and calls the specified suspend [block] with this scope.
 * The provided scope inherits its [coroutineContext][CoroutineScope.coroutineContext] from the outer scope, using the
 * [Job] from that context as the parent for the new [SupervisorJob].
 * This function returns as soon as the given block and all its child coroutines are completed.
 *
 * Unlike [coroutineScope], a failure of a child does not cause this scope to fail and does not affect its other children,
 * so a custom policy for handling failures of its children can be implemented. See [SupervisorJob] for additional details.
 *
 * If an exception happened in [block], then the supervisor job is failed and all its children are cancelled.
 * If the current coroutine was cancelled, then both the supervisor job itself and all its children are cancelled.
 *
 * The method may throw a [CancellationException] if the current job was cancelled externally,
 * or rethrow an exception thrown by the given [block].
 */
// Transliterated from: kotlinx-coroutines-core/common/src/Supervisor.kt:50-58
template <typename R, typename Block>
inline void* supervisor_scope(Block&& block, std::shared_ptr<Continuation<void*>> completion) {
    using Value = std::conditional_t<std::is_void_v<R>, Unit, R>;
    auto coroutine = std::make_shared<internal_supervisor::SupervisorCoroutine<Value>>(
        completion->get_context(), internal::result_box_completion<Value>(completion));
    return coroutine->start_undispatched_or_return(
        [coroutine, block = std::forward<Block>(block)](std::shared_ptr<Continuation<void*>> continuation) mutable -> void* {
            return block(static_cast<CoroutineScope*>(coroutine.get()), std::move(continuation));
        });
}

} // namespace kotlinx::coroutines
