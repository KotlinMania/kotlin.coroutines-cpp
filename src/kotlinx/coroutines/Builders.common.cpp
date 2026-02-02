// port-lint: source Builders.common.kt
/**
 * @file Builders.common.cpp
 * @brief Implementation of coroutine builder helper classes
 *
 * Transliterated from: kotlinx-coroutines-core/common/src/Builders.common.kt
 *
 * Kotlin imports:
 * - kotlinx.coroutines.internal.*
 * - kotlinx.coroutines.intrinsics.*
 * - kotlinx.coroutines.selects.*
 */

#include "kotlinx/coroutines/Builders.hpp"
#include "kotlinx/coroutines/internal/ScopeCoroutine.hpp"
#include "kotlinx/coroutines/intrinsics/Intrinsics.hpp"
#include "kotlinx/coroutines/selects/Select.hpp"

namespace kotlinx::coroutines {

// ---------------------------------------------------------------------------
// StandaloneCoroutine implementation
// ---------------------------------------------------------------------------

StandaloneCoroutine::StandaloneCoroutine(std::shared_ptr<CoroutineContext> parentContext, bool active)
    : AbstractCoroutine<Unit>(parentContext, true, active),
      parent_context_ref(parentContext) {}

bool StandaloneCoroutine::handle_job_exception(std::exception_ptr exception) {
    /*
     * TODO: STUB - Exception handling not implemented
     *
     * Kotlin source: StandaloneCoroutine.handleJobException() in Builders.common.kt
     *
     * What's missing:
     * - Should call handleCoroutineException(context, exception) to propagate
     *   the exception to the CoroutineExceptionHandler in the context
     * - handleCoroutineException() needs to be implemented first
     *
     * Current behavior: Silently swallows exception and returns true (handled)
     * Correct behavior: Propagate to CoroutineExceptionHandler, then return true
     */
    (void)exception;
    return true;
}

// ---------------------------------------------------------------------------
// LazyStandaloneCoroutine implementation
// ---------------------------------------------------------------------------

LazyStandaloneCoroutine::LazyStandaloneCoroutine(
    std::shared_ptr<CoroutineContext> parentContext,
    std::function<void(CoroutineScope*)> block_param
) : StandaloneCoroutine(parentContext, false),
    block(block_param) {}

void LazyStandaloneCoroutine::on_start() {
    /*
     * TODO: STUB - Lazy coroutine startup not implemented
     *
     * Kotlin source: LazyStandaloneCoroutine.onStart() in Builders.common.kt
     *
     * What's missing:
     * - Should call: continuation.startCoroutineCancellable(this)
     * - startCoroutineCancellable() wraps the block in a CancellableContinuation
     *   and dispatches it through the coroutine's dispatcher
     * - Requires: CancellableContinuationImpl, DispatchedContinuation integration
     *
     * Current behavior: Does nothing - lazy coroutine never actually starts
     * Correct behavior: Start the coroutine block with cancellation support
     *
     * Workaround: Use CoroutineStart::DEFAULT instead of LAZY for now
     */
}

} // namespace kotlinx::coroutines
