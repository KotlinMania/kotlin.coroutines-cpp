/**
 * Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/coroutines/ContinuationImpl.kt
 * Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/coroutines/intrinsics/IntrinsicsNative.kt
 */
#include "kotlinx/coroutines/ContinuationImpl.hpp"
#include "kotlinx/coroutines/ContinuationInterceptor.hpp"
#include "kotlinx/coroutines/internal/DispatchedContinuation.hpp"

namespace kotlin::coroutines::native::internal {
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/coroutines/DebugProbes.kt:48-60
// This probe is invoked when a coroutine is resumed using Continuation.resume_with.
// The coroutine machinery guarantees that frame extends BaseContinuationImpl.
// NOTE(port): This overload specializes the Native Continuation<*> parameter for the erased ABI.
void probe_coroutine_resumed(kotlinx::coroutines::Continuation<void*>* frame) {}
} // namespace kotlin::coroutines::native::internal

namespace kotlinx::coroutines {

// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/coroutines/ContinuationImpl.kt:21-45
void BaseContinuationImpl::resume_with(Result<void*> result) {

    // Invoke the resume debug probe only once, even if previous frames are resumed in the loop, too.
    kotlin::coroutines::native::internal::probe_coroutine_resumed(this);
    // This loop unrolls recursion in current.resumeWith(param) to make saner and shorter stack traces on resume
    auto current = this;
    // NOTE(port): Kotlin's local current is a GC reference. Keep the same
    // completion owner across iterations after the finished child releases it.
    auto current_owner = weak_from_this().lock();
    Result<void*> param = std::move(result);

    while (true) {
        // NOTE(port): Kotlin's GC keeps both frames alive while the loop
        // releases interception and transfers the completed Result.
        auto receiver_owner = current->weak_from_this().lock();
        auto completion_owner = current->completion;
        // with(current)
        auto* completion_ptr = completion_owner.get();
        if (!completion_ptr) {
            // fail fast when trying to resume continuation without completion
            // NOTE(port): Kotlin's completion!! maps to std::logic_error.
            throw std::logic_error("Trying to resume continuation without completion");
        }

        Result<void*> outcome;
        try {
            // val outcome = invokeSuspend(param)
            void* suspend_result = current->invoke_suspend(param);

            if (intrinsics::is_coroutine_suspended(suspend_result)) {
                return;
            }

            outcome = Result<void*>::success(suspend_result);
        } catch (...) {
            outcome = Result<void*>::failure(std::current_exception());
        }

        current->release_intercepted(); // this state machine instance is terminating

        auto* base_completion = dynamic_cast<BaseContinuationImpl*>(completion_ptr);
        if (base_completion) {
            // unrolling recursion via loop
            current_owner = std::dynamic_pointer_cast<BaseContinuationImpl>(completion_owner);
            current = base_completion;
            param = std::move(outcome);
        } else {
            // top-level completion reached -- invoke and return
            completion_ptr->resume_with(std::move(outcome));
            return;
        }
    }
}

// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/coroutines/ContinuationImpl.kt:55-57
// NOTE(port): UnsupportedOperationException currently maps to std::runtime_error.
std::shared_ptr<Continuation<void*>> BaseContinuationImpl::create(std::shared_ptr<Continuation<void*>> completion) {
    throw std::runtime_error("create(Continuation) has not been overridden");
}
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/coroutines/ContinuationImpl.kt:59-61
// NOTE(port): UnsupportedOperationException currently maps to std::runtime_error.
std::shared_ptr<Continuation<void*>> BaseContinuationImpl::create(void* value, std::shared_ptr<Continuation<void*>> completion) {
    throw std::runtime_error("create(Any?;Continuation) has not been overridden");
}

// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/coroutines/ContinuationImpl.kt:95-98
ContinuationImpl::ContinuationImpl(std::shared_ptr<Continuation<void*>> completion,
                                   std::shared_ptr<CoroutineContext> context)
    : BaseContinuationImpl(std::move(completion)), context_(std::move(context)) {}

// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/coroutines/ContinuationImpl.kt:99-99
ContinuationImpl::ContinuationImpl(std::shared_ptr<Continuation<void*>> completion)
    : ContinuationImpl(completion, completion ? completion->get_context() : nullptr) {}

// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/coroutines/ContinuationImpl.kt:101-102
// NOTE(port): Kotlin's _context!! maps to std::logic_error.
std::shared_ptr<CoroutineContext> ContinuationImpl::get_context() const {
    if (!context_) throw std::logic_error("Continuation context is null");
    return context_;
}

// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/coroutines/ContinuationImpl.kt:75-85
// NOTE(port): Kotlin IllegalArgumentException maps to std::invalid_argument.
RestrictedContinuationImpl::RestrictedContinuationImpl(std::shared_ptr<Continuation<void*>> completion)
    : BaseContinuationImpl(std::move(completion)) {
    if (this->completion) {
        if (this->completion->get_context() != EmptyCoroutineContext::instance())
            throw std::invalid_argument("Coroutines with restricted suspension must have EmptyCoroutineContext");
    }
}

// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/coroutines/ContinuationImpl.kt:87-88
std::shared_ptr<CoroutineContext> RestrictedContinuationImpl::get_context() const {
    return EmptyCoroutineContext::instance();
}

// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/coroutines/ContinuationImpl.kt:104-107
std::shared_ptr<Continuation<void*>> ContinuationImpl::intercepted() {
    if (!intercepted_) {
        auto self = std::static_pointer_cast<Continuation<void*>>(shared_from_this());
        auto interceptor = std::dynamic_pointer_cast<ContinuationInterceptor>(
            get_context()->get(ContinuationInterceptor::type_key));
        intercepted_ = interceptor ? interceptor->intercept_continuation(self) : self;
        if (!intercepted_) intercepted_ = self;
    }
    return intercepted_;
}

// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/coroutines/ContinuationImpl.kt:109-115
void ContinuationImpl::release_intercepted() {
    auto intercepted = intercepted_;
    // NOTE(port): A direct ABI entry and resume_with can both terminate the same
    // frame. The completed sentinel makes releasing the interceptor idempotent.
    if (intercepted && intercepted.get() != this &&
        intercepted != CompletedContinuation::instance()) {
        auto interceptor = std::dynamic_pointer_cast<ContinuationInterceptor>(
            get_context()->get(ContinuationInterceptor::type_key));
        if (!interceptor) throw std::logic_error("Missing continuation interceptor");
        interceptor->release_intercepted_continuation(intercepted);
    }
    intercepted_ = CompletedContinuation::instance();
}

// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/coroutines/ContinuationImpl.kt:32-41
// NOTE(port): Initial ABI invocation returns directly rather than resuming completion.
void* BaseContinuationImpl::start(Result<void*> result) {
    try {
        auto outcome = invoke_suspend(std::move(result));
        if (!intrinsics::is_coroutine_suspended(outcome)) release_intercepted();
        return outcome;
    } catch (...) {
        release_intercepted();
        throw;
    }
}

namespace intrinsics {
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/coroutines/intrinsics/IntrinsicsNative.kt:201-202
std::shared_ptr<Continuation<void*>> intercepted(std::shared_ptr<Continuation<void*>> continuation) {
    auto frame = std::dynamic_pointer_cast<ContinuationImpl>(continuation);
    return frame ? frame->intercepted() : continuation;
}
} // namespace intrinsics

namespace internal {
// NOTE(port): Kotlin GC owns a continuation passed through the raw entry ABI.
std::shared_ptr<Continuation<void*>> retain_continuation(Continuation<void*>* continuation) {
    if (auto* frame = dynamic_cast<BaseContinuationImpl*>(continuation)) {
        if (auto owner = frame->weak_from_this().lock()) return owner;
    }
    return std::shared_ptr<Continuation<void*>>(continuation, [](Continuation<void*>*) {});
}

// Transliterated from: kotlinx-coroutines-core/common/src/CancellableContinuation.kt:423-435
// NOTE(port): Typed adapters preserve on-cancellation values until dispatch has
// checked the Job, then box the value once for the lowered continuation ABI.
InterceptedDelegate intercepted_delegate(std::shared_ptr<Continuation<void*>> continuation) {
    auto intercepted = intrinsics::intercepted(std::move(continuation));
    if (auto dispatched = std::dynamic_pointer_cast<DispatchedContinuation<void*>>(intercepted)) {
        return {dispatched->continuation, dispatched->dispatcher};
    }
    return {std::move(intercepted), nullptr};
}
} // namespace internal
} // namespace kotlinx::coroutines
