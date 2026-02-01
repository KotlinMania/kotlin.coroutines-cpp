#pragma once
// port-lint: source Yield.kt
/**
 * @file Yield.hpp
 * @brief Yield function declaration
 *
 * Transliterated from: kotlinx-coroutines-core/common/src/Yield.kt
 *
 * Provides the yield_coroutine() function (named yield_coroutine to avoid conflict
 * with C++ keyword 'yield' in C++20).
 */
#include <memory>
#include "kotlinx/coroutines/Continuation.hpp"
namespace kotlinx {
namespace coroutines {

/**
 * Suspends this coroutine and immediately schedules it for further execution.
 *
 * A coroutine runs uninterrupted on a thread until the coroutine suspends,
 * giving other coroutines a chance to use that thread for their computations.
 * Normally, coroutines suspend whenever they wait for something to happen:
 * for example, trying to receive a value from a channel that's currently empty will suspend.
 * Sometimes, a coroutine does not need to wait for anything,
 * but we still want it to give other coroutines a chance to run.
 * Calling yield has this effect:
 *
 * ```cpp
 * void update_progress_bar(int value, const char* marker) {
 *     std::cout << marker;
 * }
 * auto single_threaded_dispatcher = Dispatchers::Default->limited_parallelism(1);
 * with_context(single_threaded_dispatcher, [&]() {
 *     launch(scope, []() {
 *         for (int i = 0; i < 5; ++i) {
 *             update_progress_bar(i, "A");
 *             yield(cont);
 *         }
 *     });
 *     launch(scope, []() {
 *         for (int i = 0; i < 5; ++i) {
 *             update_progress_bar(i, "B");
 *             yield(cont);
 *         }
 *     });
 * });
 * ```
 *
 * In this example, without the yield, first, A would run its five stages of work to completion, and only then
 * would B even start executing. With both yield calls, the coroutines share the single thread with each other
 * after each stage of work. This is useful when several coroutines running on the same thread (or thread pool)
 * must regularly publish their results for the program to stay responsive.
 *
 * This suspending function is cancellable: if the Job of the current coroutine is cancelled while
 * yield is invoked or while waiting for dispatch, it immediately resumes with CancellationException.
 * There is a **prompt cancellation guarantee**: even if this function is ready to return the result, but was cancelled
 * while suspended, CancellationException will be thrown. See suspend_cancellable_coroutine for low-level details.
 *
 * **Note**: if there is only a single coroutine executing on the current dispatcher,
 * it is possible that yield will not actually suspend.
 * However, even in that case, the check for cancellation still happens.
 *
 * **Note**: if there is no CoroutineDispatcher in the context, it does not suspend.
 *
 * ## Pitfall: using yield to wait for something to happen
 *
 * Using yield for anything except a way to ensure responsiveness is often a problem.
 * When possible, it is recommended to structure the code in terms of coroutines waiting for some events instead of
 * yielding.
 *
 * ### Case 1: using yield to ensure a specific interleaving of actions
 *
 * It is an antipattern to use yield to synchronize code across several coroutines.
 * Use Job::join() instead.
 *
 * ### Case 2: using yield in a loop to wait for something to happen
 *
 * `while (channel.is_empty()) { yield(cont); } channel.receive(cont);` can be replaced with just `channel.receive(cont)`;
 * `while (job.is_active()) { yield(cont); }` can be replaced with `job.join(cont)`;
 * in both cases, this will avoid the unnecessary work of checking the loop conditions.
 * In general, seek ways to allow a coroutine to stay suspended until it actually has useful work to do.
 *
 * ## Implementation details
 *
 * Some coroutine dispatchers include optimizations that make yielding different from normal suspensions.
 * For example, when yielding, Dispatchers::Unconfined checks whether there are any other coroutines in the event
 * loop where the current coroutine executes; if not, the sole coroutine continues to execute without suspending.
 *
 * For custom implementations of CoroutineDispatcher, this function checks CoroutineDispatcher::is_dispatch_needed and
 * then invokes CoroutineDispatcher::dispatch regardless of the result; no way is provided to change this behavior.
 *
 * @note Named yield_coroutine in C++ to avoid conflict with C++20's yield keyword.
 *
 * Transliterated from:
 * public suspend fun yield(): Unit
 */

/*
 * TODO(semantics): STUB - yield_coroutine() yields thread instead of suspending coroutine
 *
 * What's missing:
 * - Should be a suspend function: suspend fun yield()
 * - Should check cancellation, then suspend and re-dispatch coroutine
 * - Allows other coroutines on same dispatcher to run
 * - See Yield.cpp for full TODO details
 *
 * Current behavior: Calls std::this_thread::yield() - OS level thread yield
 * Correct behavior: Suspend coroutine, re-enqueue to dispatcher for cooperative scheduling
 */
[[deprecated("Use yield(completion) instead")]]
void yield_coroutine();

/**
 * Suspend function form - requires continuation.
 * This is the proper suspend function implementation.
 *
 * Transliterated from:
 * public suspend fun yield(): Unit = suspendCoroutineUninterceptedOrReturn sc@ { uCont -> ... }
 */
void* yield(std::shared_ptr<Continuation<void*>> completion);

/**
 * No-arg yield for test compatibility.
 * In Kotlin, yield() is a suspend function called within coroutines.
 * In C++ tests not using full coroutine machinery, this provides
 * a simple OS-level thread yield as a stand-in.
 */
inline void yield() {
    // Suppress deprecation warning - this is the test compatibility wrapper
    #pragma clang diagnostic push
    #pragma clang diagnostic ignored "-Wdeprecated-declarations"
    yield_coroutine();
    #pragma clang diagnostic pop
}

} // namespace coroutines
} // namespace kotlinx
