#pragma once
/**
 * @file Coroutine.hpp
 * @brief Kotlin Coroutine Runtime for C++
 *
 * This header provides the unified coroutine API, directly transliterated from
 * Kotlin's coroutine runtime (kotlin.coroutines package).
 *
 * ## Architecture
 *
 * The implementation has two layers:
 *
 * ### 1. Kotlin Runtime (kotlin::coroutines namespace)
 * Direct transliteration of Kotlin's coroutine kernel:
 * - `Continuation<T>` - continuation interface
 * - `BaseContinuationImpl` - the resume_with/invoke_suspend loop
 * - `ContinuationImpl` - adds context and interception
 * - `COROUTINE_SUSPENDED` - suspension marker
 *
 * ### 2. kotlinx.coroutines Library (kotlinx::coroutines namespace)
 * Higher-level constructs built on the runtime:
 * - `Job` - lifecycle and cancellation
 * - `Deferred<T>` - async result
 * - `CoroutineScope` - structured concurrency
 * - Builders: `launch`, `async`, `runBlocking`
 *
 * ## How Suspend Works
 *
 * In Kotlin, `suspend fun` is compiled to a state machine that:
 * 1. Takes a `Continuation` parameter (the callback)
 * 2. Returns `Any?` (either the result or `COROUTINE_SUSPENDED`)
 * 3. Stores local variables in fields (spilling)
 * 4. Uses a `label` field to track state
 *
 * In C++, we replicate this with:
 * - `BaseContinuationImpl::invoke_suspend()` - the state machine method
 * - `COROUTINE_SUSPENDED` marker for suspension
 * - Macros (coroutine_begin/yield/end) for state machine generation
 *
 * ## Usage Example
 *
 * ```cpp
 * // Create a suspend function as a class
 * class MySuspendFunction : public ContinuationImpl {
 *     void* _label = nullptr;  // State machine label (blockaddress)
 *     int saved_x;             // Spilled local variable
 *
 *     void* invoke_suspend(Result<void*> result) override {
 *         coroutine_begin(this)
 *
 *         saved_x = 42;
 *         coroutine_yield(this, delay(100, completion_));
 *
 *         return (void*)(intptr_t)saved_x;  // Return value
 *
 *         coroutine_end(this)
 *     }
 * };
 * ```
 *
 * See StacklessBuilders.hpp for the CO_* macros that generate this pattern.
 */

// Coroutine runtime
#include "kotlinx/coroutines/intrinsics/Intrinsics.hpp"
#include "kotlinx/coroutines/Continuation.hpp"
#include "kotlinx/coroutines/ContinuationImpl.hpp"

// kotlinx.coroutines library
#include "kotlinx/coroutines/Job.hpp"
#include "kotlinx/coroutines/Deferred.hpp"
#include "kotlinx/coroutines/Result.hpp"
#include "kotlinx/coroutines/CancellableContinuation.hpp"
