/**
 * Transliterated from: kotlinx-coroutines-core/native/src/internal/StackTraceRecovery.kt
 *
 * Kotlin file header (translated):
 *   package kotlinx.coroutines.internal
 */

#include "kotlinx/coroutines/internal/StackTraceRecovery.hpp"

#include <exception>

namespace kotlinx::coroutines::internal {

/**
 * Upstream:
 *   internal actual fun <E: Throwable> recoverStackTrace(exception: E, continuation: Continuation<*>): E = exception
 *
 * Kotlin/Native carries no JVM-style stack augmentation, so the recovery functions are
 * identity. The C++ port mirrors that exactly.
 */
// Transliterated from: kotlinx-coroutines-core/native/src/internal/StackTraceRecovery.kt:5-5
std::exception_ptr recover_stack_trace(std::exception_ptr exception,
                                       const void* /*continuation*/) {
    return exception;
}

/** Upstream: internal actual fun <E: Throwable> recoverStackTrace(exception: E): E = exception */
// Transliterated from: kotlinx-coroutines-core/native/src/internal/StackTraceRecovery.kt:6-6
std::exception_ptr recover_stack_trace(std::exception_ptr exception) {
    return exception;
}

/** Upstream: @PublishedApi internal actual fun <E : Throwable> unwrap(exception: E): E = exception */
// Transliterated from: kotlinx-coroutines-core/native/src/internal/StackTraceRecovery.kt:9-9
std::exception_ptr unwrap(std::exception_ptr exception) {
    return exception;
}

/**
 * Upstream:
 *   internal actual suspend inline fun recoverAndThrow(exception: Throwable): Nothing = throw exception
 */
// Transliterated from: kotlinx-coroutines-core/native/src/internal/StackTraceRecovery.kt:10-10
[[noreturn]] void recover_and_throw(std::exception_ptr exception) {
    std::rethrow_exception(exception);
}

/**
 * Upstream:
 *   internal actual fun Throwable.initCause(cause: Throwable) {}
 *
 * This extension does nothing in Native. Constructors that accept a cause still retain it.
 */
// Transliterated from: kotlinx-coroutines-core/native/src/internal/StackTraceRecovery.kt:20-21
void init_cause(std::exception_ptr /*throwable*/, std::exception_ptr /*cause*/) {}

} // namespace kotlinx::coroutines::internal
