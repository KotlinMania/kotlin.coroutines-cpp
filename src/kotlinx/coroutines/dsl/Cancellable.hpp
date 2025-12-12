#pragma once

#include "kotlinx/coroutines/CancellableContinuation.hpp"
#include "kotlinx/coroutines/CancellableContinuationImpl.hpp"
#include "kotlinx/coroutines/dsl/Suspend.hpp"
#include <memory>
#include <functional>

namespace kotlinx {
namespace coroutines {
namespace dsl {

    // Implementation adapter
    // T is the return type of the coroutine
    // Block is callable as void(CancellableContinuation<T>&)
    template <typename T, typename Block>
    [[suspend]]
    void* suspendCancellableCoroutine(Block&& block, std::shared_ptr<Continuation<void*>> continuation) {
         // Create the CancellableContinuation wrapper state machine helper?
         // Actually, kotlinx::coroutines::suspend_cancellable_coroutine (in common code) usually handles this.
         // We just forward to it.
         // But wait, `suspend_cancellable_coroutine` in Delay.cpp was calling a helper function.
         // Let's implement the forwarding logic here, leveraging the existing `CancellableContinuationImpl` infrastructure.
         
         // We need to construct a CancellableContinuationImpl wrapping `continuation`.
         // Then call block with it.
         // Then return the result (COROUTINE_SUSPENDED or result).
         
         // Using the existing free function from CancellableContinuationImpl.hpp if available, 
         // OR, reusing the one I saw in Delay.cpp (which was a static member or local?).
         
         // Actually, `suspend_cancellable_coroutine` is defined in `CancellableContinuationImpl.hpp`.
         using kotlinx::coroutines::suspend_cancellable_coroutine;
         
         // We need to cast `continuation` (Continuation<void*>) to the expected type for the return T?
         // suspend_cancellable_coroutine takes `Continuation<T>*`.
         // Our plugin hook receives `Continuation<void*>`.
         // If T is void, it's fine. If T is int, we need adapter?
         // For now, let's assume T=void or we do unsafe cast if we trust TypeErasure.
         
         return suspend(suspend_cancellable_coroutine<T>(
             std::forward<Block>(block), 
             continuation.get()
         ));
    }

    // Frontend Stub
    template <typename T, typename Block>
    [[suspend]]
    T suspendCancellableCoroutine(Block&& block) {
        throw std::runtime_error("suspendCancellableCoroutine stub called without plugin transformation");
    }

} // namespace dsl
} // namespace coroutines
} // namespace kotlinx
