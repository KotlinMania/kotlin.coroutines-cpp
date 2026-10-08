// NOTE(port): Ownership and typed-result bindings for the existing C++ continuation ABI.
#include "kotlinx/coroutines/ContinuationBindings.hpp"
#include "kotlinx/coroutines/internal/DispatchedContinuation.hpp"
namespace kotlinx::coroutines {
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
