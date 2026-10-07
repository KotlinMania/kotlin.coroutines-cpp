#pragma once

/**
 * Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/FlowCoroutine.kt
 */

#include "kotlinx/coroutines/CoroutineScope.hpp"
#include "kotlinx/coroutines/flow/internal/FlowImpl.hpp"
#include "kotlinx/coroutines/flow/Flow.hpp"
#include "kotlinx/coroutines/flow/internal/FlowExceptions.hpp"
#include <functional>

namespace kotlinx::coroutines::flow::internal {

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
// NOTE(port): Internal result generics are erased through the Continuation ABI.
// Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/FlowCoroutine.kt:26-30
void* flow_scope(std::function<void*(CoroutineScope*, Continuation<void*>*)> block,
                 Continuation<void*>* completion);

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
// Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/FlowCoroutine.kt:46-49
template <typename R>
std::shared_ptr<Flow<R>> scoped_flow(
    std::function<void*(CoroutineScope*, FlowCollector<R>*, Continuation<void*>*)> block) {
    return std::make_shared<FlowImpl<R>>(
        [block = std::move(block)](FlowCollector<R>* collector, Continuation<void*>* completion) -> void* {
            return flow_scope([block, collector](CoroutineScope* scope, Continuation<void*>* continuation) {
                return block(scope, collector, continuation);
            }, completion);
        });
}

} // namespace kotlinx::coroutines::flow::internal
