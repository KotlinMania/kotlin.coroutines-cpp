#pragma once
/**
 * Transliterated from: kotlinx-coroutines-core/common/src/CoroutineStart.kt
 */

#include "kotlinx/coroutines/CoroutineDispatcher.hpp"
#include "kotlinx/coroutines/ContinuationInterceptor.hpp"
#include "kotlinx/coroutines/Result.hpp"
#include <functional>
#include <exception>
#include <memory>
#include <type_traits>

namespace kotlinx {
namespace coroutines {

/**
 * Defines start options for coroutines builders.
 *
 * It is used in the `start` parameter of coroutine builder functions like
 * launch() and async() to describe when and how the coroutine should be
 * dispatched initially.
 *
 * This parameter only affects how the coroutine behaves until the code of its
 * body starts executing. After that, cancellability and dispatching are defined
 * by the behavior of the invoked suspending functions.
 *
 * The summary of coroutine start options is:
 * - DEFAULT immediately schedules the coroutine for execution according to its context.
 * - LAZY delays the moment of the initial dispatch until the result of the coroutine is needed.
 * - ATOMIC prevents the coroutine from being cancelled before it starts, ensuring that
 *   its code will start executing in any case.
 * - UNDISPATCHED immediately executes the coroutine until its first suspension point
 *   in the current thread.
 */
enum class CoroutineStart {
    /**
     * Immediately schedules the coroutine for execution according to its context.
     * This is usually the default option.
     *
     * DEFAULT uses the default dispatch procedure described in the CoroutineDispatcher
     * documentation.
     *
     * If the coroutine's Job is cancelled before it started executing, then it will not
     * start its execution at all and will be considered cancelled.
     */
    DEFAULT,

    /**
     * Starts the coroutine lazily, only when it is needed.
     *
     * Starting a coroutine with LAZY only creates the coroutine, but does not schedule
     * it for execution. When the completion of the coroutine is first awaited (for
     * example, via Job::join()) or explicitly started (Job::start()), the dispatch
     * procedure described in the CoroutineDispatcher documentation is performed in
     * the thread that did it.
     *
     * If the coroutine's Job is cancelled before it started executing, then it will not
     * start its execution at all and will be considered cancelled.
     *
     * Pitfall: launching a coroutine with LAZY without awaiting or cancelling it at any
     * point means that it will never be completed, leading to deadlocks and resource leaks.
     */
    LAZY,

    /**
     * Atomically (i.e., in a non-cancellable way) schedules the coroutine for execution
     * according to its context.
     *
     * This is similar to DEFAULT, but the coroutine is guaranteed to start executing even
     * if it was cancelled. This only affects the behavior until the body of the coroutine
     * starts executing; inside the body, cancellation will work as usual.
     *
     * Like ATOMIC, UNDISPATCHED too ensures that coroutines will be started in any case.
     * The difference is that, instead of immediately starting them on the same thread,
     * ATOMIC performs the full dispatch procedure just as DEFAULT does.
     *
     * This is a delicate API. The coroutine starts execution even if its Job is cancelled
     * before starting. However, the resources used within a coroutine may rely on the
     * cancellation mechanism, and cannot be used after the Job cancellation.
     */
    ATOMIC,

    /**
     * Immediately executes the coroutine until its first suspension point in the current thread.
     *
     * Starting a coroutine using UNDISPATCHED is similar to using Dispatchers::Unconfined
     * with DEFAULT, except:
     * - Resumptions from later suspensions will properly use the actual dispatcher from
     *   the coroutine's context. Only the code until the first suspension point will be
     *   executed immediately.
     * - Even if the coroutine was cancelled already, its code will still start running,
     *   similar to ATOMIC.
     * - The coroutine will not form an event loop. See Dispatchers::Unconfined for an
     *   explanation of event loops.
     *
     * This set of behaviors makes UNDISPATCHED well-suited for cases where the coroutine
     * has a distinct initialization phase whose side effects we want to rely on later.
     *
     * Pitfall: unlike Dispatchers::Unconfined and MainCoroutineDispatcher::immediate(),
     * nested undispatched coroutines do not form an event loop that otherwise prevents
     * potential stack overflow in case of unlimited nesting.
     */
    UNDISPATCHED
};

inline bool is_lazy(CoroutineStart start) {
    return start == CoroutineStart::LAZY;
}

// TODO(semantics): CoroutineStart.invoke() dispatch logic incomplete
// - DEFAULT should use startCoroutineCancellable() for proper cancellation checks
// - ATOMIC should skip cancellation check before starting
// - Current implementation is a simplified approximation
template <typename Block, typename Receiver, typename Completion>
void invoke(CoroutineStart start, Block&& block, Receiver&& receiver, Completion&& completion) {
    if (start == CoroutineStart::LAZY) {
        return;
    }

    auto run_block = [block = std::forward<Block>(block), receiver, completion]() mutable {
        using ReturnT = decltype(block(receiver));
        try {
            if constexpr (std::is_void_v<ReturnT>) {
                block(receiver);
                completion->resume_with(Result<void>::success());
            } else {
                auto value = block(receiver);
                completion->resume_with(Result<ReturnT>::success(std::move(value)));
            }
        } catch (...) {
            if constexpr (std::is_void_v<ReturnT>) {
                completion->resume_with(Result<void>::failure(std::current_exception()));
            } else {
                completion->resume_with(Result<ReturnT>::failure(std::current_exception()));
            }
        }
    };

    if (start == CoroutineStart::UNDISPATCHED) {
        // Kotlin: run immediately in current thread without dispatch.
        run_block();
        return;
    }

    // DEFAULT and ATOMIC both start immediately; ATOMIC differs in cancellation checks, which
    // we don't model yet in this C++ runtime.
    if (start == CoroutineStart::DEFAULT || start == CoroutineStart::ATOMIC) {
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
        
        auto runnable = std::make_shared<LambdaRunnable>([run = std::move(run_block)]() mutable { run(); });
        
        if (dispatcher) {
             dispatcher->dispatch(*context, runnable);
        } else {
             runnable->run();
        }
    }
}

} // namespace coroutines
} // namespace kotlinx
