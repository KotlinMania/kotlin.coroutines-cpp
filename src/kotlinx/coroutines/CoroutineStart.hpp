#pragma once

#include "kotlinx/coroutines/CoroutineDispatcher.hpp"
#include "kotlinx/coroutines/ContinuationInterceptor.hpp"
#include <functional>
#include <memory>

namespace kotlinx {
namespace coroutines {

/**
 * Defines start options for coroutines builders.
 */
enum class CoroutineStart {
    DEFAULT,
    LAZY,
    ATOMIC,
    UNDISPATCHED
};

inline bool is_lazy(CoroutineStart start) {
    return start == CoroutineStart::LAZY;
}

/*
 * TODO: STUB - CoroutineStart.invoke() dispatch logic not implemented
 *
 * Kotlin source: CoroutineStart.invoke() in CoroutineStart.kt
 *
 * What's missing:
 * - DEFAULT: Should dispatch via block.startCoroutineCancellable(receiver, completion)
 * - LAZY: Should not start immediately, just create the coroutine
 * - ATOMIC: Should start atomically without checking cancellation first
 * - UNDISPATCHED: Should run immediately in current thread without dispatching
 *
 * Current behavior: Does nothing - coroutines don't actually start via this path
 * Correct behavior: Dispatch or execute the coroutine block based on start mode
 *
 * Dependencies:
 * - startCoroutineCancellable() for DEFAULT mode
 * - Dispatcher integration for proper thread handling
 * - CancellableContinuation for cancellation checks
 *
 * Note: AbstractCoroutine.start() has its own implementation that works around this.
 */
template <typename Block, typename Receiver, typename Completion>
void invoke(CoroutineStart start, Block&& block, Receiver&& receiver, Completion&& completion) {
    if (start == CoroutineStart::DEFAULT) {
        auto context = completion->get_context();
        
        std::shared_ptr<CoroutineDispatcher> dispatcher;
        auto element = context->get(ContinuationInterceptor::type_key);
        if (element) {
             dispatcher = std::dynamic_pointer_cast<CoroutineDispatcher>(element);
        }
        
        struct LambdaRunnable : Runnable {
             std::function<void()> b;
             LambdaRunnable(std::function<void()> f) : b(std::move(f)) {}
             void run() override { b(); }
        };
        
        // Capture block and receiver by value (assuming they are cheap/copyable functions/pointers)
        auto runnable = std::make_shared<LambdaRunnable>([b = block, r = receiver]() {
             b(r);
        });
        
        if (dispatcher) {
             dispatcher->dispatch(*context, runnable);
        } else {
             runnable->run();
        }
    }
}

} // namespace coroutines
} // namespace kotlinx
