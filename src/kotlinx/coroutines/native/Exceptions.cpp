// port-lint: source kotlinx-coroutines-core/native/src/Exceptions.kt
/**
 * Transliterated from: kotlinx-coroutines-core/native/src/Exceptions.kt
 */
#include "kotlinx/coroutines/Exceptions.hpp"
#include <utility>

namespace kotlinx::coroutines {

// Transliterated from: libraries/stdlib/common-non-jvm/src/kotlin/coroutines/cancellation/CancellationException.kt:12-12
CancellationException::CancellationException() : CancellationException(std::nullopt, nullptr) {}

// Transliterated from: libraries/stdlib/common-non-jvm/src/kotlin/coroutines/cancellation/CancellationException.kt:13-13
CancellationException::CancellationException(std::optional<std::string> message)
    : CancellationException(std::move(message), nullptr) {}

// Transliterated from: libraries/stdlib/common-non-jvm/src/kotlin/coroutines/cancellation/CancellationException.kt:13-13
CancellationException::CancellationException(const std::string& message)
    : CancellationException(std::optional<std::string>(message), nullptr) {}

// Transliterated from: libraries/stdlib/common-non-jvm/src/kotlin/coroutines/cancellation/CancellationException.kt:13-13
CancellationException::CancellationException(const char* message)
    : CancellationException(message ? std::optional<std::string>(message) : std::nullopt, nullptr) {}

// Transliterated from: libraries/stdlib/common-non-jvm/src/kotlin/coroutines/cancellation/CancellationException.kt:14-14
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/Throwable.kt:24-30
CancellationException::CancellationException(std::optional<std::string> message, std::exception_ptr cause)
    // NOTE(port): what() is a non-null C++ text view; get_message() retains Native nullability.
    : IllegalStateException(message.value_or("")), message_(std::move(message)), cause_(cause) {}

// Transliterated from: libraries/stdlib/common-non-jvm/src/kotlin/coroutines/cancellation/CancellationException.kt:14-14
CancellationException::CancellationException(const std::string& message, std::exception_ptr cause)
    : CancellationException(std::optional<std::string>(message), cause) {}

// Transliterated from: libraries/stdlib/common-non-jvm/src/kotlin/coroutines/cancellation/CancellationException.kt:14-14
CancellationException::CancellationException(const char* message, std::exception_ptr cause)
    : CancellationException(message ? std::optional<std::string>(message) : std::nullopt, cause) {}

// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/Throwable.kt:26-26
const std::optional<std::string>& CancellationException::get_message() const { return message_; }

// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/Throwable.kt:27-27
std::exception_ptr CancellationException::get_cause() const { return cause_; }

// Transliterated from: kotlinx-coroutines-core/native/src/Exceptions.kt:13-14
// NOTE(port): This owning factory result must be deleted by its C++ caller.
CancellationException* cancellation_exception(const std::string& message, std::exception_ptr cause) {
    return new CancellationException(message, cause);
}

// Transliterated from: kotlinx-coroutines-core/native/src/Exceptions.kt:13-14
CancellationException* cancellation_exception(std::optional<std::string> message, std::exception_ptr cause) {
    return new CancellationException(std::move(message), cause);
}

// Transliterated from: kotlinx-coroutines-core/native/src/Exceptions.kt:13-14
CancellationException* cancellation_exception(const char* message, std::exception_ptr cause) {
    return new CancellationException(message, cause);
}

// For use in tests.
// Transliterated from: kotlinx-coroutines-core/native/src/Exceptions.kt:34-35
const bool RECOVER_STACK_TRACES = false;

} // namespace kotlinx::coroutines
