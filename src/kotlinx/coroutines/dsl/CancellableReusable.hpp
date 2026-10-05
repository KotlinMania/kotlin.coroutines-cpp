#pragma once
/**
 * @file CancellableReusable.hpp
 * @brief DSL for suspendCancellableCoroutineReusable pattern.
 *
 * Kotlin source: kotlinx-coroutines-core/common/src/CancellableContinuation.kt
 * Lines 442-455: suspendCancellableCoroutineReusable
 *
 * This is an optimized version of suspendCancellableCoroutine that reuses
 * CancellableContinuationImpl instances when possible. Used extensively in
 * BufferedChannel, Mutex, and Semaphore.
 */

#include "kotlinx/coroutines/CancellableContinuationImpl.hpp"
#include "kotlinx/coroutines/internal/DispatchedContinuation.hpp"
#include "kotlinx/coroutines/intrinsics/Intrinsics.hpp"
#include <memory>
#include <functional>

namespace kotlinx {
namespace coroutines {
namespace dsl {

/**
 * Claims and resets a reusable continuation when the intercepted delegate supports it.
 * Returns actual shared ownership; otherwise creates a continuation in the appropriate mode.
 */
template<typename T>
std::shared_ptr<CancellableContinuationImpl<T>> get_or_create_cancellable_continuation(
    std::shared_ptr<Continuation<T>> delegate
) {
    if (auto dispatched = std::dynamic_pointer_cast<internal::DispatchedContinuation<T>>(delegate)) {
        auto reusable = dispatched->claim_reusable_cancellable_continuation();
        if (reusable && reusable->reset_state_reusable()) return reusable;
        return std::make_shared<CancellableContinuationImpl<T>>(delegate, MODE_CANCELLABLE_REUSABLE);
    }
    return std::make_shared<CancellableContinuationImpl<T>>(delegate, MODE_CANCELLABLE);
}

/**
 * Suspend with a reusable CancellableContinuation.
 *
 * Kotlin: suspendCancellableCoroutineReusable { cont -> ... }
 * From CancellableContinuation.kt lines 456-479
 *
 * Usage:
 * ```cpp
 * void* my_suspend_function(Continuation<void*>* completion) {
 *     return suspend_cancellable_coroutine_reusable<void>(completion,
 *         [](CancellableContinuationImpl<void>* cont) {
 *             // Use cont...
 *             // cont->resume({}) when ready
 *         });
 * }
 * ```
 *
 * @param completion The completion continuation
 * @param block The block that receives the cancellable continuation
 * @return COROUTINE_SUSPENDED or the result
 */
template<typename T, typename Block>
void* suspend_cancellable_coroutine_reusable(
    Continuation<void*>* completion,
    Block&& block
) {
    // Kotlin: val cancellable = getOrCreateCancellableContinuation(uCont.intercepted())
    auto cont = get_or_create_cancellable_continuation<T>(
        std::shared_ptr<Continuation<T>>(completion, [](Continuation<T>*){})
    );

    try {
        // Kotlin: block(cancellable)
        block(cont.get());
    } catch (...) {
        // Kotlin: Issue #3613 - release claimed continuation on exception
        // This is important to prevent leaking the reusable state
        cont->release_claimed_reusable_continuation();
        throw;
    }

    // Kotlin: return cancellable.getResult()
    // This returns COROUTINE_SUSPENDED or the actual result
    return cont->get_result();
}

/**
 * Void specialization for Unit-returning suspend functions.
 *
 * This is the common case for channel operations like send/receive.
 */
template<typename Block>
void* suspend_cancellable_coroutine_reusable_void(
    Continuation<void*>* completion,
    Block&& block
) {
    return suspend_cancellable_coroutine_reusable<void>(completion, std::forward<Block>(block));
}

} // namespace dsl
} // namespace coroutines
} // namespace kotlinx

/**
 * Macro for inline suspend with reusable continuation.
 *
 * This is the macro equivalent of Kotlin's:
 *   suspendCancellableCoroutineReusable { cont -> ... }
 *
 * Usage:
 * ```cpp
 * KXS_SUSPEND_CANCELLABLE_REUSABLE(completion, cont, {
 *     // Use cont (CancellableContinuationImpl<void>*)
 *     some_async_op([cont]() {
 *         cont->resume({});
 *     });
 * });
 * ```
 */
#define KXS_SUSPEND_CANCELLABLE_REUSABLE(completion, cont_name, block) \
    ::kotlinx::coroutines::dsl::suspend_cancellable_coroutine_reusable_void( \
        completion, \
        [&](::kotlinx::coroutines::CancellableContinuationImpl<void>* cont_name) block \
    )
