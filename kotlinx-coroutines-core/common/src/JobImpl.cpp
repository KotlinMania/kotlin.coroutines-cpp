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

// JobSupport provides the key() implementation

// JobImpl-specific implementation
bool JobImpl::handles_exception() const {
    // Check if parent handles exceptions (similar to Kotlin implementation)
    // For now, return true (default behavior)
    // TODO: Implement the full parent chain checking logic
    return true;
}

bool JobImpl::complete_exceptionally(Throwable* exception) {
    // For now, create a simple exception wrapper
    // In the full implementation, this would create CompletedExceptionally
    std::exception_ptr ep;
    if (exception) {
        try {
            throw std::runtime_error("Job completed exceptionally");
        } catch (...) {
            ep = std::current_exception();
        }
    }

    // Call makeCompleting with CompletedExceptionally
    // For now, we'll just pass the exception pointer
    return make_completing(&ep);
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