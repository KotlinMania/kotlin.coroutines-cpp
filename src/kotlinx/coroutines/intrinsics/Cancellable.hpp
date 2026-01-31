#pragma once

/**
 * Transliterated from: kotlinx-coroutines-core/common/src/intrinsics/Cancellable.kt
 */

#include "kotlinx/coroutines/Continuation.hpp"
#include "kotlinx/coroutines/ContinuationImpl.hpp"
#include "kotlinx/coroutines/internal/DispatchedContinuation.hpp"
#include <functional>
#include <memory>
#include <concepts>

namespace kotlinx::coroutines::intrinsics {

// Helper: safe cast to ContinuationImpl
template<typename T>
std::shared_ptr<ContinuationImpl> as_continuation_impl(Continuation<T>* cont) {
    return std::dynamic_pointer_cast<ContinuationImpl>(cont->shared_from_this());
}

template<typename T>
class LambdaContinuation : public ContinuationImpl {
    std::function<void*(Continuation<T>*)> block_;
    std::shared_ptr<Continuation<T>> completion_;

public:
    LambdaContinuation(
        std::function<void*(Continuation<T>*)> block,
        std::shared_ptr<Continuation<T>> completion
    ) : ContinuationImpl(std::make_shared<Continuation<void>>(), completion->get_context()),
        block_(block), completion_(completion) {
        (void)completion; // Supress lint if needed, though used in init list
    }

    void* invoke_suspend(Result<void*> result) override {
        if (result.is_failure()) {
            completion_->resume_with(Result<T>::failure(result.exception_or_null()));
            return nullptr;
        }
        return block_(completion_.get());
    }
    
    // Stub definition for context necessary for ContinuationImpl base
    std::shared_ptr<CoroutineContext> get_context() const override {
        return completion_->get_context();
    }
};

template<typename R, typename T>
class ReceiverLambdaContinuation : public ContinuationImpl {
    std::function<void*(R, Continuation<T>*)> block_;
    R receiver_;
    std::shared_ptr<Continuation<T>> completion_;

public:
    ReceiverLambdaContinuation(
        std::function<void*(R, Continuation<T>*)> block,
        R receiver,
        std::shared_ptr<Continuation<T>> completion
    ) : ContinuationImpl(std::make_shared<Continuation<void>>(), completion->get_context()),
        block_(block), receiver_(receiver), completion_(completion) {
        (void)completion; 
    }

    void* invoke_suspend(Result<void*> result) override {
        if (result.is_failure()) {
            completion_->resume_with(Result<T>::failure(result.exception_or_null()));
            return nullptr;
        }
        return block_(receiver_, completion_.get());
    }

    std::shared_ptr<CoroutineContext> get_context() const override {
        return completion_->get_context();
    }
};

// Simplified create_coroutine_unintercepted
template <typename T>
std::shared_ptr<Continuation<void*>> create_coroutine_unintercepted(
    std::function<void*(Continuation<T>*)> block,
    std::shared_ptr<Continuation<T>> completion
) {
    return std::make_shared<LambdaContinuation<T>>(block, completion);
}

template <typename R, typename T>
std::shared_ptr<Continuation<void*>> create_coroutine_unintercepted(
    std::function<void*(R, Continuation<T>*)> block,
    R receiver,
    std::shared_ptr<Continuation<T>> completion
) {
    return std::make_shared<ReceiverLambdaContinuation<R, T>>(block, receiver, completion);
}

// Internal helper for dispatcherFailure
inline void dispatcher_failure(Continuation<void*>* completion, std::exception_ptr e) {
    auto report_exception = e;
    // if (e is DispatchException) e.cause else e -- simplified for now
    
    // We can't easily access resumeWith on raw pointer without knowing type T. 
    // This signature uses void*, implying Unit.
    completion->resume_with(Result<void*>::failure(report_exception));
    std::rethrow_exception(report_exception);
}

/**
 * Runs given block and completes completion with its exception if it occurs.
 * Rationale: [startCoroutineCancellable] is invoked when we are about to run coroutine asynchronously in its own dispatcher.
 * Thus if dispatcher throws an exception during coroutine start, coroutine never completes, so we should treat dispatcher exception
 * as its cause and resume completion.
 */
inline void run_safely(Continuation<void*>* completion, std::function<void()> block) {
    try {
        block();
    } catch (...) {
        dispatcher_failure(completion, std::current_exception());
    }
}

// Specialization for typed continuation (not void*) is tricky without template. 
// We implement the template logic inside start_coroutine_cancellable.

/**
 * Use this function to start coroutine in a cancellable way, so that it can be cancelled
 * while waiting to be dispatched.
 *
 * @suppress **This is internal API and is subject to change.**
 */
template <typename T>
void start_coroutine_cancellable(std::function<void*(Continuation<T>*)> block, Continuation<T>* completion) {
    // We need shared_ptr for completion to pass to create_coroutine
    // Assuming completion is managed or we can share_from_this? 
    // If raw pointer, we might be in trouble unless it's enable_shared_from_this.
    // Most Continuations should be.
    
    auto shared_completion = std::dynamic_pointer_cast<Continuation<T>>(completion->shared_from_this());
    if (!shared_completion) {
         // Fallback if not shared (shouldn't happen in our model)
         return; 
    }

    try {
        auto coroutine = create_coroutine_unintercepted(block, shared_completion);
        auto intercepted = std::dynamic_pointer_cast<ContinuationImpl>(coroutine)->intercepted();
        resume_cancellable_with(intercepted, Result<void*>::success(nullptr));
    } catch (...) {
         // logic from runSafely/dispatcherFailure adapted for T
         auto ex = std::current_exception();
         shared_completion->resume_with(Result<T>::failure(ex));
         throw;
    }
}

/**
 * Use this function to start coroutine in a cancellable way, so that it can be cancelled
 * while waiting to be dispatched.
 */
template <typename R, typename T>
void start_coroutine_cancellable(std::function<void*(R, Continuation<T>*)> block, R receiver, Continuation<T>* completion) {
    auto shared_completion = std::dynamic_pointer_cast<Continuation<T>>(completion->shared_from_this());
    if (!shared_completion) return;

    try {
        auto coroutine = create_coroutine_unintercepted(block, receiver, shared_completion);
        auto intercepted = std::dynamic_pointer_cast<ContinuationImpl>(coroutine)->intercepted();
        resume_cancellable_with(intercepted, Result<void*>::success(nullptr));
    } catch (...) {
         auto ex = std::current_exception();
         shared_completion->resume_with(Result<T>::failure(ex));
         throw;
    }
}

/**
 * Similar to [startCoroutineCancellable], but for already created coroutine.
 * [fatalCompletion] is used only when interception machinery throws an exception
 */
inline void start_coroutine_cancellable(Continuation<void*>* continuation, Continuation<void*>* fatal_completion) {
    auto shared_fatal = std::dynamic_pointer_cast<Continuation<void*>>(fatal_completion->shared_from_this());
    
    run_safely(fatal_completion, [continuation]() {
        auto impl = std::dynamic_pointer_cast<ContinuationImpl>(continuation->shared_from_this());
        if (impl) {
            auto intercepted = impl->intercepted();
            resume_cancellable_with(intercepted, Result<void*>::success(nullptr));
        }
    });
}

} // namespace kotlinx::coroutines::intrinsics
