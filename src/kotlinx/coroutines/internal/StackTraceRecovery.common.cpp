// port-lint: source internal/StackTraceRecovery.common.kt
/**
 * Transliterated from: kotlinx-coroutines-core/common/src/internal/StackTraceRecovery.common.kt
 *
 * Kotlin file header (translated):
 *   package kotlinx.coroutines.internal
 *
 * Upstream:
 *   internal expect fun <E : Throwable> recoverStackTrace(exception: E, continuation: Continuation<*>): E
 *   internal expect fun <E : Throwable> recoverStackTrace(exception: E): E
 *   internal expect suspend inline fun recoverAndThrow(exception: Throwable): Nothing
 *   @PublishedApi internal expect fun <E : Throwable> unwrap(exception: E): E
 *   internal expect fun Throwable.initCause(cause: Throwable)
 *   internal expect interface CoroutineStackFrame {
 *       val callerFrame: CoroutineStackFrame?
 *       fun getStackTraceElement(): StackTraceElement?
 *   }
 *   internal expect typealias StackTraceElement = ...
 *
 * Recovery / cause / stack-augmentation is a JVM-debug-mode feature. K/N actuals are
 * identity-functions (no augmentation). The C++ port matches the K/N behavior; the
 * matching `CoroutineStackFrame` interface is declared in `CoroutineStackFrame.hpp` and
 * `StackTraceElement` is a void* opaque.
 */

#include "kotlinx/coroutines/internal/CoroutineStackFrame.hpp"

#include <exception>

namespace kotlinx::coroutines::internal {

template <typename T>
class Continuation;

/**
 * Upstream:
 *   internal expect fun <E : Throwable> recoverStackTrace(exception: E, continuation: Continuation<*>): E
 */
template <typename E>
inline E* recover_stack_trace(E* exception, Continuation<void>* /*continuation*/) {
    return exception;
}

/**
 * Upstream:
 *   internal expect fun <E : Throwable> recoverStackTrace(exception: E): E
 */
template <typename E>
inline E* recover_stack_trace(E* exception) {
    return exception;
}

/**
 * Upstream:
 *   internal expect fun Throwable.initCause(cause: Throwable)
 *
 * No JVM-style cause chain on K/N — identity for compatibility with the API surface.
 */
inline void init_cause(std::exception* /*self*/, std::exception* /*cause*/) {}

/**
 * Upstream:
 *   @PublishedApi internal expect fun <E : Throwable> unwrap(exception: E): E
 *
 * Guarantee: `unwrap(recoverStackTrace(e)) === e`. K/N's `recoverStackTrace` is identity,
 * so `unwrap` is identity too.
 */
template <typename E>
inline E* unwrap(E* exception) {
    return exception;
}

} // namespace kotlinx::coroutines::internal
