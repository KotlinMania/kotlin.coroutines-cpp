#pragma once

/**
 * Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/FlowCoroutine.kt
 */

#include "kotlinx/coroutines/CoroutineScope.hpp"
#include "kotlinx/coroutines/internal/ScopeCoroutine.hpp"
#include "kotlinx/coroutines/flow/Flow.hpp"
#include <functional>

namespace kotlinx::coroutines::flow::internal {

template <typename T>
class FlowCoroutine : public kotlinx::coroutines::internal::ScopeCoroutine<T> {
public:
    FlowCoroutine(std::shared_ptr<CoroutineContext> context, std::shared_ptr<Continuation<T>> uCont)
        : kotlinx::coroutines::internal::ScopeCoroutine<T>(context, uCont) {}

    bool child_cancelled(std::exception_ptr cause) override {
        // Kotlin: if (cause is ChildCancelledException) return true
        // TODO(semantics): check exception type properly
        // For now we assume typical cancellation propagation
        if (cause) {
            // Placeholder type check
             // if (is_child_cancelled_exception(cause)) return true;
        }
        return this->cancel_impl(cause);
    }
};

/**
 * Creates a [CoroutineScope] and calls the specified suspend block with this scope.
 * This builder is similar to [coroutineScope] with the only exception that it *ties* lifecycle of children
 * and itself regarding the cancellation, thus being cancelled when one of the children becomes cancelled.
 *
 * For example:
 * ```
 * flowScope {
 *     launch {
 *         throw CancellationException()
 *     }
 * } // <- CE will be rethrown here
 * ```
 */
template <typename R>
void* flow_scope(std::function<void*(CoroutineScope*, Continuation<R>*)> block, Continuation<R>* cont) {
    // Kotlin: suspendCoroutineUninterceptedOrReturn { uCont ->
    //    val coroutine = FlowCoroutine(uCont.context, uCont)
    //    coroutine.startUndispatchedOrReturn(coroutine, block)
    // }
    
    // We assume cont is the uCont here.
    auto uCont = std::dynamic_pointer_cast<Continuation<R>>(cont->shared_from_this());
    if (!uCont) return nullptr; // Should not happen

    auto coroutine = std::make_shared<FlowCoroutine<R>>(uCont->get_context(), uCont);
    
    // Wrapping block to match signature expected by start_undispatched_or_return
    // FlowCoroutine extends ScopeCoroutine which has start_undispatched_or_return_ignore_timeout, 
    // but here we want startUndispatchedOrReturn logic.
    // Usually startUndispatchedOrReturn is on ScopeCoroutine.
    
    // For now, mapping to start_undispatched_or_return_ignore_timeout logic as closest match:
    return coroutine->start_undispatched_or_return_ignore_timeout(coroutine, [block](CoroutineScope& scope) -> R {
        // This lambda needs to suspend and wait if block suspends.
        // It's a bit complex to map the void* return type here.
        // Simplified: return default R
        return R{}; 
    });
}

namespace {
    // Forward declaration for scopedFlow
    template <typename R>
    struct ScopedFlowImpl : public Flow<R> {
        std::function<void*(CoroutineScope*, FlowCollector<R>*, Continuation<void*>*)> block;

        ScopedFlowImpl(std::function<void*(CoroutineScope*, FlowCollector<R>*, Continuation<void*>*)> b) : block(b) {}

        void* collect(FlowCollector<R>* collector, Continuation<void*>* cont) override {
             return flow_scope<void*>([this, collector](CoroutineScope* scope, Continuation<void*>* c) -> void* {
                 return this->block(scope, collector, c);
             }, cont);
        }
    };
}

/**
 * Creates a flow that also provides a [CoroutineScope] for each collector
 * Shorthand for:
 * ```
 * flow {
 *     flowScope {
 *         ...
 *     }
 * }
 * ```
 * with additional constraint on cancellation.
 * To cancel child without cancelling itself, `cancel(ChildCancelledException())` should be used.
 */
template <typename R>
Flow<R>* scoped_flow(std::function<void*(CoroutineScope*, FlowCollector<R>*, Continuation<void*>*)> block) {
    return new ScopedFlowImpl<R>(block);
}

} // namespace kotlinx::coroutines::flow::internal
