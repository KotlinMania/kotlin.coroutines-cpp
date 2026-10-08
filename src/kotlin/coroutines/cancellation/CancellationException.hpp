#pragma once
// port-lint: source libraries/stdlib/common-non-jvm/src/kotlin/coroutines/cancellation/CancellationException.kt
/**
 * Transliterated from: libraries/stdlib/common-non-jvm/src/kotlin/coroutines/cancellation/CancellationException.kt
 */
#include "kotlinx/coroutines/ExceptionTransport.hpp"
#include <cstdint>
#include <exception>
#include <optional>
#include <string>

namespace kotlin::coroutines::cancellation {
/**
 * Thrown by cancellable suspending functions if the Job of the coroutine is
 * cancelled while it is suspending. It indicates normal cancellation of a coroutine.
 * It is not printed to console/log by the default uncaught exception handler.
 * See CoroutineExceptionHandler.
 */
// Transliterated from: libraries/stdlib/common-non-jvm/src/kotlin/coroutines/cancellation/CancellationException.kt:11-16
// NOTE(port): The existing C++ exception base supplies std::exception transport;
// its full kotlin.IllegalStateException ancestry remains a separate consumed dependency.
class CancellationException : public kotlinx::coroutines::IllegalStateException {
private:
    std::optional<std::string> message_;
    std::exception_ptr cause_;

public:
    // Transliterated from: libraries/stdlib/common-non-jvm/src/kotlin/coroutines/cancellation/CancellationException.kt:12-12
    CancellationException();
    // Transliterated from: libraries/stdlib/common-non-jvm/src/kotlin/coroutines/cancellation/CancellationException.kt:13-13
    explicit CancellationException(std::optional<std::string> message);
    // Transliterated from: libraries/stdlib/common-non-jvm/src/kotlin/coroutines/cancellation/CancellationException.kt:13-13
    explicit CancellationException(const std::string& message);
    // Transliterated from: libraries/stdlib/common-non-jvm/src/kotlin/coroutines/cancellation/CancellationException.kt:13-13
    explicit CancellationException(const char* message);

    // Transliterated from: libraries/stdlib/common-non-jvm/src/kotlin/coroutines/cancellation/CancellationException.kt:14-14
    CancellationException(std::optional<std::string> message, std::exception_ptr cause);
    // Transliterated from: libraries/stdlib/common-non-jvm/src/kotlin/coroutines/cancellation/CancellationException.kt:14-14
    CancellationException(const std::string& message, std::exception_ptr cause);
    // Transliterated from: libraries/stdlib/common-non-jvm/src/kotlin/coroutines/cancellation/CancellationException.kt:14-14
    CancellationException(const char* message, std::exception_ptr cause);

    // Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/Throwable.kt:26-27
    virtual const std::optional<std::string>& get_message() const;
    // Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/Throwable.kt:27-27
    virtual std::exception_ptr get_cause() const;

    // Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/Any.kt:31-31
    // NOTE(port): The C++ Throwable carrier is std::exception. Other objects
    // cannot be a JobCancellationException and are outside this typed projection.
    virtual bool equals(const std::exception* other) const;

    // Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/Any.kt:41-41
    virtual std::int32_t hash_code() const;

    ~CancellationException() override = default;
};
} // namespace kotlin::coroutines::cancellation
