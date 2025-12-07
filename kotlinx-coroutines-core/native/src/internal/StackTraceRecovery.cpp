#include "kotlinx/coroutines/core_fwd.hpp"
// Transliterated from Kotlin to C++
// Original: kotlinx-coroutines-core/native/src/internal/StackTraceRecovery.kt
//
// TODO: actual keyword - platform-specific implementation
// TODO: @PublishedApi annotation
// TODO: suspend inline function
// TODO: Continuation interface
// TODO: @Suppress annotation

namespace kotlinx {
namespace coroutines {
namespace {

// TODO: Remove imports, fully qualify or add includes:
// import kotlin.coroutines.*

// TODO: actual function
template<typename E>
E* recover_stack_trace(E* exception, Continuation<void>* continuation) {
    return exception;
}

// TODO: actual function (overload)
template<typename E>
E* recover_stack_trace(E* exception) {
    return exception;
}

// TODO: @PublishedApi actual function
template<typename E>
E* unwrap(E* exception) {
    return exception;
}

// TODO: actual suspend inline function
inline void recover_and_throw(std::exception* exception) {
    throw *exception;
}

// TODO: @Suppress("UNUSED")
// TODO: actual interface
class CoroutineStackFrame {
public:
    CoroutineStackFrame* caller_frame;

    virtual void* get_stack_trace_element() = 0;

    virtual ~CoroutineStackFrame() = default;
};

// TODO: actual typealias
using StackTraceElement = void*;

// TODO: actual function - extension on Throwable
void init_cause(std::exception& throwable, std::exception* cause) {
    // Empty implementation
}

} // namespace internal
} // namespace coroutines
} // namespace kotlinx
