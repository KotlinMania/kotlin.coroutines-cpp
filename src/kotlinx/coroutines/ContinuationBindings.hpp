#pragma once
// NOTE(port): C++ owning/borrowed continuation ABI bindings, separate from the
// Native continuation classes and the kotlin.coroutines.intrinsics functions.
#include "kotlinx/coroutines/ContinuationImpl.hpp"
namespace kotlinx::coroutines {
class CoroutineDispatcher;
namespace internal {
// NOTE(port): Keep typed cancellable results until dispatch cancellation checks
// have run, then box them at the Continuation<void*> ABI boundary.
struct InterceptedDelegate {
    std::shared_ptr<Continuation<void*>> continuation;
    std::shared_ptr<CoroutineDispatcher> dispatcher;
};
std::shared_ptr<Continuation<void*>> retain_continuation(Continuation<void*>* continuation);
InterceptedDelegate intercepted_delegate(std::shared_ptr<Continuation<void*>> continuation);
} // namespace internal

} // namespace kotlinx::coroutines
namespace kotlin::coroutines::intrinsics {
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/coroutines/intrinsics/IntrinsicsNative.kt:201-202
std::shared_ptr<Continuation<void*>> intercepted(std::shared_ptr<Continuation<void*>> continuation);
}
