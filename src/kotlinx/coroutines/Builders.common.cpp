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

/**
 * Upstream:
 *   override fun handleJobException(exception: Throwable): Boolean {
 *       handleCoroutineException(context, exception)
 *       return true
 *   }
 */
bool StandaloneCoroutine::handle_job_exception(std::exception_ptr exception) {
    auto ctx = this->get_context();
    handle_coroutine_exception(*ctx, exception);
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

/**
 * Upstream:
 *   private val continuation = block.createCoroutineUnintercepted(this, this)
 *   override fun onStart() {
 *       continuation.startCoroutineCancellable(this)
 *   }
 */
void LazyStandaloneCoroutine::on_start() {
    auto continuation = intrinsics::create_coroutine_unintercepted<CoroutineScope*, Unit>(
        block,
        this->shared_from_this_as<Continuation<Unit>>(),
        this);
    intrinsics::start_coroutine_cancellable(continuation, this);
}

} // namespace kotlinx::coroutines
