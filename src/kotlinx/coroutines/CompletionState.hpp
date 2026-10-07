#pragma once
/**
 * Transliterated from: kotlinx-coroutines-core/common/src/CompletionState.kt
 *
 * Kotlin file header (translated):
 *   package kotlinx.coroutines
 */

#include "kotlinx/coroutines/CancellableContinuation.hpp"
#include "kotlinx/coroutines/CompletedExceptionally.hpp"
#include "kotlinx/coroutines/CompletedValue.hpp"
#include "kotlinx/coroutines/Continuation.hpp"
#include "kotlinx/coroutines/Result.hpp"
#include "kotlinx/coroutines/internal/StackTraceRecovery.hpp"

#include <memory>

namespace kotlinx::coroutines {

/**
 * Upstream:
 *   internal fun <T> Result<T>.toState(): Any? = getOrElse { CompletedExceptionally(it) }
 *
 * NOTE(port): JobState boxes preserve the actual runtime type of Any? so that
 * CompletedExceptionally and successful values can be distinguished safely.
 * Ownership transfers to the job state machine.
 */
// Transliterated from: kotlinx-coroutines-core/common/src/CompletionState.kt:8-8
template <typename T>
inline JobState* to_state(Result<T> result) {
    if (result.is_success()) {
        return new CompletedValue<T>(result.get_or_throw());
    }
    return new CompletedExceptionally(result.exception_or_null());
}

/**
 * Upstream:
 *   internal fun <T> Result<T>.toState(caller: CancellableContinuation<*>): Any? =
 *       getOrElse { CompletedExceptionally(recoverStackTrace(it, caller)) }
 */
// Transliterated from: kotlinx-coroutines-core/common/src/CompletionState.kt:10-11
template <typename T>
inline JobState* to_state(Result<T> result, CancellableContinuation<void>* caller) {
    if (result.is_success()) {
        return new CompletedValue<T>(result.get_or_throw());
    }
    return new CompletedExceptionally(
        internal::recover_stack_trace(result.exception_or_null(), caller));
}

/**
 * Upstream:
 *   @Suppress("RESULT_CLASS_IN_RETURN_TYPE", "UNCHECKED_CAST")
 *   internal fun <T> recoverResult(state: Any?, uCont: Continuation<T>): Result<T> =
 *       if (state is CompletedExceptionally)
 *           Result.failure(recoverStackTrace(state.cause, uCont))
 *       else
 *           Result.success(state as T)
 */
// Transliterated from: kotlinx-coroutines-core/common/src/CompletionState.kt:14-18
template <typename T>
inline Result<T> recover_result(JobState* state, Continuation<T>* u_cont) {
    if (auto* completed_exceptionally = dynamic_cast<CompletedExceptionally*>(state)) {
        return Result<T>::failure(
            internal::recover_stack_trace(completed_exceptionally->cause, u_cont));
    }
    return Result<T>::success(dynamic_cast<CompletedValue<T>&>(*state).value);
}

} // namespace kotlinx::coroutines
