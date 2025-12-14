#pragma once
/**
 * @file SemaphoreImpl.hpp
 * @brief Semaphore implementation using lock-free segment queue.
 *
 * Transliterated from: kotlinx-coroutines-core/common/src/sync/Semaphore.kt
 * Lines 355-357
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
 */
class SemaphoreImplFull : public SemaphoreAndMutexImpl, public Semaphore {
public:
    SemaphoreImplFull(int permits, int acquired_permits)
        : SemaphoreAndMutexImpl(permits, acquired_permits)
    {}

    // Line 25: val availablePermits: Int
    int available_permits() const override {
        return SemaphoreAndMutexImpl::available_permits();
    }

    // Line 44: suspend fun acquire()
    void* acquire(Continuation<void*>* cont) override {
        return SemaphoreAndMutexImpl::acquire(cont);
    }

    // Line 51: fun tryAcquire(): Boolean
    bool try_acquire() override {
        return SemaphoreAndMutexImpl::try_acquire();
    }

    // Line 58: fun release()
    void release() override {
        SemaphoreAndMutexImpl::release();
    }
};

// Factory function implementation
inline std::shared_ptr<Semaphore> create_semaphore(int permits, int acquired_permits) {
    return std::make_shared<SemaphoreImplFull>(permits, acquired_permits);
}

} // namespace sync
} // namespace coroutines
} // namespace kotlinx
