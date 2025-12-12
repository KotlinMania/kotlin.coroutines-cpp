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
public:
    explicit CancellationException(const std::string& message)
        : std::runtime_error(message) {}

    virtual ~CancellationException() = default;
};

/**
 * Thrown by cancellable suspending functions if the Job of the coroutine is cancelled
 * while it is suspending. Contains a reference to the job that was cancelled.
 */
class JobCancellationException : public CancellationException {
private:
    std::exception_ptr cause_;
    struct Job* job_;

public:
    JobCancellationException(const std::string& message, std::exception_ptr cause, struct Job* job)
        : CancellationException(message), cause_(cause), job_(job) {}

    struct Job* get_job() const { return job_; }
    std::exception_ptr get_cause() const { return cause_; }
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

} // namespace coroutines
} // namespace kotlinx
