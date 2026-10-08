#pragma once
// port-lint: source kotlinx-coroutines-core/common/src/Exceptions.common.kt
// port-lint: source libraries/stdlib/common-non-jvm/src/kotlin/coroutines/cancellation/CancellationException.kt
/**
 * @file Exceptions.hpp
 * @brief Exception types for kotlinx.coroutines
 *
 * Transliterated from: kotlinx-coroutines-core/common/src/Exceptions.common.kt
 */

#include <stdexcept>
#include <string>
#include <exception>
#include <optional>
#include <cstdint>

namespace kotlinx {
namespace coroutines {

// Note: We use 'struct Job' everywhere because there's a function named 'Job' that shadows the type

/**
 * Thrown when an element cannot be found or retrieved.
 * Parity with Kotlin's kotlin.NoSuchElementException.
 */
class NoSuchElementException : public std::out_of_range {
public:
    explicit NoSuchElementException(const std::string& message = "No such element")
        : std::out_of_range(message) {}
};

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

/**
 * This exception gets thrown if an exception is caught while processing CompletionHandler invocation for Job.
 */
class CompletionHandlerException : public std::runtime_error {
private:
    std::exception_ptr cause_;

public:
    CompletionHandlerException(const std::string& message, std::exception_ptr cause)
        : std::runtime_error(message), cause_(cause) {}

    std::exception_ptr get_cause() const { return cause_; }
};

} // namespace coroutines
} // namespace kotlinx

namespace kotlin::coroutines::cancellation {

/**
 * Thrown by cancellable suspending functions if the Job of the coroutine is
 * cancelled while it is suspending. It indicates normal cancellation of a coroutine.
 * It is not printed to console/log by the default uncaught exception handler.
 * See CoroutineExceptionHandler.
 */
// Transliterated from: libraries/stdlib/common-non-jvm/src/kotlin/coroutines/cancellation/CancellationException.kt:11-16
// Transliterated from: kotlinx-coroutines-core/common/src/Exceptions.common.kt:12-14
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

namespace kotlinx::coroutines {
// Native Exceptions.kt imports the actual stdlib class, preserving its identity.
// Transliterated from: kotlinx-coroutines-core/native/src/Exceptions.kt:9-9
using kotlin::coroutines::cancellation::CancellationException;

// Transliterated from: kotlinx-coroutines-core/common/src/Exceptions.common.kt:16-22
class JobCancellationException;

/**
 * Thrown when an internal error occurs in the coroutines library.
 */
class CoroutinesInternalError : public std::runtime_error {
private:
    std::exception_ptr cause_;

public:
    CoroutinesInternalError(const std::string& message, std::exception_ptr cause)
        : std::runtime_error(message), cause_(cause) {}

    std::exception_ptr get_cause() const { return cause_; }
};

/**
 * Factory function to create a CancellationException with its original cause.
 * The caller owns and deletes the returned exception.
 *
 * Transliterated from: kotlinx-coroutines-core/native/src/Exceptions.kt:13-14
 */
CancellationException* cancellation_exception(const std::string& message, std::exception_ptr cause);
// Transliterated from: kotlinx-coroutines-core/native/src/Exceptions.kt:13-14
CancellationException* cancellation_exception(std::optional<std::string> message, std::exception_ptr cause);
// Transliterated from: kotlinx-coroutines-core/native/src/Exceptions.kt:13-14
CancellationException* cancellation_exception(const char* message, std::exception_ptr cause);

/**
 * Converts an exception_ptr to a CancellationException.
 * Transliterated from: protected fun Throwable.toCancellationException(message: String?): CancellationException
 *     (JobSupport.kt:421-422)
 *
 * If the exception is already a CancellationException, it is rethrown.
 * Otherwise, a new CancellationException is created with the original exception as cause.
 *
 * @param exception The exception to convert
 * @param message Optional message for the new CancellationException (if creating one)
 * @return A CancellationException wrapping the original exception
 */
inline std::exception_ptr to_cancellation_exception(
    std::exception_ptr exception,
    const std::string& message = ""
) {
    if (!exception) {
        return std::make_exception_ptr(CancellationException(
            message.empty() ? "Job was cancelled" : message));
    }

    // Check if already a CancellationException
    try {
        std::rethrow_exception(exception);
    } catch (const CancellationException&) {
        // Already a CancellationException, return as-is
        return exception;
    } catch (...) {
        // Wrap in CancellationException
        return std::make_exception_ptr(CancellationException(
            message.empty() ? "Job was cancelled" : message,
            exception));
    }
}

/**
 * Checks if an exception_ptr contains a CancellationException.
 * Transliterated from: cause is CancellationException (various places in JobSupport.kt)
 *
 * @param exception The exception to check
 * @return true if the exception is a CancellationException
 */
inline bool is_cancellation_exception(std::exception_ptr exception) {
    if (!exception) return false;
    try {
        std::rethrow_exception(exception);
    } catch (const CancellationException&) {
        return true;
    } catch (...) {
        return false;
    }
}

/**
 * For use in tests - whether to recover stack traces.
 * Native: false (no stack trace recovery support)
 */
extern const bool RECOVER_STACK_TRACES;

} // namespace kotlinx::coroutines

// The common expect surface exposes the selected Native actual implementation.
#include "native/Exceptions.hpp"
