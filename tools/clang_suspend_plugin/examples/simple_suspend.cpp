#include <kotlinx/coroutines/ContinuationImpl.hpp>
#include <kotlinx/coroutines/intrinsics/Intrinsics.hpp>
#include <memory>

using namespace kotlinx::coroutines;

// Dummy suspend callee (decl only for syntax/IR experiments).
void* foo_suspend(std::shared_ptr<Continuation<void*>> completion);

[[kotlinx::suspend]]
void* demo(std::shared_ptr<Continuation<void*>> completion) {
    int x = 1;

    // Mark a suspension point. Phase‑1 expects either this annotate or a call to kx::suspend_call(...).
    [[clang::annotate("kotlinx_suspend_call")]]
    foo_suspend(completion);

    x += 1;
    return nullptr;
}

