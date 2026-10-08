#pragma once
/** Transliterated from: kotlinx-coroutines-core/native/src/internal/StackTraceRecovery.kt:13-18 */

namespace kotlinx::coroutines::internal {

// NOTE(port): Native StackTraceElement is Any, erased at this internal ABI boundary.
// Transliterated from: kotlinx-coroutines-core/native/src/internal/StackTraceRecovery.kt:18-18
using StackTraceElement = void;

// Transliterated from: kotlinx-coroutines-core/native/src/internal/StackTraceRecovery.kt:13-16
class CoroutineStackFrame {
public:
    virtual ~CoroutineStackFrame() = default;
    // Transliterated from: kotlinx-coroutines-core/native/src/internal/StackTraceRecovery.kt:14-14
    virtual CoroutineStackFrame* get_caller_frame() const = 0;
    // Transliterated from: kotlinx-coroutines-core/native/src/internal/StackTraceRecovery.kt:15-15
    virtual StackTraceElement* get_stack_trace_element() const = 0;
};

} // namespace kotlinx::coroutines::internal
