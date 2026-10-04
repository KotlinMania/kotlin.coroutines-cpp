/**
 * Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/coroutines/ContinuationImpl.kt
 * Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/coroutines/intrinsics/IntrinsicsNative.kt
 */
#include "kotlinx/coroutines/ContinuationImpl.hpp"
#include "kotlinx/coroutines/ContinuationInterceptor.hpp"
#include "kotlinx/coroutines/internal/DispatchedContinuation.hpp"

namespace kotlinx::coroutines {

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
