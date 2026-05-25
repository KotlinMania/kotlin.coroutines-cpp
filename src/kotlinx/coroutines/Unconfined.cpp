/**
 * Transliterated from: kotlinx-coroutines-core/common/src/Unconfined.kt
 */

#include "kotlinx/coroutines/Unconfined.hpp"

#include <stdexcept>

namespace kotlinx::coroutines {

namespace {

/** Singleton key instance for YieldContext, mirroring Kotlin's `companion object Key`. */
struct YieldContextKey : public CoroutineContext::Key {};
const YieldContextKey k_yield_context_key{};

} // namespace

const CoroutineContext::Key* YieldContext::type_key() { return &k_yield_context_key; }

void Unconfined::dispatch(const CoroutineContext& context, Runnable& /*block*/) {
    /** Upstream: It can only be called by the [yield] function. */
    auto* element = context.get(*YieldContext::type_key());
    auto* yield_context = dynamic_cast<YieldContext*>(element);
    if (yield_context != nullptr) {
        // report to "yield" that it is an unconfined dispatcher and don't call "block.run()"
        yield_context->dispatcher_was_unconfined = true;
        return;
    }
    throw std::logic_error(
        "Dispatchers.Unconfined.dispatch function can only be used by the yield function. "
        "If you wrap Unconfined dispatcher in your code, make sure you properly delegate "
        "isDispatchNeeded and dispatch calls.");
}

} // namespace kotlinx::coroutines
