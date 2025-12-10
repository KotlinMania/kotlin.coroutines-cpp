#pragma once
/**
 * @file Yield.hpp
 * @brief Yield function declaration
 *
 * Transliterated from: kotlinx-coroutines-core/common/src/Yield.kt
 *
 * Provides the yield_coroutine() function (named yield_coroutine to avoid conflict
 * with C++ keyword 'yield' in C++20).
 */

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
 * Calling yield_coroutine() has this effect.
 *
 * This suspending function is cancellable: if the Job of the current coroutine is cancelled while
 * yield is invoked or while waiting for dispatch, it immediately resumes with CancellationException.
 *
 * NOTE: In this C++ transliteration, this function yields the current thread's time slice
 * using std::this_thread::yield(). In true Kotlin coroutines, it would suspend the coroutine
 * without blocking the thread.
 *
 * NOTE: Named yield_coroutine instead of yield to avoid conflict with C++20's yield keyword.
 */
void yield_coroutine();

} // namespace coroutines
} // namespace kotlinx
