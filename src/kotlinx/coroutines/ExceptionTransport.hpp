#pragma once
// NOTE(port): Existing C++ exception transport used by the Native
// CancellationException constructors. Full kotlin.IllegalStateException ancestry
// is not defined by this carrier. Source dependency:
// libraries/stdlib/common-non-jvm/src/kotlin/coroutines/cancellation/CancellationException.kt:11-16.
#include <stdexcept>
#include <string>

namespace kotlinx::coroutines {
/**
 * Signals that a method has been invoked at an illegal or inappropriate time.
 * Parity with Kotlin's java.lang.IllegalStateException / kotlin.IllegalStateException.
 */
class IllegalStateException : public std::logic_error {
public:
    explicit IllegalStateException(const std::string& message = "Illegal state")
        : std::logic_error(message) {}
    explicit IllegalStateException(const char* message)
        : std::logic_error(message) {}
};
} // namespace kotlinx::coroutines
