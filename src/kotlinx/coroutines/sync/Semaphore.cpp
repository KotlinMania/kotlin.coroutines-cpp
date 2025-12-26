/**
 * @file Semaphore.cpp
 * @brief Semaphore implementation using lock-free segment queue.
 *
 * Transliterated from: kotlinx-coroutines-core/common/src/sync/Semaphore.kt
 *
 * Contains the private SemaphoreImpl class (lines 355-357 in Kotlin).
 */

#include "kotlinx/coroutines/sync/Semaphore.hpp"
#include "kotlinx/coroutines/sync/SemaphoreAndMutexImpl.hpp"
#include <memory>

namespace kotlinx {
namespace coroutines {
namespace sync {

/**
 * Line 355-357: SemaphoreImpl
 *
 * private class SemaphoreImpl(
 *     permits: Int, acquiredPermits: Int
 * ): SemaphoreAndMutexImpl(permits, acquiredPermits), Semaphore
 *
 * This private implementation class inherits from both the implementation
 * base and the public interface to provide the complete Semaphore functionality.
 */
class SemaphoreImpl : public SemaphoreAndMutexImpl, public Semaphore {
public:
    SemaphoreImpl(int permits, int acquired_permits)
        : SemaphoreAndMutexImpl(permits, acquired_permits)
    {}

    int available_permits() const override {
        return SemaphoreAndMutexImpl::available_permits();
    }

    void* acquire(Continuation<void*>* cont) override {
        return SemaphoreAndMutexImpl::acquire(cont);
    }

    bool try_acquire() override {
        return SemaphoreAndMutexImpl::try_acquire();
    }

    void release() override {
        SemaphoreAndMutexImpl::release();
    }
};

std::shared_ptr<Semaphore> create_semaphore(int permits, int acquired_permits) {
    return std::make_shared<SemaphoreImpl>(permits, acquired_permits);
}

} // namespace sync
} // namespace coroutines
} // namespace kotlinx
