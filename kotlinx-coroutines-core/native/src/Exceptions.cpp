#include <string>
#include "kotlinx/coroutines/core_fwd.hpp"
// Transliterated from Kotlin to C++
// Original: kotlinx-coroutines-core/native/src/Exceptions.kt
//
// TODO: actual typealias - use C++ using declaration
// TODO: actual class with constructor
// TODO: @Suppress annotations
// TODO: @kotlin.internal.LowPriorityInOverloadResolution annotation

namespace kotlinx {
namespace coroutines {

/**
 * Thrown by cancellable suspending functions if the [Job] of the coroutine is cancelled while it is suspending.
 * It indicates _normal_ cancellation of a coroutine.
 * **It is not printed to console/log by default uncaught exception handler**.
 * (see [CoroutineExceptionHandler]).
 */
// TODO: actual typealias
using CancellationException = kotlin::coroutines::cancellation::CancellationException;

// TODO: @Suppress("INVISIBLE_MEMBER", "INVISIBLE_REFERENCE")
// TODO: @kotlin.internal.LowPriorityInOverloadResolution
CancellationException* make_cancellation_exception(std::string* message, std::exception* cause) {
    return new CancellationException(message, cause);
}

/**
 * Thrown by cancellable suspending functions if the [Job] of the coroutine is cancelled or completed
 * without cause, or with a cause or exception that is not [CancellationException]
 * (see [Job.getCancellationException]).
 */
// TODO: actual class
class JobCancellationException : CancellationException {
public:
    Job* job;

    JobCancellationException(std::string message, std::exception* cause, Job* job)
        : CancellationException(message, cause)
        , job(job)
    {
    }

    std::string to_string() const override {
        return CancellationException::to_string() + "; job=" + job->to_string();
    }

    bool equals(void* other) const override {
        if (other == this) return true;
        auto* other_ex = dynamic_cast<JobCancellationException*>(static_cast<std::exception*>(other));
        if (other_ex == nullptr) return false;
        return other_ex->what() == this->what() &&
               other_ex->job == this->job &&
               other_ex->cause == this->cause;
    }

    int hash_code() const override {
        return (std::hash<std::string>{}(this->what()) * 31 + std::hash<Job*>{}(job)) * 31 +
               (cause != nullptr ? std::hash<std::exception*>{}(cause) : 0);
    }
};

// For use in tests
// TODO: actual auto const bool kRecoverStackTraces = false;

} // namespace coroutines
} // namespace kotlinx
