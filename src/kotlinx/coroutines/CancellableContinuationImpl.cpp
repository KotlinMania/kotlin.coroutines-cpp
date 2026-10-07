/**
 * Transliterated from: kotlinx-coroutines-core/common/src/CancellableContinuationImpl.kt
 */

#include "kotlinx/coroutines/CancellableContinuationImpl.hpp"
#include "kotlinx/coroutines/internal/DispatchedContinuation.hpp"
#include <sstream>

namespace kotlinx {
namespace coroutines {
// Transliterated from: kotlinx-coroutines-core/common/src/CancellableContinuationImpl.kt:138-139
bool CancellableContinuationImpl<void>::is_reusable() const {
    if (!is_reusable_mode(this->resume_mode)) return false;
    auto dispatched = std::dynamic_pointer_cast<internal::DispatchedContinuation<void>>(delegate);
    return dispatched && dispatched->is_reusable();
}

// Transliterated from: kotlinx-coroutines-core/common/src/CancellableContinuationImpl.kt:351-356
void CancellableContinuationImpl<void>::release_claimed_reusable_continuation() {
    auto dispatched = std::dynamic_pointer_cast<internal::DispatchedContinuation<void>>(delegate);
    if (!dispatched) return;

    std::exception_ptr cancellation_cause = dispatched->try_release_claimed_continuation(this);
    if (!cancellation_cause) return;

    detach_child();
    cancel(cancellation_cause);
}

// Transliterated from: kotlinx-coroutines-core/common/src/CancellableContinuationImpl.kt:194-199
bool CancellableContinuationImpl<void>::cancel_later(std::exception_ptr cause) {
    if (!is_reusable()) return false;
    auto dispatched = std::dynamic_pointer_cast<internal::DispatchedContinuation<void>>(delegate);
    if (!dispatched) return false;
    return dispatched->postpone_cancellation(cause);
}

// Transliterated from: kotlinx-coroutines-core/common/src/CompletionState.kt:45-54
CancelledContinuation::CancelledContinuation(const void* continuation, std::exception_ptr cause, bool handled)
    : CompletedExceptionally([&] {
          if (cause) return cause;
          // NOTE(port): The erased C++ continuation is rendered by its object address.
          std::ostringstream message;
          message << "Continuation " << continuation << " was cancelled normally";
          return std::make_exception_ptr(CancellationException(message.str()));
      }(), handled) {}

// Transliterated from: kotlinx-coroutines-core/common/src/CancellableContinuationImpl.kt:473-491
State* CancellableContinuationImpl<void>::resumed_state(
    NotCompleted* state, const std::shared_ptr<State>& state_owner, int mode, OnCancellation on_cancellation, void* idempotent) {
    if (!is_cancellable_mode(mode) && idempotent == nullptr) return new CompletedWithValue<void>();
    auto* handler = dynamic_cast<CancelHandler*>(state);
    if (on_cancellation || handler || idempotent) {
        auto handler_owner = handler ? std::dynamic_pointer_cast<CancelHandler>(state_owner) : nullptr;
        return new CompletedCancellableContinuationState<void>(handler_owner, std::move(on_cancellation), idempotent);
    }
    return new CompletedWithValue<void>();
}

// Transliterated from: kotlinx-coroutines-core/common/src/CancellableContinuationImpl.kt:493-523
void CancellableContinuationImpl<void>::resume_impl(int mode, OnCancellation on_cancellation) {
    // NOTE(port): Kotlin retains the GC receiver across parent detachment and dispatch.
    auto receiver_owner = weak_from_this().lock();
    while (true) {
        auto state_owner = std::atomic_load(&state_);
        State* state = state_owner.get();
        if (auto* active = dynamic_cast<NotCompleted*>(state)) {
            std::shared_ptr<State> update(resumed_state(active, state_owner, mode, on_cancellation, nullptr));
            if (!std::atomic_compare_exchange_strong(&state_, &state_owner, update)) {
                continue;
            }

            detach_child_if_non_reusable();
            dispatch_resume(mode);
            return;
        }
        if (auto* cancelled = dynamic_cast<CancelledContinuation*>(state)) {
            // Cancellation is asynchronous and may race with resume. Ignore the first
            // resume attempt; a second resume attempt is an error.
            if (cancelled->make_resumed()) {
                if (on_cancellation) call_on_cancellation(on_cancellation, cancelled->cause);
                return;
            }
        }
        throw std::logic_error("Already resumed, but proposed with update Unit");
    }
}

// Transliterated from: kotlinx-coroutines-core/common/src/CancellableContinuationImpl.kt:493-523
void CancellableContinuationImpl<void>::resume_impl_exception(std::exception_ptr exception, int mode) {
    auto receiver_owner = weak_from_this().lock();
    while (true) {
        auto state_owner = std::atomic_load(&state_);
        State* state = state_owner.get();
        if (dynamic_cast<NotCompleted*>(state)) {
            std::shared_ptr<State> update(new CompletedExceptionState(exception));
            if (!std::atomic_compare_exchange_strong(&state_, &state_owner, update)) {
                continue;
            }

            detach_child_if_non_reusable();
            dispatch_resume(mode);
            return;
        }
        if (auto* cancelled = dynamic_cast<CancelledContinuation*>(state)) {
            if (cancelled->make_resumed()) return; // Racy exceptions are lost, too.
        }
        throw std::logic_error("Already resumed, but proposed with exceptional update");
    }
}

// Transliterated from: kotlinx-coroutines-core/common/src/CancellableContinuationImpl.kt:529-553
void* CancellableContinuationImpl<void>::try_resume_impl(void* idempotent, OnCancellation on_cancellation) {
    auto receiver_owner = weak_from_this().lock();
    while (true) {
        auto state_owner = std::atomic_load(&state_);
        State* state = state_owner.get();
        if (auto* active = dynamic_cast<NotCompleted*>(state)) {
            std::shared_ptr<State> update(resumed_state(active, state_owner, this->resume_mode, on_cancellation, idempotent));
            if (!std::atomic_compare_exchange_strong(&state_, &state_owner, update)) {
                continue;
            }

            detach_child_if_non_reusable();
            return &RESUME_TOKEN;
        }
        if (auto* completed = dynamic_cast<CompletedCancellableContinuationState<void>*>(state)) {
            // Unit values are equal. Identity of the idempotent token must still match.
            return idempotent && completed->idempotent_resume == idempotent ? &RESUME_TOKEN : nullptr;
        }
        return nullptr;
    }
}

// Transliterated from: kotlinx-coroutines-core/common/src/CancellableContinuationImpl.kt:575-576
void* CancellableContinuationImpl<void>::try_resume(void* idempotent) {
    return try_resume_impl(idempotent, nullptr);
}

// Transliterated from: kotlinx-coroutines-core/common/src/CancellableContinuationImpl.kt:578-583
void* CancellableContinuationImpl<void>::try_resume(void* idempotent,
    std::function<void(std::exception_ptr, void*, std::shared_ptr<CoroutineContext>)> on_cancellation) {
    OnCancellation adapted;
    if (on_cancellation) adapted = [on_cancellation](std::exception_ptr cause, std::shared_ptr<CoroutineContext> context) {
        on_cancellation(cause, nullptr, std::move(context));
    };
    return try_resume_impl(idempotent, std::move(adapted));
}

// Transliterated from: kotlinx-coroutines-core/common/src/CancellableContinuationImpl.kt:585-586,529-553
void* CancellableContinuationImpl<void>::try_resume_with_exception(std::exception_ptr exception) {
    auto receiver_owner = weak_from_this().lock();
    while (true) {
        auto state_owner = std::atomic_load(&state_);
        State* state = state_owner.get();
        if (!dynamic_cast<NotCompleted*>(state)) return nullptr;
        std::shared_ptr<State> update(new CompletedExceptionState(exception));
        if (!std::atomic_compare_exchange_strong(&state_, &state_owner, update)) {
            continue;
        }

        detach_child_if_non_reusable();
        return &RESUME_TOKEN;
    }
}

// Transliterated from: kotlinx-coroutines-core/common/src/CancellableContinuationImpl.kt:589-592
void CancellableContinuationImpl<void>::complete_resume(void* token) {
    auto receiver_owner = weak_from_this().lock();
    assert(token == &RESUME_TOKEN);
    dispatch_resume(this->resume_mode);
}

// Transliterated from: kotlinx-coroutines-core/common/src/CancellableContinuationImpl.kt:362-370
void CancellableContinuationImpl<void>::resume(std::function<void(std::exception_ptr)> on_cancellation) {
    OnCancellation adapted;
    if (on_cancellation) adapted = [on_cancellation](std::exception_ptr cause, std::shared_ptr<CoroutineContext>) {
        on_cancellation(cause);
    };
    resume_impl(this->resume_mode, std::move(adapted));
}

// Transliterated from: kotlinx-coroutines-core/common/src/CancellableContinuationImpl.kt:594-597
void CancellableContinuationImpl<void>::resume_undispatched(CoroutineDispatcher* dispatcher) {
    auto dc = std::dynamic_pointer_cast<internal::DispatchedContinuation<void>>(delegate);
    resume_impl(dc && dc->dispatcher.get() == dispatcher ? MODE_UNDISPATCHED : this->resume_mode);
}

// Transliterated from: kotlinx-coroutines-core/common/src/CancellableContinuationImpl.kt:599-602
void CancellableContinuationImpl<void>::resume_undispatched_with_exception(
    CoroutineDispatcher* dispatcher, std::exception_ptr exception) {
    auto dc = std::dynamic_pointer_cast<internal::DispatchedContinuation<void>>(delegate);
    resume_impl_exception(exception, dc && dc->dispatcher.get() == dispatcher ? MODE_UNDISPATCHED : this->resume_mode);
}

// Transliterated from: kotlinx-coroutines-core/common/src/CancellableContinuationImpl.kt:358-360
void CancellableContinuationImpl<void>::resume_with(Result<void> result) {
    if (result.is_success()) resume_impl(this->resume_mode);
    else resume_impl_exception(result.exception_or_null(), this->resume_mode);
}

// Transliterated from: kotlinx-coroutines-core/common/src/CancellableContinuationImpl.kt:247-264
void CancellableContinuationImpl<void>::call_on_cancellation(
    const OnCancellation& on_cancellation, std::exception_ptr cause) {
    try {
        on_cancellation(cause, context_);
    } catch (...) {
        handle_coroutine_exception(*context_, std::make_exception_ptr(
            CompletionHandlerException("Exception in resume onCancellation handler", std::current_exception())));
    }
}


// Transliterated from: kotlinx-coroutines-core/common/src/CancellableContinuationImpl.kt:266-267
std::exception_ptr CancellableContinuationImpl<void>::get_continuation_cancellation_cause(Job& parent) {
        return parent.get_cancellation_exception();
    }

// Transliterated from: kotlinx-coroutines-core/common/src/CancellableContinuationImpl.kt:269-277
bool CancellableContinuationImpl<void>::try_suspend() {
        while (true) {
            int cur = decision_and_index_.load(std::memory_order_acquire);
            int decision = get_decision(cur);
            int index = get_index(cur);
            switch (decision) {
                case UNDECIDED:
                    if (decision_and_index_.compare_exchange_strong(cur, decision_and_index(SUSPENDED, index)))
                        return true;
                    break;
                case RESUMED:
                    return false;
                default:
                    throw std::logic_error("Already suspended");
            }
        }
    }

// Transliterated from: kotlinx-coroutines-core/common/src/CancellableContinuationImpl.kt:279-287
bool CancellableContinuationImpl<void>::try_resume_decision() {
        while (true) {
            int cur = decision_and_index_.load(std::memory_order_acquire);
            int decision = get_decision(cur);
            int index = get_index(cur);
            switch (decision) {
                case UNDECIDED:
                    if (decision_and_index_.compare_exchange_strong(cur, decision_and_index(RESUMED, index)))
                        return true;
                    break;
                case SUSPENDED:
                    return false;
                default:
                    throw std::logic_error("Already resumed");
            }
        }
    }

// Transliterated from: kotlinx-coroutines-core/common/src/CancellableContinuationImpl.kt:169-189
void CancellableContinuationImpl<void>::cancel_completed_result(Result<void> taken_state, std::exception_ptr cause) {
        while (true) {
            auto state_owner = std::atomic_load(&state_);
            State* state = state_owner.get();
            if (dynamic_cast<NotCompleted*>(state)) throw std::runtime_error("Not completed");
            if (dynamic_cast<CompletedExceptionally*>(state)) return;

            // Handle CompletedWithValue - promote to CompletedCancellableContinuationState with cancelCause
            if (dynamic_cast<CompletedWithValue<void>*>(state)) {
                std::shared_ptr<State> update(new CompletedCancellableContinuationState<void>(nullptr, nullptr, nullptr, cause));
                if (std::atomic_compare_exchange_strong(&state_, &state_owner, update)) {
                    return;
                }

                continue;
            }

            if (auto* cc = dynamic_cast<CompletedCancellableContinuationState<void>*>(state)) {
                if (cc->is_cancelled()) throw std::runtime_error("Must be called at most once");
                std::shared_ptr<State> update(new CompletedCancellableContinuationState<void>(cc->cancel_handler, cc->on_cancellation, cc->idempotent_resume, cause));
                if (std::atomic_compare_exchange_strong(&state_, &state_owner, update)) {
                    if (cc->cancel_handler) call_cancel_handler(cc->cancel_handler, cause);
                    if (cc->on_cancellation) call_on_cancellation(cc->on_cancellation, cause);
                    return;
                }

                continue;
            }
            return;
        }
    }

// Transliterated from: kotlinx-coroutines-core/common/src/CancellableContinuationImpl.kt:201-217
bool CancellableContinuationImpl<void>::cancel(std::exception_ptr cause) {
        // NOTE(port): Retain the GC-owned receiver while detaching parent handlers.
        auto self_guard = this->weak_from_this().lock();
        while (true) {
            auto state_owner = std::atomic_load(&state_);
            State* state = state_owner.get();
            // if (state !is NotCompleted) return false
            if (!dynamic_cast<NotCompleted*>(state)) return false;

            // handled = state is CancelHandler || state is Segment<*>
            bool is_cancel_handler = dynamic_cast<CancelHandler*>(state) != nullptr;
            bool is_segment = dynamic_cast<internal::SegmentBase*>(state) != nullptr;
            bool handled = is_cancel_handler || is_segment;

            std::shared_ptr<State> update(new CancelledContinuation(this, cause, handled));

            // Retain the same source state across compare-and-set and handler invocation.
            std::shared_ptr<CancelHandler> handler_to_call;
            if (is_cancel_handler) {
                handler_to_call = std::dynamic_pointer_cast<CancelHandler>(state_owner);
            }

            if (!std::atomic_compare_exchange_strong(&state_, &state_owner, update)) {
                continue;
            }

            // Invoke cancel handler if it was present
            // when (state) { is CancelHandler -> ..., is Segment<*> -> ... }
            if (is_cancel_handler && handler_to_call) {
                call_cancel_handler(handler_to_call, cause);
            } else if (is_segment) {
                call_segment_on_cancellation(dynamic_cast<internal::SegmentBase*>(state), cause);
            }

            detach_child_if_non_reusable();
            dispatch_resume(this->resume_mode);
            return true;
        }
    }

// Transliterated from: kotlinx-coroutines-core/common/src/CancellableContinuationImpl.kt:219-224
void CancellableContinuationImpl<void>::parent_cancelled(std::exception_ptr cause) {
        if (cancel_later(cause)) return;
        cancel(cause);
        // Even if cancellation has failed, we should detach child to avoid potential leak
        detach_child_if_non_reusable();
    }

// Transliterated from: kotlinx-coroutines-core/common/src/CancellableContinuationImpl.kt:226-239
void CancellableContinuationImpl<void>::call_cancel_handler(std::shared_ptr<CancelHandler> handler, std::exception_ptr cause) {
        try {
            handler->invoke(cause);
        } catch (...) {
            // Handler should never fail, if it does -- it is an unhandled exception
            if (context_) {
                handle_coroutine_exception(*context_,
                    std::make_exception_ptr(CompletionHandlerException("Exception in invokeOnCancellation handler", std::current_exception())));
            }
        }
    }

// Transliterated from: kotlinx-coroutines-core/common/src/CancellableContinuationImpl.kt:241-245
void CancellableContinuationImpl<void>::call_segment_on_cancellation(internal::SegmentBase* segment, std::exception_ptr cause) {
        // val index = _decisionAndIndex.value.index
        int index = get_index(decision_and_index_.load(std::memory_order_acquire));
        // check(index != NO_INDEX) { "The index for Segment.onCancellation(..) is broken" }
        if (index == NO_INDEX) {
            throw std::logic_error("The index for Segment.onCancellation(..) is broken");
        }
        // callCancelHandlerSafely { segment.onCancellation(index, cause, context) }
        try {
            segment->on_cancellation(index, cause, context_);
        } catch (...) {
            if (context_) {
                handle_coroutine_exception(*context_,
                    std::make_exception_ptr(CompletionHandlerException("Exception in invokeOnCancellation handler", std::current_exception())));
            }
        }
    }

// Transliterated from: kotlinx-coroutines-core/common/src/CancellableContinuationImpl.kt:339-345
std::shared_ptr<DisposableHandle> CancellableContinuationImpl<void>::install_parent_handle() {
        if (!context_) return nullptr;
        auto job_element = context_->get(Job::type_key);
        auto parent = std::dynamic_pointer_cast<Job>(job_element);
        if (!parent) return nullptr;  // don't do anything without a parent

        // Create ChildContinuation and set its job pointer
        auto child_handler = std::make_shared<ChildContinuation<void>>(this);
        child_handler->job = dynamic_cast<JobSupport*>(parent.get());

        auto self = this->shared_from_this();
        auto handle = parent->invoke_on_completion(
            true,  // onCancelling = true
            true,  // invokeImmediately = true
            [child_handler, self](std::exception_ptr cause) {
                child_handler->invoke(cause);
            }
        );

        // _parentHandle.compareAndSet(null, handle)
        std::shared_ptr<DisposableHandle> expected;
        std::atomic_compare_exchange_strong(&parent_handle_, &expected, handle);
        return handle;
    }

// Transliterated from: kotlinx-coroutines-core/common/src/CancellableContinuationImpl.kt:120-136
void CancellableContinuationImpl<void>::init_cancellability() {
        // Kotlin lines 120-136
        auto handle = install_parent_handle();
        if (!handle) return;  // fast path
        // now check our state _after_ registering
        if (is_completed()) {
            handle->dispose();
            std::atomic_store(&parent_handle_, non_disposable_handle());
        }
    }

// Transliterated from: kotlinx-coroutines-core/common/src/CancellableContinuationImpl.kt:560-563
void CancellableContinuationImpl<void>::detach_child_if_non_reusable() {
        if (!is_reusable()) detach_child();
    }

// Transliterated from: kotlinx-coroutines-core/common/src/CancellableContinuationImpl.kt:568-572
void CancellableContinuationImpl<void>::detach_child() {
        auto handle = std::atomic_load(&parent_handle_);
        if (!handle) return;
        handle->dispose();
        std::atomic_store(&parent_handle_, non_disposable_handle());
    }

// Transliterated from: kotlinx-coroutines-core/common/src/CancellableContinuationImpl.kt:400-461
void CancellableContinuationImpl<void>::invoke_on_cancellation_impl_segment(internal::SegmentBase* segment) {
        while (true) {
            auto state_owner = std::atomic_load(&state_);
            State* state = state_owner.get();

            // Active -> store segment marker
            if (dynamic_cast<Active*>(state)) {
                // SegmentBase implements NotCompleted, matching Kotlin's state union.
                if (std::atomic_compare_exchange_strong(&state_, &state_owner,
                        std::shared_ptr<State>(segment, [](State*) {}))) {
                    return;
                }
                continue;
            }

            if (dynamic_cast<CancelHandler*>(state) || dynamic_cast<internal::SegmentBase*>(state)) {
                throw std::runtime_error("Multiple handlers prohibited");
            }

            // CompletedExceptionally (includes CancelledContinuation) - Kotlin lines 408-429
            if (auto* ex = dynamic_cast<CompletedExceptionally*>(state)) {
                if (!ex->make_handled()) {
                    throw std::runtime_error("Multiple handlers prohibited");
                }
                // Call segment cancellation only if cancelled
                if (dynamic_cast<CancelledContinuation*>(state)) {
                    call_segment_on_cancellation(segment, ex->cause);
                }
                return;
            }

            // CompletedCancellableContinuationState -> segment doesn't need to be called
            // Kotlin line 437-438: if (handler is Segment<*>) return
            if (dynamic_cast<CompletedCancellableContinuationState<void>*>(state)) {
                return;
            }

            // CompletedWithValue -> segment doesn't need to be called on completed continuation
            // Kotlin line 454: if (handler is Segment<*>) return
            if (dynamic_cast<CompletedWithValue<void>*>(state)) {
                return;
            }

            return;
        }
    }

// Transliterated from: kotlinx-coroutines-core/common/src/CancellableContinuationImpl.kt:385-398
void CancellableContinuationImpl<void>::invoke_on_cancellation(std::function<void(std::exception_ptr)> handler) {
        invoke_on_cancellation_impl(std::make_shared<UserSuppliedCancelHandler>(handler));
    }

// Transliterated from: kotlinx-coroutines-core/common/src/CancellableContinuationImpl.kt:400-461
void CancellableContinuationImpl<void>::invoke_on_cancellation_impl(std::shared_ptr<CancelHandler> handler) {
        while (true) {
            auto state_owner = std::atomic_load(&state_);
            State* state = state_owner.get();

            // Active -> store handler as the new state
            if (dynamic_cast<Active*>(state)) {
                if (std::atomic_compare_exchange_strong(&state_, &state_owner, std::static_pointer_cast<State>(handler))) {
                    return;
                }
                continue; // retry on CAS failure
            }

            // Already has a handler -> error (Kotlin line 407)
            if (dynamic_cast<CancelHandler*>(state) || dynamic_cast<internal::SegmentBase*>(state)) {
                throw std::runtime_error("Multiple handlers prohibited");
            }

            // CompletedExceptionally (includes CancelledContinuation) - Kotlin lines 408-429
            if (auto* ex = dynamic_cast<CompletedExceptionally*>(state)) {
                // if (!state.makeHandled()) multipleHandlersError(...)
                if (!ex->make_handled()) {
                    throw std::runtime_error("Multiple handlers prohibited");
                }
                // Call handler only if cancelled (not just exceptionally completed)
                if (dynamic_cast<CancelledContinuation*>(state)) {
                    call_cancel_handler(handler, ex->cause);
                }
                return;
            }

            // CompletedCancellableContinuationState -> copy with handler (Kotlin lines 432-446)
            if (auto* cc = dynamic_cast<CompletedCancellableContinuationState<void>*>(state)) {
                if (cc->cancel_handler) {
                    throw std::runtime_error("Multiple handlers prohibited");
                }
                if (cc->is_cancelled()) {
                    // Was already cancelled while being dispatched -- invoke the handler directly
                    call_cancel_handler(handler, cc->cancel_cause);
                    return;
                }
                std::shared_ptr<State> update(new CompletedCancellableContinuationState<void>(
                    handler, cc->on_cancellation, cc->idempotent_resume, cc->cancel_cause));
                if (std::atomic_compare_exchange_strong(&state_, &state_owner, update)) {
                    return;
                }

                continue;
            }

            // Kotlin lines 448-458: else branch - raw completed value
            // CompletedWithValue -> wrap in CompletedCancellableContinuationState with handler
            if (dynamic_cast<CompletedWithValue<void>*>(state)) {
                std::shared_ptr<State> update(new CompletedCancellableContinuationState<void>(handler, nullptr, nullptr, nullptr));
                if (std::atomic_compare_exchange_strong(&state_, &state_owner, update)) {
                    return;
                }

                continue;
            }

            // Unknown state - should not happen
            return;
        }
    }

// Transliterated from: kotlinx-coroutines-core/common/src/CancellableContinuationImpl.kt:107-107
bool CancellableContinuationImpl<void>::is_active() const { return dynamic_cast<NotCompleted*>(std::atomic_load(&state_).get()); }

// Transliterated from: kotlinx-coroutines-core/common/src/CancellableContinuationImpl.kt:109-109
bool CancellableContinuationImpl<void>::is_completed() const { return !is_active(); }

// Transliterated from: kotlinx-coroutines-core/common/src/CancellableContinuationImpl.kt:111-111
bool CancellableContinuationImpl<void>::is_cancelled() const { return dynamic_cast<CancelledContinuation*>(std::atomic_load(&state_).get()); }

// Transliterated from: kotlinx-coroutines-core/common/src/CancellableContinuationImpl.kt:165-165
Result<void> CancellableContinuationImpl<void>::take_state() {
        auto state_owner = std::atomic_load(&state_);
        State* st = state_owner.get();
        if (auto* ex = dynamic_cast<CompletedExceptionally*>(st)) {
            return Result<void>::failure(ex->cause);
        }
        return Result<void>::success();
    }

// Transliterated from: kotlinx-coroutines-core/common/src/CancellableContinuationImpl.kt:290-337
void* CancellableContinuationImpl<void>::get_result() {
        // val isReusable = isReusable()
        bool is_reusable_flag = is_reusable();

        if (try_suspend()) {
            // if (parentHandle == null) { installParentHandle() }
            if (std::atomic_load(&parent_handle_) == nullptr) {
                install_parent_handle();
            }
            // if (isReusable) { releaseClaimedReusableContinuation() }
            if (is_reusable_flag) {
                release_claimed_reusable_continuation();
            }
            return intrinsics::get_COROUTINE_SUSPENDED();
        }

        // if (isReusable) { releaseClaimedReusableContinuation() }
        if (is_reusable_flag) {
            release_claimed_reusable_continuation();
        }

        auto state_owner = std::atomic_load(&state_);

        State* state = state_owner.get();
        if (auto* ex = dynamic_cast<CompletedExceptionally*>(state)) {
            std::rethrow_exception(ex->cause);
        }

        // Check cancellable mode - Kotlin lines 328-334
        if (is_cancellable_mode(this->resume_mode) && context_) {
            auto job_element = context_->get(Job::type_key);
            if (auto job = std::dynamic_pointer_cast<Job>(job_element)) {
                if (!job->is_active()) {
                    std::exception_ptr cause = job->get_cancellation_exception();
                    cancel_completed_result(Result<void>(), cause);
                    std::rethrow_exception(cause);
                }
            }
        }

        return nullptr;
    }

// Transliterated from: kotlinx-coroutines-core/common/src/CancellableContinuationImpl.kt:467-470
void CancellableContinuationImpl<void>::dispatch_resume(int mode) {
        if (try_resume_decision()) return;
        dispatch(this, mode);
    }

// Transliterated from: kotlinx-coroutines-core/common/src/CancellableContinuationImpl.kt:160-161
internal::CoroutineStackFrame* CancellableContinuationImpl<void>::get_caller_frame() const {
    return dynamic_cast<internal::CoroutineStackFrame*>(delegate.get());
}

// Transliterated from: kotlinx-coroutines-core/common/src/CancellableContinuationImpl.kt:163-163
internal::StackTraceElement* CancellableContinuationImpl<void>::get_stack_trace_element() const {
    return nullptr;
}

// Transliterated from: kotlinx-coroutines-core/common/src/CancellableContinuationImpl.kt:398-398
void CancellableContinuationImpl<void>::invoke_on_cancellation_internal(std::shared_ptr<CancelHandler> handler) {
    invoke_on_cancellation_impl(std::move(handler));
}

// Transliterated from: kotlinx-coroutines-core/common/src/CancellableContinuationImpl.kt:365-383
void CancellableContinuationImpl<void>::resume(Unit value,
    std::function<void(std::exception_ptr, Unit, std::shared_ptr<CoroutineContext>)> on_cancellation) {
    OnCancellation adapted;
    if (on_cancellation) adapted = [on_cancellation, value](std::exception_ptr cause, std::shared_ptr<CoroutineContext> context) {
        on_cancellation(cause, value, std::move(context));
    };
    resume_impl(this->resume_mode, std::move(adapted));
}

// Transliterated from: kotlinx-coroutines-core/common/src/CancellableContinuationImpl.kt:685-688
void CompletedCancellableContinuationState<void>::invoke_handlers(
    CancellableContinuationImpl<void>& continuation, std::exception_ptr cause) {
    if (cancel_handler) continuation.call_cancel_handler(cancel_handler, cause);
    if (on_cancellation) continuation.call_on_cancellation(on_cancellation, cause);
}

} // namespace coroutines
} // namespace kotlinx