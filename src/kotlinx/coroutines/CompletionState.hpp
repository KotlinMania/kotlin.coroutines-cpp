#pragma once
/**
 * Transliterated from: kotlinx-coroutines-core/common/src/CompletionState.kt
 *
 * Kotlin file header (translated):
 *   package kotlinx.coroutines
 */

#include "kotlinx/coroutines/CancellableContinuation.hpp"
#include "kotlinx/coroutines/CompletedExceptionally.hpp"
#include "kotlinx/coroutines/Continuation.hpp"
#include "kotlinx/coroutines/Result.hpp"
#include "kotlinx/coroutines/internal/StackTraceRecovery.hpp"

#include <memory>

namespace kotlinx::coroutines {

/**
 * Upstream:
 *   internal fun <T> Result<T>.toState(): Any? = getOrElse { CompletedExceptionally(it) }
 *
 * `Any?` in the C++ port is modelled as a heap-allocated payload pointer. On success the
 * value is moved onto the heap; on failure a [CompletedExceptionally] carries the cause.
 * Ownership of the returned pointer transfers to the caller.
 */
template <typename T>
inline void* to_state(Result<T> result) {
    if (result.is_success()) {
        return new T(result.get_or_throw());
    }
    return new CompletedExceptionally(result.exception_or_null());
}

/**
 * Upstream:
 *   internal fun <T> Result<T>.toState(caller: CancellableContinuation<*>): Any? =
 *       getOrElse { CompletedExceptionally(recoverStackTrace(it, caller)) }
 */
template <typename T>
inline void* to_state(Result<T> result, CancellableContinuation<void>* caller) {
    if (result.is_success()) {
        return new T(result.get_or_throw());
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
template <typename T>
inline Result<T> recover_result(void* state, Continuation<T>* u_cont) {
    if (auto* completed_exceptionally = dynamic_cast<CompletedExceptionally*>(
            reinterpret_cast<CompletedExceptionally*>(state))) {
        return Result<T>::failure(
            internal::recover_stack_trace(completed_exceptionally->cause, u_cont));
    }
    return Result<T>::success(*static_cast<T*>(state));
}

} // namespace kotlinx::coroutines
