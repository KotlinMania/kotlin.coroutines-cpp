#pragma once
/**
 * @file ChannelSuspend.hpp
 * @brief DSL for channel suspend operations (send/receive).
 *
 * Kotlin source: kotlinx-coroutines-core/common/src/channels/BufferedChannel.kt
 *
 * This provides helper patterns for the suspend paths in BufferedChannel:
 * - sendOnNoWaiterSuspend (lines 141-164)
 * - receiveOnNoWaiterSuspend (lines 706-729)
 * - hasNextOnNoWaiterSuspend (lines 760-781)
 */

#include "kotlinx/coroutines/dsl/CancellableReusable.hpp"
#include "kotlinx/coroutines/Waiter.hpp"
#include <functional>

namespace kotlinx {
namespace coroutines {
namespace dsl {

/**
 * Channel send suspend pattern.
 *
 * Kotlin pattern (BufferedChannel.kt lines 141-164):
 * ```kotlin
 * private suspend fun sendOnNoWaiterSuspend(...) = suspendCancellableCoroutineReusable { cont ->
 *     sendImplOnNoWaiter(segment, index, element, s,
 *         waiter = cont,
 *         onRendezvousOrBuffered = { cont.resume(Unit) },
 *         onClosed = { onClosedSendOnNoWaiterSuspend(element, cont) }
 *     )
 * }
 * ```
 *
 * Usage:
 * ```cpp
 * void* send_on_no_waiter_suspend(ChannelSegment<E>* segment, int index,
 *                                  E element, int64_t s,
 *                                  Continuation<void*>* completion) {
 *     return KXS_CHANNEL_SEND_SUSPEND(completion, cont, {
 *         send_impl_on_no_waiter(segment, index, element, s,
 *             cont,
 *             [cont]() { cont->resume({}); },
 *             [this, element, cont]() { on_closed_send_on_no_waiter_suspend(element, cont); }
 *         );
 *     });
 * }
 * ```
 */
#define KXS_CHANNEL_SEND_SUSPEND(completion, cont_name, block) \
    KXS_SUSPEND_CANCELLABLE_REUSABLE(completion, cont_name, block)

/**
 * Channel receive suspend pattern.
 *
 * Kotlin pattern (BufferedChannel.kt lines 706-729):
 * ```kotlin
 * private suspend fun receiveOnNoWaiterSuspend(...) = suspendCancellableCoroutineReusable { cont ->
 *     receiveImplOnNoWaiter(segment, index, r,
 *         waiter = cont,
 *         onElementRetrieved = { element -> cont.resume(element) },
 *         onClosed = { onClosedReceiveOnNoWaiterSuspend(cont) }
 *     )
 * }
 * ```
 */
#define KXS_CHANNEL_RECEIVE_SUSPEND(completion, cont_name, block) \
    KXS_SUSPEND_CANCELLABLE_REUSABLE(completion, cont_name, block)

/**
 * Channel hasNext suspend pattern.
 *
 * Kotlin pattern (BufferedChannel.kt lines 760-781):
 * ```kotlin
 * private suspend fun hasNextOnNoWaiterSuspend(...) = suspendCancellableCoroutineReusable { cont ->
 *     receiveImplOnNoWaiter(segment, index, r,
 *         waiter = cont,
 *         onElementRetrieved = { element -> cont.resume(true to element) },
 *         onClosed = { cont.resume(false to null) }
 *     )
 * }
 * ```
 */
#define KXS_CHANNEL_HAS_NEXT_SUSPEND(completion, cont_name, block) \
    KXS_SUSPEND_CANCELLABLE_REUSABLE(completion, cont_name, block)

/**
 * Helper for channel suspend with waiter registration.
 *
 * This is the common pattern where we:
 * 1. Create/claim a CancellableContinuationImpl
 * 2. Pass it as a Waiter to the channel operation
 * 3. The segment stores the waiter ref for lifetime management
 * 4. Return COROUTINE_SUSPENDED or result
 *
 * @tparam E Element type
 * @tparam Op Operation function type
 * @param completion The completion continuation
 * @param op The channel operation that takes a Waiter*
 * @return COROUTINE_SUSPENDED or the result
 */
template<typename E, typename Op>
void* channel_suspend_with_waiter(Continuation<void*>* completion, Op&& op) {
    return suspend_cancellable_coroutine_reusable_void(completion,
        [&op](CancellableContinuationImpl<void>* cont) {
            // cont implements Waiter, segment will store waiter ref via shared_from_this_waiter
            op(cont);
        });
}

} // namespace dsl
} // namespace coroutines
} // namespace kotlinx
