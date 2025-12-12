#pragma once
/**
 * @file CoroutineStackFrame.hpp
 *
 * Transliterated from: kotlin.coroutines.jvm.internal.CoroutineStackFrame
 *
 * This internal interface is used by stacktrace recovery to walk coroutine frames.
 * Native implementations are no-ops.
 */

namespace kotlinx {
namespace coroutines {
namespace internal {

/**
 * Platform-specific representation of a stack trace element.
 * On native targets this is currently a stub.
 */
class StackTraceElement {
public:
    virtual ~StackTraceElement() = default;
};

/**
 * Coroutine stack frame interface.
 */
class CoroutineStackFrame {
public:
    virtual ~CoroutineStackFrame() = default;

    virtual CoroutineStackFrame* get_caller_frame() = 0;
    virtual StackTraceElement* get_stack_trace_element() = 0;
};

} // namespace internal
} // namespace coroutines
} // namespace kotlinx

