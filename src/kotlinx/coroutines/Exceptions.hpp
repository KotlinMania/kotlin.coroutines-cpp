#pragma once
// port-lint: source kotlinx-coroutines-core/common/src/Exceptions.common.kt
/**
 * @file Exceptions.hpp
 * @brief Exception types for kotlinx.coroutines
 *
 * Transliterated from: kotlinx-coroutines-core/common/src/Exceptions.common.kt
 */

#include "ExceptionTransport.hpp"
#include "../../kotlin/coroutines/cancellation/CancellationException.hpp"
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
