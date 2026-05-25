/**
 * Transliterated from: kotlinx-coroutines-core/native/src/internal/StackTraceRecovery.kt
 *
 * Kotlin file header (translated):
 *   package kotlinx.coroutines.internal
 */

#include "kotlinx/coroutines/Continuation.hpp"

#include <exception>

namespace kotlinx::coroutines::internal {

/**
 * Upstream:
 *   internal actual fun <E: Throwable> recoverStackTrace(exception: E, continuation: Continuation<*>): E = exception
 *
 * Kotlin/Native carries no JVM-style stack augmentation, so the recovery functions are
 * identity. The C++ port mirrors that exactly.
 */
std::exception_ptr recover_stack_trace(std::exception_ptr exception,
                                       Continuation<void*>* /*continuation*/) {
    return exception;
}

/** Upstream: internal actual fun <E: Throwable> recoverStackTrace(exception: E): E = exception */
std::exception_ptr recover_stack_trace(std::exception_ptr exception) {
    return exception;
}

/** Upstream: @PublishedApi internal actual fun <E : Throwable> unwrap(exception: E): E = exception */
std::exception_ptr unwrap(std::exception_ptr exception) {
    return exception;
}

/**
 * Upstream:
 *   internal actual suspend inline fun recoverAndThrow(exception: Throwable): Nothing = throw exception
 */
[[noreturn]] void recover_and_throw(std::exception_ptr exception) {
    std::rethrow_exception(exception);
}

/**
 * Upstream:
 *   internal actual fun Throwable.initCause(cause: Throwable) {}
 *
 * Native Kotlin throws don't carry a cause chain — this is a no-op.
 */
void init_cause(std::exception_ptr /*throwable*/, std::exception_ptr /*cause*/) {}

} // namespace kotlinx::coroutines::internal
