// port-lint: source kotlinx-coroutines-core/common/src/intrinsics/Undispatched.kt
/** Transliterated from: kotlinx-coroutines-core/common/src/intrinsics/Undispatched.kt */
#include "kotlinx/coroutines/intrinsics/Cancellable.hpp"
#include "kotlinx/coroutines/common/CoroutineContextUtils.hpp"

namespace kotlinx::coroutines::intrinsics {
// Transliterated from: kotlinx-coroutines-core/common/src/intrinsics/Undispatched.kt:13-31
void start_coroutine_undispatched(ErasedSuspendFunction block, std::shared_ptr<Continuation<void*>> completion) {
    auto actual_completion = kotlin::coroutines::native::internal::probe_coroutine_created(std::move(completion));
    void* value;
    try {
        // Start immediately in the current stack frame, until the first suspension.
        value = with_coroutine_context<void*>(actual_completion->get_context(), nullptr, [&] {
            kotlin::coroutines::native::internal::probe_coroutine_resumed(actual_completion.get());
            return start_coroutine_unintercepted_or_return(std::move(block), actual_completion);
        });
    } catch (...) {
        auto report_exception = std::current_exception();
        try { std::rethrow_exception(report_exception); }
        catch (const internal::DispatchException& failure) { report_exception = failure.cause; }
        catch (...) {}
        actual_completion->resume_with(Result<void*>::failure(report_exception));
        return;
    }
    if (!is_coroutine_suspended(value)) actual_completion->resume_with(Result<void*>::success(value));
}
} // namespace kotlinx::coroutines::intrinsics
