// port-lint: source kotlinx-coroutines-core/common/src/CoroutineStart.kt
/** Transliterated from: kotlinx-coroutines-core/common/src/CoroutineStart.kt */
#include "kotlinx/coroutines/CoroutineStart.hpp"

namespace kotlinx::coroutines {
// Transliterated from: kotlinx-coroutines-core/common/src/CoroutineStart.kt:356-362
void invoke(CoroutineStart start, intrinsics::ErasedSuspendFunction block,
            std::shared_ptr<Continuation<void*>> completion) {
    switch (start) {
        case CoroutineStart::DEFAULT:
            intrinsics::start_coroutine_cancellable(std::move(block), std::move(completion));
            break;
        case CoroutineStart::ATOMIC:
            intrinsics::start_coroutine(std::move(block), std::move(completion));
            break;
        case CoroutineStart::UNDISPATCHED:
            intrinsics::start_coroutine_undispatched(std::move(block), std::move(completion));
            break;
        case CoroutineStart::LAZY:
            break; // Will start lazily.
    }
}
// Transliterated from: kotlinx-coroutines-core/common/src/CoroutineStart.kt:370-370
bool is_lazy(CoroutineStart start) { return start == CoroutineStart::LAZY; }
} // namespace kotlinx::coroutines
