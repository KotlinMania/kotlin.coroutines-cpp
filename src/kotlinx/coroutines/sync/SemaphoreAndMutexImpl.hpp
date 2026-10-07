#pragma once
/**
 * @file SemaphoreAndMutexImpl.hpp
 * @brief Shared implementation base for Semaphore and Mutex.
 *
 * Transliterated from: kotlinx-coroutines-core/common/src/sync/Semaphore.kt
 * Lines 90-353
 *
 * This is a lock-free implementation using segment-based queues for waiting
 * acquirers, following the Michael-Scott queue algorithm with modifications.
 */

#include <atomic>
#include <memory>
#include <functional>
#include <stdexcept>
#include <algorithm>
#include <cassert>

#include "kotlinx/coroutines/sync/SemaphoreSegment.hpp"
#include "kotlinx/coroutines/CancellableContinuation.hpp"
#include "kotlinx/coroutines/internal/ConcurrentLinkedList.hpp"



namespace kotlinx {
namespace coroutines {

// Forward declaration
namespace selects {
    class SelectInstanceBase;
}

namespace sync {


/**
 * Line 90-353: SemaphoreAndMutexImpl
 *
 * The queue of waiting acquirers is essentially an infinite array based on
 * the list of segments (see SemaphoreSegment); each segment contains a fixed
 * number of slots.
 *
 * State machine for cells:
 *
 *   +------+ `acquire` suspends   +------+   `release` tries    +--------+
 *   | NULL | -------------------> | cont | -------------------> | PERMIT | (cont RETRIEVED)
 *   +------+                      +------+   to resume `cont`   +--------+
 *      |                             |
 *      |                             | `acquire` cancelled, continuation replaced with CANCEL
 *      | `release` comes             V
 *      | before `acquire`      +-----------+   `release` has    +--------+
 *      | and puts permit       | CANCELLED | -----------------> | PERMIT | (RELEASE FAILED)
 *      |                       +-----------+        failed      +--------+
 *      |
 *      |           `acquire` gets   +-------+
 *      |        +-----------------> | TAKEN | (ELIMINATION HAPPENED)
 *      V        |    the permit     +-------+
 *  +--------+   |
 *  | PERMIT | -<
 *  +--------+  |
 *              |  `release` waited bounded time,   +--------+
 *              +---------------------------------> | BROKEN | (BOTH FAILED)
 *                     but `acquire` has not come   +--------+
 */
// Transliterated from: kotlinx-coroutines-core/common/src/sync/Semaphore.kt:90-353
class SemaphoreAndMutexImpl {
protected:
    std::atomic<SemaphoreSegment*> head_;
    std::atomic<long> deq_idx_{0};
    std::atomic<SemaphoreSegment*> tail_;
    std::atomic<long> enq_idx_{0};

    const int permits_;

    std::atomic<int> available_permits_;

    // Stored as member for use in resume callbacks
    std::function<void(std::exception_ptr, void*, std::shared_ptr<CoroutineContext>)> on_cancellation_release_;

public:
    /**
     * Line 90, 131-137: Constructor
     */
    // Transliterated from: kotlinx-coroutines-core/common/src/sync/Semaphore.kt:90-149
    SemaphoreAndMutexImpl(int permits, int acquired_permits);

    virtual ~SemaphoreAndMutexImpl() {
        // Segment cleanup managed by remove() calls during operation
    }

    // Transliterated from: kotlinx-coroutines-core/common/src/sync/Semaphore.kt:147-147
    int available_permits() const;

    /**
     * Line 151-168: tryAcquire
     */
    // Transliterated from: kotlinx-coroutines-core/common/src/sync/Semaphore.kt:151-168
    bool try_acquire();

    /**
     * Line 170-180: acquire (suspend function)
     *
     * This is the suspend entry point. Returns COROUTINE_SUSPENDED or nullptr.
     */
    // Transliterated from: kotlinx-coroutines-core/common/src/sync/Semaphore.kt:170-180
    void* acquire(Continuation<void*>* cont);

    /**
     * Line 242-262: release
     */
    // Transliterated from: kotlinx-coroutines-core/common/src/sync/Semaphore.kt:242-262
    void release();

protected:
    /**
     * Line 192-196: acquire(waiter: CancellableContinuation<Unit>)
     *
     * For use by subclasses (MutexImpl)
     */
    // Transliterated from: kotlinx-coroutines-core/common/src/sync/Semaphore.kt:192-196
    void acquire_waiter(CancellableContinuation<void>* waiter);

    /**
     * Line 215-220: onAcquireRegFunction (for select)
     *
     * Called during select registration phase. Implements acquire semantics
     * for select clause on Semaphore/Mutex.
     */
    // Transliterated from: kotlinx-coroutines-core/common/src/sync/Semaphore.kt:215-220
    void on_acquire_reg_function(selects::SelectInstanceBase* select, void*);

private:
    /**
     * Line 182-189: acquireSlowPath
     *
     * suspendCancellableCoroutineReusable<Unit> { cont -> ... }
     */
    // Transliterated from: kotlinx-coroutines-core/common/src/sync/Semaphore.kt:182-189
    void* acquire_slow_path(Continuation<void*>* cont);

    /**
     * Line 199-211: acquire internal loop
     */
    // Transliterated from: kotlinx-coroutines-core/common/src/sync/Semaphore.kt:199-211
    void acquire_internal(Waiter* waiter, std::function<bool(Waiter*)> suspend_func,
                          std::function<void(Waiter*)> on_acquired);

    /**
     * Line 229-240: decPermits
     *
     * Decrements the number of available permits and ensures it is not
     * greater than permits at the point of decrement.
     */
    // Transliterated from: kotlinx-coroutines-core/common/src/sync/Semaphore.kt:229-240
    int dec_permits();

    /**
     * Line 269-275: coerceAvailablePermitsAtMaximum
     *
     * Changes the number of available permits to permits if it became
     * greater due to an incorrect release() call.
     */
    // Transliterated from: kotlinx-coroutines-core/common/src/sync/Semaphore.kt:269-275
    void coerce_available_permits_at_maximum();

    /**
     * Line 280-310: addAcquireToQueue
     *
     * Returns false if the received permit cannot be used and the calling
     * operation should restart.
     */
    // Transliterated from: kotlinx-coroutines-core/common/src/sync/Semaphore.kt:280-310
    bool add_acquire_to_queue(Waiter* waiter);

    /**
     * Line 313-337: tryResumeNextFromQueue
     */
    // Transliterated from: kotlinx-coroutines-core/common/src/sync/Semaphore.kt:313-337
    bool try_resume_next_from_queue();

    /**
     * Line 339-352: tryResumeAcquire
     *
     * Try to resume a waiter that was stored in the cell.
     */
    // Transliterated from: kotlinx-coroutines-core/common/src/sync/Semaphore.kt:339-352
    bool try_resume_acquire(Waiter* waiter);

    // Transliterated from: kotlinx-coroutines-core/common/src/sync/Semaphore.kt:296-305
    void resume_waiter_with_permit(Waiter* waiter);

};

} // namespace sync
} // namespace coroutines
} // namespace kotlinx
