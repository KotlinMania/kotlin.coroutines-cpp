#pragma once
/**
 * @file Semaphore.hpp
 * @brief Counting semaphore for coroutines.
 *
 * Transliterated from: kotlinx-coroutines-core/common/src/sync/Semaphore.kt
 * Lines 12-87 (interface and factory function)
 */

#include <atomic>
#include <functional>
#include <memory>
#include <exception>
#include "kotlinx/coroutines/Continuation.hpp"

namespace kotlinx {
namespace coroutines {
namespace sync {

/**
 * Line 12-59: Semaphore interface
 *
 * A counting semaphore for coroutines that logically maintains a number of
 * available permits. Each acquire() takes a single permit or suspends until
 * it is available. Each release() adds a permit, potentially releasing a
 * suspended acquirer. Semaphore is fair and maintains a FIFO order of acquirers.
 *
 * Semaphores are mostly used to limit the number of coroutines that have
 * access to particular resource. Semaphore with `permits = 1` is essentially
 * a Mutex.
 */
class Semaphore {
public:
    virtual ~Semaphore() = default;

    /**
     * Line 24-25: Returns the current number of permits available in this semaphore.
     */
    virtual int available_permits() const = 0;

    /**
     * Line 28-44: Acquires a permit from this semaphore, suspending until one is available.
     *
     * All suspending acquirers are processed in first-in-first-out (FIFO) order.
     *
     * This suspending function is cancellable: if the Job of the current coroutine
     * is cancelled while this suspending function is waiting, this function
     * immediately resumes with CancellationException.
     *
     * There is a **prompt cancellation guarantee**: even if this function is ready
     * to return the result, but was cancelled while suspended, CancellationException
     * will be thrown.
     *
     * @param cont The continuation for suspend/resume
     * @return COROUTINE_SUSPENDED or nullptr (Unit)
     */
    virtual void* acquire(Continuation<void*>* cont) = 0;

    /**
     * Line 47-51: Tries to acquire a permit from this semaphore without suspension.
     *
     * @return true if a permit was acquired, false otherwise.
     */
    virtual bool try_acquire() = 0;

    /**
     * Line 54-58: Releases a permit, returning it into this semaphore.
     *
     * Resumes the first suspending acquirer if there is one at the point of
     * invocation. Throws std::logic_error if the number of release invocations
     * is greater than the number of preceding acquire.
     */
    virtual void release() = 0;

    // Blocking acquire for non-coroutine contexts
    void acquire() {
        while (!try_acquire()) {
            // Spin-wait fallback
        }
    }
};

/**
 * Line 67-68: Factory function
 *
 * Creates new Semaphore instance.
 * @param permits the number of permits available in this semaphore.
 * @param acquired_permits the number of already acquired permits,
 *        should be between 0 and permits (inclusively).
 */
std::shared_ptr<Semaphore> create_semaphore(int permits, int acquired_permits = 0);

/**
 * Line 77-87: withPermit
 *
 * Executes the given action, acquiring a permit from this semaphore at the
 * beginning and releasing it after the action is completed.
 *
 * @return the return value of the action.
 */
template<typename T, typename ActionFunc>
T with_permit(Semaphore& semaphore, ActionFunc&& action) {
    semaphore.acquire();
    try {
        T result = action();
        semaphore.release();
        return result;
    } catch (...) {
        semaphore.release();
        throw;
    }
}

// Void specialization
template<typename ActionFunc>
void with_permit_void(Semaphore& semaphore, ActionFunc&& action) {
    semaphore.acquire();
    try {
        action();
        semaphore.release();
    } catch (...) {
        semaphore.release();
        throw;
    }
}

} // namespace sync
} // namespace coroutines
} // namespace kotlinx
