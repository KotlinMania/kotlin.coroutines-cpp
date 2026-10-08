// port-lint: source kotlinx-coroutines-core/common/src/intrinsics/Cancellable.kt
/**
 * Transliterated from: kotlinx-coroutines-core/common/src/intrinsics/Cancellable.kt
 */
#include "kotlinx/coroutines/intrinsics/Cancellable.hpp"

namespace kotlinx::coroutines::intrinsics {

// Transliterated from: kotlinx-coroutines-core/common/src/intrinsics/Cancellable.kt:52-64
void dispatcher_failure(Continuation<void*>* completion, std::exception_ptr exception) {
    // Resume the coroutine with the reported exception so it cannot prevent its
    // parent from completion, then immediately rethrow to the caller.
    auto report_exception = exception;
    try {
        std::rethrow_exception(exception);
    } catch (const internal::DispatchException& failure) {
        report_exception = failure.cause;
    } catch (...) {
    }
    completion->resume_with(Result<void*>::failure(report_exception));
    std::rethrow_exception(report_exception);
}

// Transliterated from: kotlinx-coroutines-core/common/src/intrinsics/Cancellable.kt:44-50
void run_safely(Continuation<void*>* completion, std::function<void()> block) {
    try {
        block();
    } catch (...) {
        dispatcher_failure(completion, std::current_exception());
    }
}

namespace {
// Transliterated from: kotlinx-coroutines-core/common/src/intrinsics/Cancellable.kt:16-16,26-26,35-35
void resume_cancellable_start(std::shared_ptr<Continuation<void*>> owner) {
    try {
        resume_cancellable_with(intercepted(owner), Result<void*>::success(nullptr));
    } catch (...) {
        // NOTE(port): Release the actual failed-start interception cycle that Native GC collects.
        if (auto frame = std::dynamic_pointer_cast<BaseContinuationImpl>(owner)) frame->release_intercepted();
        throw;
    }
}
}

// Transliterated from: kotlinx-coroutines-core/common/src/intrinsics/Cancellable.kt:33-36
void start_coroutine_cancellable(Continuation<void*>* continuation, Continuation<void*>* fatal_completion) {
    auto owner = internal::retain_continuation(continuation);
    run_safely(fatal_completion, [owner = std::move(owner)] { resume_cancellable_start(owner); });
}
// Transliterated from: kotlinx-coroutines-core/common/src/intrinsics/Cancellable.kt:15-17
void start_coroutine_cancellable(ErasedSuspendFunction block, std::shared_ptr<Continuation<void*>> completion) {
    run_safely(completion.get(), [&] {
        resume_cancellable_start(create_coroutine_unintercepted(std::move(block), completion));
    });
}
// Transliterated from: libraries/stdlib/src/kotlin/coroutines/Continuation.kt:112-116
void start_coroutine(ErasedSuspendFunction block, std::shared_ptr<Continuation<void*>> completion) {
    auto coroutine = create_coroutine_unintercepted(std::move(block), std::move(completion));
    try {
        intercepted(coroutine)->resume_with(Result<void*>::success(nullptr));
    } catch (...) {
        // NOTE(port): Cleanup an actual frame's interception cycle without delivering
        // an extra completion failure; stdlib startCoroutine only propagates here.
        if (auto frame = std::dynamic_pointer_cast<BaseContinuationImpl>(coroutine)) frame->release_intercepted();
        throw;
    }
}
} // namespace kotlinx::coroutines::intrinsics
