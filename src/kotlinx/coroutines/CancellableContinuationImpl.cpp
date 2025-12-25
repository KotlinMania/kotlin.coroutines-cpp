/**
 * @file CancellableContinuationImpl.cpp
 * @brief Implementation of CancellableContinuationImpl<void> methods.
 *
 * The void specialization's methods that depend on DispatchedContinuation live here.
 * This avoids circular includes: DispatchedContinuation.hpp includes CancellableContinuationImpl.hpp,
 * so we can't use DispatchedContinuation in the header. The .cpp includes both, solving the cycle.
 *
 * Template CancellableContinuationImpl<T> methods remain in the header (template instantiation).
 */

#include "kotlinx/coroutines/CancellableContinuationImpl.hpp"
#include "kotlinx/coroutines/internal/DispatchedContinuation.hpp"

namespace kotlinx {
namespace coroutines {

// Kotlin line 138
bool CancellableContinuationImpl<void>::is_reusable() const {
    if (!is_reusable_mode(this->resume_mode)) return false;
    auto dispatched = std::dynamic_pointer_cast<internal::DispatchedContinuation<void>>(delegate);
    return dispatched && dispatched->is_reusable();
}

// Kotlin lines 351-356
void CancellableContinuationImpl<void>::release_claimed_reusable_continuation() {
    auto dispatched = std::dynamic_pointer_cast<internal::DispatchedContinuation<void>>(delegate);
    if (!dispatched) return;

    std::exception_ptr cancellation_cause = dispatched->try_release_claimed_continuation(this);
    if (!cancellation_cause) return;

    detach_child();
    cancel(cancellation_cause);
}

// Kotlin lines 194-199
bool CancellableContinuationImpl<void>::cancel_later(std::exception_ptr cause) {
    if (!is_reusable()) return false;
    auto dispatched = std::dynamic_pointer_cast<internal::DispatchedContinuation<void>>(delegate);
    if (!dispatched) return false;
    return dispatched->postpone_cancellation(cause);
}

} // namespace coroutines
} // namespace kotlinx