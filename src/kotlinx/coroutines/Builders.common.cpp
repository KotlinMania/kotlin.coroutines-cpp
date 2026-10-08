// port-lint: source kotlinx-coroutines-core/common/src/Builders.common.kt
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
#include "kotlinx/coroutines/Dispatchers.hpp"

namespace kotlinx::coroutines {

// Transliterated from: kotlinx-coroutines-core/common/src/Builders.common.kt:43-54
std::shared_ptr<Job> launch(
    CoroutineScope* scope, std::shared_ptr<CoroutineContext> context,
    CoroutineStart start,
    std::function<void*(CoroutineScope*, std::shared_ptr<Continuation<void*>>)> block) {
    auto new_context = new_coroutine_context(
        scope, std::move(context));
    std::shared_ptr<StandaloneCoroutine> coroutine;
    if (start == CoroutineStart::LAZY) {
        coroutine = std::make_shared<LazyStandaloneCoroutine>(new_context, block);
    } else {
        coroutine = std::make_shared<StandaloneCoroutine>(new_context, true);
    }
    coroutine->start(start, static_cast<CoroutineScope*>(coroutine.get()), std::move(block));
    return coroutine;
}

// Transliterated from: kotlinx-coroutines-core/common/src/Builders.common.kt:43-54
std::shared_ptr<Job> launch(
    CoroutineScope* scope,
    std::function<void*(CoroutineScope*, std::shared_ptr<Continuation<void*>>)> block) {
    return launch(scope, EmptyCoroutineContext::instance(), CoroutineStart::DEFAULT, std::move(block));
}

// Transliterated from: kotlinx-coroutines-core/common/src/Builders.common.kt:43-54
std::shared_ptr<Job> launch(
    CoroutineScope* scope, std::shared_ptr<CoroutineContext> context,
    std::function<void*(CoroutineScope*, std::shared_ptr<Continuation<void*>>)> block) {
    return launch(scope, std::move(context), CoroutineStart::DEFAULT, std::move(block));
}

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

// Transliterated from: kotlinx-coroutines-core/common/src/Builders.common.kt:199-208
LazyStandaloneCoroutine::LazyStandaloneCoroutine(
    std::shared_ptr<CoroutineContext> parent_context,
    std::function<void*(CoroutineScope*, std::shared_ptr<Continuation<void*>>)> block_param
) : StandaloneCoroutine(std::move(parent_context), false),
    suspend_block_(std::move(block_param)) {}

/**
 * Upstream:
 *   private val continuation = block.createCoroutineUnintercepted(this, this)
 *   override fun onStart() {
 *       continuation.startCoroutineCancellable(this)
 *   }
 */
void LazyStandaloneCoroutine::on_start() {
    if (suspend_block_) {
        this->start(CoroutineStart::DEFAULT, static_cast<CoroutineScope*>(this),
                    std::move(suspend_block_));
        return;
    }
    try {
        if (block) {
            block(this);
        }
        this->resume_with(Result<Unit>::success(Unit()));
    } catch (...) {
        this->resume_with(Result<Unit>::failure(std::current_exception()));
    }
}

} // namespace kotlinx::coroutines
