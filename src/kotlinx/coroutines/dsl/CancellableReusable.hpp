#pragma once
/**
 * @file CancellableReusable.hpp
 * @brief Owning reusable cancellable continuation helpers for the lowered ABI.
 *
 * Transliterated from: kotlinx-coroutines-core/common/src/CancellableContinuation.kt:442-479
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

namespace detail {
template<typename T>
class ReusableResultAdapter final : public Continuation<T> {
    std::shared_ptr<Continuation<void*>> outer_;
public:
    explicit ReusableResultAdapter(std::shared_ptr<Continuation<void*>> outer)
        : outer_(std::move(outer)) {}

    std::shared_ptr<CoroutineContext> get_context() const override {
        return outer_ ? outer_->get_context() : nullptr;
    }

    void resume_with(Result<T> result) override {
        if (!outer_) return;
        if (result.is_failure()) {
            outer_->resume_with(Result<void*>::failure(result.exception_or_null()));
        } else if constexpr (std::is_void_v<T>) {
            outer_->resume_with(Result<void*>::success(nullptr));
        } else {
            outer_->resume_with(Result<void*>::success(new T(result.get_or_throw())));
        }
    }
};
} // namespace detail

/**
 * Retains and intercepts the lowered completion, claims a typed reusable continuation,
 * invokes the block and returns its result or the suspension marker. A successful typed
 * result is an owned heap box on both direct and resumed paths; void returns nullptr.
 * Unexpected block failure releases the claim before propagating the exception.
 */
template<typename T, typename Block>
void* suspend_cancellable_coroutine_reusable(
    Continuation<void*>* completion,
    Block&& block
) {
    auto intercepted = intrinsics::intercepted(internal::retain_continuation(completion));
    std::shared_ptr<Continuation<T>> delegate;
    if (auto dispatched = std::dynamic_pointer_cast<internal::DispatchedContinuation<void*>>(intercepted)) {
        delegate = dispatched->template typed_reusable_delegate<T>([outer = dispatched->continuation] {
            return std::make_shared<detail::ReusableResultAdapter<T>>(outer);
        });
    } else {
        delegate = std::make_shared<detail::ReusableResultAdapter<T>>(std::move(intercepted));
    }
    auto cont = get_or_create_cancellable_continuation<T>(std::move(delegate));
    try {
        block(cont.get());
    } catch (...) {
        cont->release_claimed_reusable_continuation();
        throw;
    }
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
 * Runs an inline block with a reusable void continuation and returns through the lowered ABI.
 */
#define KXS_SUSPEND_CANCELLABLE_REUSABLE(completion, cont_name, block) \
    ::kotlinx::coroutines::dsl::suspend_cancellable_coroutine_reusable_void( \
        completion, \
        [&](::kotlinx::coroutines::CancellableContinuationImpl<void>* cont_name) block \
    )
