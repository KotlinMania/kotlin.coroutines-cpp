#pragma once
/**
 * @file CoroutineStart.hpp
 * @brief Start options for coroutine builders
 *
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
 * [launch][CoroutineScope::launch] and [async][CoroutineScope::async]
 * to describe when and how the coroutine should be dispatched initially.
 *
 * This parameter only affects how the coroutine behaves until the code of its body starts executing.
 * After that, cancellability and dispatching are defined by the behavior of the invoked suspending functions.
 *
 * The summary of coroutine start options is:
 * - [DEFAULT] immediately schedules the coroutine for execution according to its context.
 * - [LAZY] delays the moment of the initial dispatch until the result of the coroutine is needed.
 * - [ATOMIC] prevents the coroutine from being cancelled before it starts, ensuring that its code will start
 *   executing in any case.
 * - [UNDISPATCHED] immediately executes the coroutine until its first suspension point _in the current thread_.
 *
 * Transliterated from:
 * public enum class CoroutineStart
 */
enum class CoroutineStart {
    /**
     * Immediately schedules the coroutine for execution according to its context. This is usually the default option.
     *
     * [DEFAULT] uses the default dispatch procedure described in the [CoroutineDispatcher] documentation.
     *
     * If the coroutine's [Job] is cancelled before it started executing, then it will not start its
     * execution at all and will be considered [cancelled][Job::is_cancelled].
     *
     * Examples:
     *
     * ```cpp
     * // Example of starting a new coroutine that goes through a dispatch
     * run_blocking([](CoroutineScope& scope) {
     *     std::cout << "1. About to start a new coroutine.\n";
     *     // Dispatch the job to execute later.
     *     // The parent coroutine's dispatcher is inherited by default.
     *     launch(scope, []() { // CoroutineStart::DEFAULT is launch's default start mode
     *         std::cout << "3. When the thread is available, we start the coroutine\n";
     *     });
     *     std::cout << "2. The thread keeps doing other work after launching the coroutine\n";
     * });
     * ```
     *
     * ```cpp
     * // Example of cancelling coroutines before they start executing.
     * run_blocking([](CoroutineScope& scope) {
     *     // dispatch the job to execute on this thread later
     *     launch(scope, []() { // CoroutineStart::DEFAULT is the launch's default start mode
     *         std::cout << "This code will never execute\n";
     *     });
     *     scope.cancel(); // cancels the current coroutine scope and its children
     *     std::cout << "This code will execute.\n";
     * });
     * ```
     *
     * Transliterated from:
     * DEFAULT
     */
    DEFAULT,

    /**
     * Starts the coroutine lazily, only when it is needed.
     *
     * Starting a coroutine with [LAZY] only creates the coroutine, but does not schedule it for execution.
     * When the completion of the coroutine is first awaited
     * (for example, via [Job::join]) or explicitly [started][Job::start],
     * the dispatch procedure described in the [CoroutineDispatcher] documentation is performed in the thread
     * that did it.
     *
     * The details of what counts as waiting can be found in the documentation of the corresponding coroutine builders
     * like [launch][CoroutineScope::launch] and [async][CoroutineScope::async].
     *
     * If the coroutine's [Job] is cancelled before it started executing, then it will not start its
     * execution at all and will be considered [cancelled][Job::is_cancelled].
     *
     * **Pitfall**: launching a coroutine with [LAZY] without awaiting or cancelling it at any point means that it will
     * never be completed, leading to deadlocks and resource leaks.
     * For example, the following code will deadlock, since [coroutine_scope] waits for all of its child coroutines to
     * complete:
     * ```cpp
     * // This code hangs!
     * coroutine_scope([](CoroutineScope& scope) {
     *     launch(scope, CoroutineStart::LAZY, []() { });
     * }, cont);
     * ```
     *
     * ## Alternatives
     *
     * The effects of [LAZY] can usually be achieved more idiomatically without it.
     *
     * If a coroutine is started with [LAZY] and then unconditionally started,
     * it is more idiomatic to create the coroutine in the exact place where it is started:
     *
     * ```cpp
     * // instead of `auto job = launch(scope, CoroutineStart::LAZY, []{}); job->start();`, do
     * launch(scope, []() { });
     * ```
     *
     * Transliterated from:
     * LAZY
     */
    LAZY,

    /**
     * Atomically (i.e., in a non-cancellable way) schedules the coroutine for execution according to its context.
     *
     * This is similar to [DEFAULT], but the coroutine is guaranteed to start executing even if it was cancelled.
     * This only affects the behavior until the body of the coroutine starts executing;
     * inside the body, cancellation will work as usual.
     *
     * Like [ATOMIC], [UNDISPATCHED], too, ensures that coroutines will be started in any case.
     * The difference is that, instead of immediately starting them on the same thread,
     * [ATOMIC] performs the full dispatch procedure just as [DEFAULT] does.
     *
     * Because of this, we can use [ATOMIC] in cases where we want to be certain that some code eventually runs
     * and uses a specific dispatcher to do that.
     *
     * Example:
     * ```cpp
     * Mutex mutex;
     *
     * mutex.lock(); // lock the mutex outside the coroutine
     * // ... // initial portion of the work, protected by the mutex
     * auto job = launch(scope, CoroutineStart::ATOMIC, [&mutex]() {
     *     // the work must continue in a coroutine, but still under the mutex
     *     std::cout << "Coroutine running!\n";
     *     try {
     *         // this `try` block will be entered in any case because of ATOMIC
     *         std::cout << "Starting task...\n";
     *         delay(10, cont); // throws due to cancellation
     *         std::cout << "Finished task.\n";
     *     } catch (...) {
     *         mutex.unlock(); // correctly release the mutex
     *         throw;
     *     }
     *     mutex.unlock();
     * });
     *
     * job->cancel_and_join(cont); // we immediately cancel the coroutine.
     * mutex.with_lock([]() {
     *     std::cout << "The lock has been returned correctly!\n";
     * });
     * ```
     *
     * Here, we used [ATOMIC] to ensure that a mutex that was acquired outside the coroutine does get released
     * even if cancellation happens between `lock()` and `launch`.
     * As a result, the mutex will always be released.
     *
     * This is a **delicate** API. The coroutine starts execution even if its [Job] is cancelled before starting.
     * However, the resources used within a coroutine may rely on the cancellation mechanism,
     * and cannot be used after the [Job] cancellation.
     *
     * @DelicateCoroutinesApi
     *
     * Transliterated from:
     * ATOMIC
     */
    ATOMIC,

    /**
     * Immediately executes the coroutine until its first suspension point _in the current thread_.
     *
     * Starting a coroutine using [UNDISPATCHED] is similar to using [Dispatchers::Unconfined] with [DEFAULT], except:
     * - Resumptions from later suspensions will properly use the actual dispatcher from the coroutine's context.
     *   Only the code until the first suspension point will be executed immediately.
     * - Even if the coroutine was cancelled already, its code will still start running, similar to [ATOMIC].
     * - The coroutine will not form an event loop. See [Dispatchers::Unconfined] for an explanation of event loops.
     *
     * This set of behaviors makes [UNDISPATCHED] well-suited for cases where the coroutine has a distinct
     * initialization phase whose side effects we want to rely on later.
     *
     * Example:
     * ```cpp
     * int tasks = 0;
     * for (int i = 0; i < 3; ++i) {
     *     launch(scope, CoroutineStart::UNDISPATCHED, [&tasks]() {
     *         tasks++;
     *         try {
     *             std::cout << "Waiting for a reply...\n";
     *             delay(50, cont);
     *             std::cout << "Got a reply!\n";
     *         } catch (...) {
     *             tasks--;
     *             throw;
     *         }
     *         tasks--;
     *     });
     * }
     * // Because of UNDISPATCHED,
     * // we know that the tasks already ran to their first suspension point,
     * // so this number is non-zero initially.
     * while (tasks > 0) {
     *     std::cout << "currently active: " << tasks << "\n";
     *     delay(10, cont);
     * }
     * ```
     *
     * **Pitfall**: unlike [Dispatchers::Unconfined] and [MainCoroutineDispatcher::immediate], nested undispatched
     * coroutines do not form an event loop that otherwise prevents potential stack overflow in case of unlimited
     * nesting. This property is necessary for the use case of guaranteed initialization, but may be undesirable in
     * other cases.
     * See [Dispatchers::Unconfined] for an explanation of event loops.
     *
     * Transliterated from:
     * UNDISPATCHED
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
