#include "kotlinx/coroutines/core_fwd.hpp"
// Transliterated from Kotlin to C++ (first-pass, mechanical syntax mapping)
// Original: kotlinx-coroutines-core/common/src/Exceptions.common.kt
//
// TODO:
// - expect class/function need platform-specific implementations
// - Exception hierarchy needs proper C++ exception classes
// - RuntimeException, IllegalStateException base classes

#include <stdexcept>
#include <string>

namespace kotlinx {
namespace coroutines {

class Job; // Forward declaration

/**
 * This exception gets thrown if an exception is caught while processing [CompletionHandler] invocation for [Job].
 *
 * @suppress **This an API and should not be used from general code.**
 */
// @InternalCoroutinesApi
class CompletionHandlerException : std::runtime_error {
private:
    std::exception_ptr cause_;

public:
    CompletionHandlerException(const std::string& message, std::exception_ptr cause)
        : std::runtime_error(message), cause_(cause) {}

    std::exception_ptr get_cause() const { return cause_; }
};

// TODO: expect open class - needs platform-specific base implementation
class CancellationException : std::runtime_error {
public:
    explicit CancellationException(const std::string& message)
        : std::runtime_error(message) {}

    virtual ~CancellationException() = default;
};

// TODO: expect function - factory function for CancellationException with cause
CancellationException* make_cancellation_exception(const std::string& message, std::exception_ptr cause);

// TODO: expect class - needs platform-specific implementation
class JobCancellationException : CancellationException {
private:
    std::exception_ptr cause_;
    Job* job_;

public:
    JobCancellationException(const std::string& message, std::exception_ptr cause, Job* job)
        : CancellationException(message), cause_(cause), job_(job) {}

    Job* get_job() const { return job_; }
    std::exception_ptr get_cause() const { return cause_; }
};

class CoroutinesInternalError : std::runtime_error {
private:
    std::exception_ptr cause_;

public:
    CoroutinesInternalError(const std::string& message, std::exception_ptr cause)
        : std::runtime_error(message), cause_(cause) {}

    std::exception_ptr get_cause() const { return cause_; }
};

// For use in tests
// TODO: expect auto - needs platform-specific implementation
extern const bool RECOVER_STACK_TRACES;

} // namespace coroutines
} // namespace kotlinx
