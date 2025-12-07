/**
 * @file Await.hpp
 * @brief Header declarations for awaitAll and joinAll functions.
 *
 * Transliterated from Kotlin to C++
 * Original: kotlinx-coroutines-core/common/src/Await.kt
 */

#pragma once

#include <vector>
#include <initializer_list>

namespace kotlinx {
namespace coroutines {

class Job;
template <typename T> class Deferred;

/**
 * Awaits for completion of given deferred values without blocking a thread and resumes normally with the list of values
 * when all deferred computations are complete or resumes with the first thrown exception if any of computations
 * complete exceptionally including cancellation.
 *
 * This function is **not** equivalent to `deferreds.map { it.await() }` which fails only when it sequentially
 * gets to wait for the failing deferred, while this `awaitAll` fails immediately as soon as any of the deferreds fail.
 *
 * This suspending function is cancellable: if the [Job] of the current coroutine is cancelled while this
 * suspending function is waiting, this function immediately resumes with [CancellationException].
 * There is a **prompt cancellation guarantee**: even if this function is ready to return the result, but was cancelled
 * while suspended, [CancellationException] will be thrown. See [suspendCancellableCoroutine] for low-level details.
 */
template<typename T>
std::vector<T> await_all(std::initializer_list<Deferred<T>*> deferreds);

/**
 * Awaits for completion of given deferred values without blocking a thread and resumes normally with the list of values
 * when all deferred computations are complete or resumes with the first thrown exception if any of computations
 * complete exceptionally including cancellation.
 *
 * This function is **not** equivalent to `this.map { it.await() }` which fails only when it sequentially
 * gets to wait for the failing deferred, while this `awaitAll` fails immediately as soon as any of the deferreds fail.
 *
 * This suspending function is cancellable: if the [Job] of the current coroutine is cancelled while this
 * suspending function is waiting, this function immediately resumes with [CancellationException].
 * There is a **prompt cancellation guarantee**: even if this function is ready to return the result, but was cancelled
 * while suspended, [CancellationException] will be thrown. See [suspendCancellableCoroutine] for low-level details.
 */
template<typename T>
std::vector<T> await_all(const std::vector<Deferred<T>*>& collection);

// TODO: Add join_all functions when Job class is fully implemented

} // namespace coroutines
} // namespace kotlinx