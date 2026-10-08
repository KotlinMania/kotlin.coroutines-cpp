// port-lint: source libraries/stdlib/common-non-jvm/src/kotlin/coroutines/cancellation/CancellationException.kt
/**
 * Transliterated from: libraries/stdlib/common-non-jvm/src/kotlin/coroutines/cancellation/CancellationException.kt
 */
#include "CancellationException.hpp"
#include <bit>
#include <utility>

namespace kotlin::coroutines::cancellation {
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

// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/Any.kt:31-31
bool CancellationException::equals(const std::exception* other) const {
    return this == other;
}

// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/Any.kt:41-41
// Transliterated from: kotlin-native/runtime/src/main/cpp/Natives.cpp:40-49
std::int32_t CancellationException::hash_code() const {
    // NOTE(port): Inline the actual Native identity primitive for ordinary C++ object storage.
    return std::bit_cast<std::int32_t>(static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(this)));
}
} // namespace kotlin::coroutines::cancellation
