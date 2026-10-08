// port-lint: source kotlinx-coroutines-core/common/src/internal/DispatchedTask.kt
/** Transliterated from: kotlinx-coroutines-core/common/src/internal/DispatchedTask.kt */
#include "kotlinx/coroutines/internal/DispatchedTask.hpp"

namespace kotlinx::coroutines::internal {
// Transliterated from: kotlinx-coroutines-core/common/src/internal/DispatchedTask.kt:215-219
// NOTE(port): Existing pointer-boundary callers can supply null; source objects
// use their actual virtual string representation. The exception retains its cause.
DispatchException::DispatchException(std::exception_ptr cause_value,
                                    const CoroutineDispatcher* dispatcher,
                                    const CoroutineContext* context)
    : cause(cause_value),
      message_(std::string("Coroutine dispatcher ") +
               (dispatcher ? dispatcher->to_string() : "<null>") +
               " threw an exception, context = " +
               (context ? context->to_string() : "<null>")) {}

// NOTE(port): C++ exception transport view of the source constructor message.
const char* DispatchException::what() const noexcept { return message_.c_str(); }
} // namespace kotlinx::coroutines::internal
