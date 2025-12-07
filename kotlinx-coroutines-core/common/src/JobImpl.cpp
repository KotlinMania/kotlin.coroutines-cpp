/**
 * @file JobImpl.cpp
 * @brief Implementation of JobImpl.
 *
 * NOTE: The detailed API documentation, KDocs, and class definitions are located
 * in the companion header file: `include/kotlinx/coroutines/JobImpl.hpp`.
 */

#include "JobImpl.hpp"
#include "../../../include/kotlinx/coroutines/CoroutineContext.hpp"
#include <stdexcept>

namespace kotlinx {
namespace coroutines {

// JobImpl implementation

JobImpl::JobImpl(bool active) : JobSupport(active) {
}

JobImpl::JobImpl(struct Job* parent) : JobSupport(true) {
    // Call initParentJob as in the Kotlin version
    init_parent_job(parent ? std::shared_ptr<struct Job>(parent, [](struct Job*){}) : nullptr);
}

JobImpl::~JobImpl() = default;

// CoroutineContext::Element interface implementation
CoroutineContext::Key* JobImpl::key() const {
    return Job::key;
}

// JobImpl-specific implementation
bool JobImpl::handles_exception() const {
    // Based on Kotlin: override val handlesException: Boolean = handlesException()
    // For now, simplified implementation
    // TODO: Implement full parent chain checking logic
    return true; // Default to handling exceptions
}

// CompletableJob interface implementation
bool JobImpl::complete() {
    // Based on Kotlin: override fun complete() = makeCompleting(Unit)
    // Simplified implementation for now
    if (is_completed()) {
        return false; // Already completed
    }
    // In a full implementation, this would transition the state machine
    // For now, just return true to indicate successful completion
    return true;
}

bool JobImpl::complete_exceptionally(Throwable* exception) {
    // Based on Kotlin: makeCompleting(CompletedExceptionally(exception))
    // Simplified implementation for now
    if (is_completed()) {
        return false; // Already completed
    }
    // Cancel the job with the exception
    cancel(std::make_exception_ptr(std::runtime_error("Job completed exceptionally")));
    return true;
}

// Factory functions

CompletableJob* createJob(struct Job* parent) {
    return new JobImpl(parent);
}

CompletableJob* createSupervisorJob(struct Job* parent) {
    // SupervisorJob is similar to Job but with different cancellation behavior
    // For now, we'll use the same implementation
    // In a full implementation, this would create a SupervisorJobImpl
    return new JobImpl(parent);
}

} // namespace coroutines
} // namespace kotlinx