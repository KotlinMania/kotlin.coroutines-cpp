#pragma once
/**
 * @file Dsl.hpp
 * @brief Unified header for all kotlinx.coroutines DSL macros and helpers.
 *
 * This header provides a complete DSL for writing suspend functions in C++
 * that match Kotlin/Native's coroutine patterns.
 *
 * Includes:
 * - Suspend.hpp:          coroutine_begin/yield/end macros (computed goto)
 * - Cancellable.hpp:      suspendCancellableCoroutine pattern
 * - CancellableReusable.hpp: suspendCancellableCoroutineReusable pattern
 * - VarSpilling.hpp:      Variable save/restore across suspend points
 * - ChannelSuspend.hpp:   Channel-specific suspend patterns
 * - Await.hpp:            Await/async patterns
 * - Coroutines.hpp:       Basic coroutine utilities
 *
 * Usage:
 * ```cpp
 * #include <kotlinx/coroutines/dsl/Dsl.hpp>
 * using namespace kotlinx::coroutines::dsl;
 *
 * class MyCoroutine : public ContinuationImpl {
 *     void* _label = nullptr;
 *     int _spilled_count;
 *
 *     void* invoke_suspend(Result<void*> result) override {
 *         int count;
 *
 *         coroutine_begin(this)
 *
 *         count = 0;
 *         while (count < 10) {
 *             KXS_SAVE_VAR(this, count, _spilled_count);
 *             coroutine_yield(this, delay(100, completion));
 *             KXS_RESTORE_VAR(this, count, _spilled_count);
 *             count++;
 *         }
 *
 *         coroutine_end(this)
 *     }
 * };
 * ```
 */

// Core state machine macros (computed goto / Duff's device)
#include "kotlinx/coroutines/dsl/Suspend.hpp"

// Cancellable coroutine patterns
#include "kotlinx/coroutines/dsl/Cancellable.hpp"
#include "kotlinx/coroutines/dsl/CancellableReusable.hpp"

// Variable spilling (save/restore state)
#include "kotlinx/coroutines/dsl/VarSpilling.hpp"

// Channel-specific patterns
#include "kotlinx/coroutines/dsl/ChannelSuspend.hpp"

// Await patterns
#include "kotlinx/coroutines/dsl/Await.hpp"

// Basic utilities
#include "kotlinx/coroutines/dsl/Coroutines.hpp"

namespace kotlinx {
namespace coroutines {
namespace dsl {

/**
 * DSL version for compatibility checks.
 */
constexpr int KXS_DSL_VERSION_MAJOR = 1;
constexpr int KXS_DSL_VERSION_MINOR = 0;

} // namespace dsl
} // namespace coroutines
} // namespace kotlinx
