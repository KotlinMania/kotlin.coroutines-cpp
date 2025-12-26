#pragma once
/**
 * @file Exceptions.hpp
 * @brief Exception types for kotlinx.coroutines
 *
 * Transliterated from: kotlinx-coroutines-core/common/src/Exceptions.common.kt
 */

#include <stdexcept>
#include <string>
#include <exception>

namespace kotlinx {
namespace coroutines {

// Note: We use 'struct Job' everywhere because there's a function named 'Job' that shadows the type

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

/**
 * Thrown by cancellable suspending functions if the Job of the coroutine is cancelled
 * while it is suspending.
 *
 * Unlike most other exceptions, this exception is caught by the parent coroutine's machinery
 * and does not cause its parent to fail.
 */
class CancellationException : public std::runtime_error {
private:
    std::exception_ptr cause_;

public:
    explicit CancellationException(const std::string& message)
        : std::runtime_error(message), cause_(nullptr) {}

    CancellationException(const std::string& message, std::exception_ptr cause)
        : std::runtime_error(message), cause_(cause) {}

    std::exception_ptr get_cause() const { return cause_; }

    virtual ~CancellationException() = default;
};

/**
 * Thrown by cancellable suspending functions if the Job of the coroutine is cancelled
 * or completed without cause, or with a cause or exception that is not CancellationException.
 * See Job.getCancellationException().
 */
class JobCancellationException : public CancellationException {
private:
    struct Job* job_;

public:
    JobCancellationException(const std::string& message, std::exception_ptr cause, struct Job* job)
        : CancellationException(message, cause), job_(job) {}

    struct Job* get_job() const { return job_; }
    // get_cause() inherited from CancellationException
};

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
 * Factory function to create a CancellationException.
 */
CancellationException* make_cancellation_exception(const std::string& message, std::exception_ptr cause);

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

} // namespace coroutines
} // namespace kotlinx
