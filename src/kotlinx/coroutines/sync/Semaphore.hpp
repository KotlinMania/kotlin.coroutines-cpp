#pragma once
// port-lint: source sync/Semaphore.kt
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
#include <thread>
#include <algorithm>
#include <stdexcept>
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

// ============================================================================
// SemaphoreImpl - Line 90-353: Concrete implementation of Semaphore
// ============================================================================

/**
 * Line 90-353: SemaphoreAndMutexImpl base (simplified for C++)
 *
 * The queue of waiting acquirers uses a segment-based approach in Kotlin.
 * This C++ implementation uses a simpler atomic counter approach for
 * correctness, with spin-wait for suspension (TODO: proper coroutine integration).
 */
class SemaphoreImpl : public Semaphore {
public:
    /**
     * Creates a new SemaphoreImpl.
     * @param permits The maximum number of permits
     * @param acquired_permits Initial number of permits already acquired
     */
    SemaphoreImpl(int permits, int acquired_permits = 0)
        : permits_(permits)
        , available_permits_(permits - acquired_permits)
    {
        if (permits <= 0) {
            throw std::invalid_argument(
                "Semaphore should have at least 1 permit");
        }
        if (acquired_permits < 0 || acquired_permits > permits) {
            throw std::invalid_argument(
                "The number of acquired permits should be in 0..permits");
        }
    }

    ~SemaphoreImpl() override = default;

    // Line 147: availablePermits property
    int available_permits() const override {
        int val = available_permits_.load(std::memory_order_acquire);
        return std::max(val, 0);
    }

    /**
     * Line 151-168: tryAcquire
     *
     * Tries to acquire a permit without suspension.
     * @return true if permit acquired, false otherwise
     */
    bool try_acquire() override {
        while (true) {
            // Get the current number of available permits
            int p = available_permits_.load(std::memory_order_acquire);

            // Is the number of available permits greater than the maximal one
            // because of an incorrect release() call without preceding acquire()?
            // Change it to permits and start from the beginning.
            if (p > permits_) {
                coerce_available_permits_at_maximum();
                continue;
            }

            // Try to decrement the number of available permits if > 0
            if (p <= 0) return false;

            if (available_permits_.compare_exchange_weak(
                    p, p - 1,
                    std::memory_order_acquire,
                    std::memory_order_relaxed)) {
                return true;
            }
        }
    }

    /**
     * Line 170-180: acquire (suspending version)
     *
     * Suspending function - decrements permits and suspends if needed.
     */
    void* acquire(Continuation<void*>* cont) override {
        // Decrement the number of available permits
        int p = dec_permits();
        // Is the permit acquired?
        if (p > 0) return nullptr; // permit acquired (Unit)

        // Need to suspend - use spin-wait fallback
        // TODO: Implement proper segment-based queue with coroutine suspension
        while (!try_acquire()) {
            std::this_thread::yield();
        }
        return nullptr;
    }

    /**
     * Line 242-262: release
     *
     * Releases a permit back to the semaphore.
     */
    void release() override {
        while (true) {
            // Increment the number of available permits
            int p = available_permits_.fetch_add(1, std::memory_order_release);

            // Does this release call exceed the maximal number of permits?
            if (p >= permits_) {
                // Revert and throw error
                coerce_available_permits_at_maximum();
                throw std::logic_error(
                    "The number of released permits cannot be greater than the max");
            }

            // Is there a waiter that should be resumed?
            if (p >= 0) return;

            // Try to resume first waiter
            // TODO: Implement segment-based waiter queue
            // For now, the permit is available for next tryAcquire
            return;
        }
    }

private:
    const int permits_;
    std::atomic<int> available_permits_;

    /**
     * Line 229-240: decPermits
     *
     * Decrements the number of available permits and ensures it is not
     * greater than permits at the point of decrement.
     */
    int dec_permits() {
        while (true) {
            // Decrement the number of available permits
            int p = available_permits_.fetch_sub(1, std::memory_order_acquire);

            // Is the number > max due to incorrect release()?
            if (p > permits_) continue;

            // The number of permits is correct, return it
            return p;
        }
    }

    /**
     * Line 269-275: coerceAvailablePermitsAtMaximum
     *
     * Changes the number of available permits to permits if it became
     * greater due to incorrect release() call.
     */
    void coerce_available_permits_at_maximum() {
        while (true) {
            int cur = available_permits_.load(std::memory_order_relaxed);
            if (cur <= permits_) break;
            if (available_permits_.compare_exchange_weak(
                    cur, permits_,
                    std::memory_order_release,
                    std::memory_order_relaxed)) {
                break;
            }
        }
    }
};

// ============================================================================
// Factory functions
// ============================================================================

inline std::shared_ptr<Semaphore> create_semaphore(int permits, int acquired_permits) {
    return std::make_shared<SemaphoreImpl>(permits, acquired_permits);
}

} // namespace sync
} // namespace coroutines
} // namespace kotlinx

