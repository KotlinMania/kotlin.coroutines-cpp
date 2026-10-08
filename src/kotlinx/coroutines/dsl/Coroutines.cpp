/**
 * @file Coroutines.cpp
 * @brief Implementation of DSL suspend function wrappers.
 */

#include "kotlinx/coroutines/dsl/Coroutines.hpp"
#include "kotlinx/coroutines/Delay.hpp"
#include "kotlinx/coroutines/Yield.hpp"
#include "kotlinx/coroutines/Job.hpp"
#include "kotlinx/coroutines/sync/Mutex.hpp"
#include "kotlinx/coroutines/sync/Semaphore.hpp"
#include "kotlinx/coroutines/intrinsics/Intrinsics.hpp"

namespace kotlinx {
namespace coroutines {
namespace dsl {

// =============================================================================
// Job operations
// =============================================================================

void* join(Job& job, std::shared_ptr<Continuation<void*>> cont) {
    return job.join(cont.get());
}

void* join(std::shared_ptr<Job> job, std::shared_ptr<Continuation<void*>> cont) {
    if (!job) {
        return nullptr;  // No job to join
    }
    return job->join(cont.get());
}

// =============================================================================
// Mutex operations
// =============================================================================

void* lock(sync::Mutex& mutex, std::shared_ptr<Continuation<void*>> cont) {
    // MutexImpl::lock_suspend delegates to suspend_cancellable_coroutine
    mutex.lock();
    return nullptr;
}

// =============================================================================
// Semaphore operations
// =============================================================================

void* acquire(sync::Semaphore& semaphore, std::shared_ptr<Continuation<void*>> cont) {
    return semaphore.acquire(cont.get());
}

} // namespace dsl
} // namespace coroutines
} // namespace kotlinx
