#pragma once
/** Transliterated from: kotlinx-coroutines-core/common/src/internal/StackTraceRecovery.common.kt */
#include "kotlinx/coroutines/internal/CoroutineStackFrame.hpp"
#include <exception>

namespace kotlinx::coroutines::internal {

// NOTE(port): Throwable is std::exception_ptr; Continuation<*> is an opaque pointer.
// Native recovery preserves the original exception and needs no frame inspection.
// Transliterated from: kotlinx-coroutines-core/native/src/internal/StackTraceRecovery.kt:5-5
std::exception_ptr recover_stack_trace(std::exception_ptr exception, const void* continuation);
// Transliterated from: kotlinx-coroutines-core/native/src/internal/StackTraceRecovery.kt:6-6
std::exception_ptr recover_stack_trace(std::exception_ptr exception);
// Transliterated from: kotlinx-coroutines-core/native/src/internal/StackTraceRecovery.kt:9-9
std::exception_ptr unwrap(std::exception_ptr exception);
// Transliterated from: kotlinx-coroutines-core/native/src/internal/StackTraceRecovery.kt:10-10
[[noreturn]] void recover_and_throw(std::exception_ptr exception);
// Transliterated from: kotlinx-coroutines-core/native/src/internal/StackTraceRecovery.kt:20-21
void init_cause(std::exception_ptr exception, std::exception_ptr cause);

} // namespace kotlinx::coroutines::internal
