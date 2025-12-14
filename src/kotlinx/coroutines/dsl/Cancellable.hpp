#pragma once

#include "kotlinx/coroutines/CancellableContinuation.hpp"
#include "kotlinx/coroutines/CancellableContinuationImpl.hpp"
#include <memory>
#include <functional>

namespace kotlinx {
namespace coroutines {
namespace dsl {

    /**
     * suspend_cancellable_coroutine - DSL wrapper for cancellable suspension.
     *
     * Usage within coroutine_yield:
     *   coroutine_yield(this, suspend_cancellable_coroutine<T>(block, completion));
     *
     * @param block Callable as void(CancellableContinuation<T>&)
     * @param continuation The completion continuation
     * @return COROUTINE_SUSPENDED or result
     */
    template <typename T, typename Block>
    void* suspend_cancellable_coroutine(Block&& block, std::shared_ptr<Continuation<void*>> continuation) {
        return kotlinx::coroutines::suspend_cancellable_coroutine<T>(
            std::forward<Block>(block),
            continuation.get()
        );
    }

} // namespace dsl
} // namespace coroutines
} // namespace kotlinx
