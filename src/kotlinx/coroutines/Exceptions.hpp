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
 * For use in tests - whether to recover stack traces.
 * Native: false (no stack trace recovery support)
 */
extern const bool RECOVER_STACK_TRACES;

} // namespace coroutines
} // namespace kotlinx
