/**
 * @file JobImpl.cpp
 * @brief Implementation of JobImpl.
 *
 * NOTE: The detailed API documentation, KDocs, and class definitions are located
 * in the companion header file: `include/kotlinx/coroutines/JobImpl.hpp`.
 */

#include "JobImpl.hpp"
#include <stdexcept>

namespace kotlinx {
namespace coroutines {

// JobImpl implementation

JobImpl::JobImpl(bool active) : JobSupport(active) {
}

JobImpl::JobImpl(struct Job* parent) : JobSupport(true) {
    if (parent) {
        // Initialize parent-child relationship
        auto parent_job = std::shared_ptr<struct Job>(parent, [](struct Job*){}); // Non-owning shared_ptr
        auto child_handle = parent_job->attach_child(shared_from_this());
        _parent_handle.store(child_handle.get(), std::memory_order_release);
    }
}

JobImpl::~JobImpl() = default;

bool JobImpl::complete() {
    // Simplified implementation for completable jobs
    // In the full implementation, this would call JobSupport's makeCompleting
    // For now, just return true to indicate the job can be completed
    return !is_completed();
}

bool JobImpl::complete_exceptionally(Throwable* exception) {
    // Convert Throwable to exception_ptr
    std::exception_ptr ep;
    if (exception) {
        // For now, create a runtime_error. In a full implementation,
        // this would properly convert the Throwable
        try {
            throw std::runtime_error("Job completed exceptionally");
        } catch (...) {
            ep = std::current_exception();
        }
    } else {
        // Create a generic cancellation exception
        try {
            throw std::runtime_error("Job cancelled");
        } catch (...) {
            ep = std::current_exception();
        }
    }

    // Cancel the job with the exception
    cancel(ep);
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