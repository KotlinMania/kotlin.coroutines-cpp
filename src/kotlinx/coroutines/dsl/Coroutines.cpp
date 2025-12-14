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
// Delay functions - delegate to kotlinx::coroutines::delay()
// =============================================================================

void* delay(long long time_millis, std::shared_ptr<Continuation<void*>> cont) {
    return kotlinx::coroutines::delay(time_millis, std::move(cont));
}

void* delay(std::chrono::milliseconds duration, std::shared_ptr<Continuation<void*>> cont) {
    return kotlinx::coroutines::delay(duration, std::move(cont));
}

void* delay(std::chrono::nanoseconds duration, std::shared_ptr<Continuation<void*>> cont) {
    return kotlinx::coroutines::delay(duration, std::move(cont));
}

// =============================================================================
// Yield - delegate to kotlinx::coroutines::yield()
// =============================================================================

void* yield(std::shared_ptr<Continuation<void*>> cont) {
    return kotlinx::coroutines::yield(std::move(cont));
}

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
// Mutex operations - TODO(port): implement with suspend_cancellable_coroutine
// =============================================================================

void* lock(sync::Mutex& mutex, std::shared_ptr<Continuation<void*>> cont) {
    // TODO(port): Implement using suspend_cancellable_coroutine
    // For now, blocking acquire
    mutex.lock();
    return nullptr;
}

// =============================================================================
// Semaphore operations - TODO(port): implement with suspend_cancellable_coroutine
// =============================================================================

void* acquire(sync::Semaphore& semaphore, std::shared_ptr<Continuation<void*>> cont) {
    // TODO(port): Implement using suspend_cancellable_coroutine
    // For now, blocking acquire
    semaphore.acquire();
    return nullptr;
}

} // namespace dsl
} // namespace coroutines
} // namespace kotlinx
