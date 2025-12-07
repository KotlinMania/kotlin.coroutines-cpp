#pragma once
#include <stdexcept>
#include <string>
#include <exception>
#include "core_fwd.hpp"

namespace kotlinx {
namespace coroutines {

class Job; // Forward declaration

/**
 * This exception gets thrown if an exception is caught while processing [CompletionHandler] invocation for [Job].
 */
class CompletionHandlerException : public std::runtime_error {
private:
    std::exception_ptr cause_;

public:
    CompletionHandlerException(const std::string& message, std::exception_ptr cause)
        : std::runtime_error(message), cause_(cause) {}

    std::exception_ptr get_cause() const { return cause_; }
};

class CancellationException : public std::runtime_error {
public:
    explicit CancellationException(const std::string& message)
        : std::runtime_error(message) {}

    virtual ~CancellationException() = default;
};

class JobCancellationException : public CancellationException {
private:
    std::exception_ptr cause_;
    Job* job_;

public:
    JobCancellationException(const std::string& message, std::exception_ptr cause, Job* job)
        : CancellationException(message), cause_(cause), job_(job) {}

    Job* get_job() const { return job_; }
    std::exception_ptr get_cause() const { return cause_; }
};

class CoroutinesInternalError : public std::runtime_error {
private:
    std::exception_ptr cause_;

public:
    CoroutinesInternalError(const std::string& message, std::exception_ptr cause)
        : std::runtime_error(message), cause_(cause) {}

    std::exception_ptr get_cause() const { return cause_; }
};

// Global function delcaration
CancellationException* make_cancellation_exception(const std::string& message, std::exception_ptr cause);

} // namespace coroutines
} // namespace kotlinx
