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
 * - `BaseContinuationImpl` - the resumeWith/invokeSuspend loop
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
 * - `BaseContinuationImpl::invokeSuspend()` - the state machine method
 * - `COROUTINE_SUSPENDED` marker for suspension
 * - Protothreads macros (CO_BEGIN/CO_END) for state machine generation
 *
 * ## Usage Example
 *
 * ```cpp
 * // Create a suspend function as a class
 * class MySuspendFunction : public ContinuationImpl {
 *     int label = 0;  // State machine label
 *     int saved_x;    // Spilled local variable
 *
 *     void* invokeSuspend(Result<void*> result) override {
 *         switch (label) {
 *         case 0:
 *             saved_x = 42;
 *             label = 1;
 *             // Call another suspend function, passing this as continuation
 *             if (delay(100, this) == COROUTINE_SUSPENDED)
 *                 return COROUTINE_SUSPENDED;
 *             [[fallthrough]];
 *         case 1:
 *             return (void*)(intptr_t)saved_x;  // Return value
 *         }
 *         return nullptr;
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
