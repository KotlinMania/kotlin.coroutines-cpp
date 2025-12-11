#include <kotlinx/coroutines/ContinuationImpl.hpp>
#include <kotlinx/coroutines/intrinsics/Intrinsics.hpp>
#include <kotlinx/coroutines/dsl/Suspend.hpp>
#include <memory>

using namespace kotlinx::coroutines;
using namespace kotlinx::coroutines::dsl;

// Dummy suspend callee (decl only for syntax/IR experiments).
void* foo_suspend(std::shared_ptr<Continuation<void*>> completion);

[[suspend]]
void* demo(std::shared_ptr<Continuation<void*>> completion) {
    int x = 1;

    // Preferred Kotlin-like spelling: suspend(...)
    suspend(foo_suspend(completion));

    x += 1;
    return nullptr;
}
