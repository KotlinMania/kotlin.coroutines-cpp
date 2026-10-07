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

// Transliterated from: kotlinx-coroutines-core/common/src/intrinsics/Cancellable.kt:33-36
void start_coroutine_cancellable(Continuation<void*>* continuation, Continuation<void*>* fatal_completion) {
    auto owner = internal::retain_continuation(continuation);
    run_safely(fatal_completion, [owner = std::move(owner)] {
        try {
            auto intercepted_continuation = intercepted(owner);
            resume_cancellable_with(intercepted_continuation, Result<void*>::success(nullptr));
        } catch (...) {
            // NOTE(port): Native GC collects a failed start's interception cycle.
            // Release that actual frame's cached interception before reporting
            // the fatal failure; borrowed continuations are never adopted.
            if (auto frame = std::dynamic_pointer_cast<BaseContinuationImpl>(owner)) {
                frame->release_intercepted();
            }
            throw;
        }
    });
}

} // namespace kotlinx::coroutines::intrinsics
