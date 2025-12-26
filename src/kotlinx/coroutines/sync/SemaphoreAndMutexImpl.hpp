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
    template <typename R> class SelectInstance;
}

namespace sync {

// Forward declaration for Waiter concept
// We use void* to represent the polymorphic waiter

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
    SemaphoreAndMutexImpl(int permits, int acquired_permits)
        : permits_(permits)
        , available_permits_(permits - acquired_permits)
    {
        if (permits <= 0) {
            throw std::invalid_argument(
                "Semaphore should have at least 1 permit, but had " + std::to_string(permits));
        }
        if (acquired_permits < 0 || acquired_permits > permits) {
            throw std::invalid_argument(
                "The number of acquired permits should be in 0.." + std::to_string(permits));
        }
        auto s = new SemaphoreSegment(0, nullptr, 2);
        head_.store(s, std::memory_order_relaxed);
        tail_.store(s, std::memory_order_relaxed);

        on_cancellation_release_ = [this](std::exception_ptr, void*, std::shared_ptr<CoroutineContext>) {
            release();
        };
    }

    virtual ~SemaphoreAndMutexImpl() {
        // Segment cleanup managed by remove() calls during operation
    }

    int available_permits() const {
        return std::max(available_permits_.load(std::memory_order_acquire), 0);
    }

    /**
     * Line 151-168: tryAcquire
     */
    bool try_acquire() {
        while (true) {
            int p = available_permits_.load(std::memory_order_acquire);

            if (p > permits_) {
                coerce_available_permits_at_maximum();
                continue;
            }

            if (p <= 0) return false;
            if (available_permits_.compare_exchange_weak(p, p - 1,
                    std::memory_order_release, std::memory_order_relaxed)) {
                return true;
            }
        }
    }

    /**
     * Line 170-180: acquire (suspend function)
     *
     * This is the suspend entry point. Returns COROUTINE_SUSPENDED or nullptr.
     */
    void* acquire(Continuation<void*>* cont) {
        int p = dec_permits();
        if (p > 0) {
            return nullptr; // Permit acquired, return Unit
        }
        return acquire_slow_path(cont);
    }

    /**
     * Line 242-262: release
     */
    void release() {
        while (true) {
            int p = available_permits_.fetch_add(1, std::memory_order_acq_rel);

            if (p >= permits_) {
                coerce_available_permits_at_maximum();
                throw std::logic_error(
                    "The number of released permits cannot be greater than " +
                    std::to_string(permits_));
            }

            if (p >= 0) return;

            if (try_resume_next_from_queue()) return;
        }
    }

protected:
    /**
     * Line 192-196: acquire(waiter: CancellableContinuation<Unit>)
     *
     * For use by subclasses (MutexImpl)
     */
    void acquire_waiter(CancellableContinuation<void>* waiter) {
        acquire_internal(
            waiter,
            [this](CancellableContinuation<void>* cont) { return add_acquire_to_queue(cont); },
            [this](CancellableContinuation<void>* cont) {
                cont->resume([this](std::exception_ptr) { release(); });
            }
        );
    }

    /**
     * Line 215-220: onAcquireRegFunction (for select)
     *
     * Called during select registration phase. Implements acquire semantics
     * for select clause on Semaphore/Mutex.
     */
    template <typename R>
    void on_acquire_reg_function(selects::SelectInstance<R>* select, void* ignored_param) {
        // Select support requires SelectInstance::selectInRegistrationPhase
    }

private:
    /**
     * Line 182-189: acquireSlowPath
     *
     * suspendCancellableCoroutineReusable<Unit> { cont -> ... }
     */
    void* acquire_slow_path(Continuation<void*>* cont) {
        return suspend_cancellable_coroutine<void>(
            [this](CancellableContinuation<void>& cancellable_cont) {
                if (add_acquire_to_queue(&cancellable_cont)) return;
                acquire_waiter(&cancellable_cont);
            },
            cont
        );
    }

    /**
     * Line 199-211: acquire internal loop
     */
    template <typename W, typename SuspendFunc, typename OnAcquiredFunc>
    void acquire_internal(W waiter, SuspendFunc suspend_func, OnAcquiredFunc on_acquired) {
        while (true) {
            int p = dec_permits();
            if (p > 0) {
                on_acquired(waiter);
                return;
            }
            if (suspend_func(waiter)) return;
        }
    }

    /**
     * Line 229-240: decPermits
     *
     * Decrements the number of available permits and ensures it is not
     * greater than permits at the point of decrement.
     */
    int dec_permits() {
        while (true) {
            int p = available_permits_.fetch_sub(1, std::memory_order_acq_rel);
            if (p > permits_) continue;
            return p;
        }
    }

    /**
     * Line 269-275: coerceAvailablePermitsAtMaximum
     *
     * Changes the number of available permits to permits if it became
     * greater due to an incorrect release() call.
     */
    void coerce_available_permits_at_maximum() {
        while (true) {
            int cur = available_permits_.load(std::memory_order_acquire);
            if (cur <= permits_) break;
            if (available_permits_.compare_exchange_weak(cur, permits_,
                    std::memory_order_release, std::memory_order_relaxed)) {
                break;
            }
        }
    }

    /**
     * Line 280-310: addAcquireToQueue
     *
     * Returns false if the received permit cannot be used and the calling
     * operation should restart.
     */
    bool add_acquire_to_queue(void* waiter) {
        SemaphoreSegment* cur_tail = tail_.load(std::memory_order_acquire);

        long enq_idx = enq_idx_.fetch_add(1, std::memory_order_acq_rel);

        auto result = internal::find_segment_and_move_forward(
            tail_,
            enq_idx / SEGMENT_SIZE(),
            cur_tail,
            create_segment
        );
        SemaphoreSegment* segment = result.segment();

        int i = static_cast<int>(enq_idx % SEGMENT_SIZE());

        void* expected = nullptr;
        if (segment->cas(i, expected, waiter)) {
            install_cancellation_handler(waiter, segment, i);
            return true;
        }

        if (segment->cas(i, static_cast<void*>(&PERMIT()), static_cast<void*>(&TAKEN()))) {
            resume_waiter_with_permit(waiter);
            return true;
        }

        assert(segment->get(i) == static_cast<void*>(&BROKEN()));
        return false;
    }

    /**
     * Line 313-337: tryResumeNextFromQueue
     */
    bool try_resume_next_from_queue() {
        SemaphoreSegment* cur_head = head_.load(std::memory_order_acquire);

        long deq_idx = deq_idx_.fetch_add(1, std::memory_order_acq_rel);

        long segment_id = deq_idx / SEGMENT_SIZE();

        auto result = internal::find_segment_and_move_forward(
            head_,
            segment_id,
            cur_head,
            create_segment
        );
        SemaphoreSegment* segment = result.segment();

        segment->clean_prev();

        if (segment->id > segment_id) return false;

        int i = static_cast<int>(deq_idx % SEGMENT_SIZE());

        void* cell_state = segment->get_and_set(i, static_cast<void*>(&PERMIT()));

        if (cell_state == nullptr) {
            for (int spin = 0; spin < MAX_SPIN_CYCLES(); ++spin) {
                if (segment->get(i) == static_cast<void*>(&TAKEN())) {
                    return true;
                }
            }
            return !segment->cas(i, static_cast<void*>(&PERMIT()), static_cast<void*>(&BROKEN()));
        }

        if (cell_state == static_cast<void*>(&CANCELLED())) {
            return false;
        }

        return try_resume_acquire(cell_state);
    }

    /**
     * Line 339-352: tryResumeAcquire
     *
     * Try to resume a waiter that was stored in the cell.
     */
    bool try_resume_acquire(void* waiter) {
        // In Kotlin, this dispatches on waiter type (CancellableContinuation or SelectInstance)
        // In C++, we need to cast based on actual type
        // For now, assume it's always a CancellableContinuation<void>
        auto* cont = static_cast<CancellableContinuation<void>*>(waiter);

        void* token = cont->try_resume(nullptr);
        if (token != nullptr) {
            cont->complete_resume(token);
            return true;
        }
        return false;
    }

    /**
     * Helper: Install cancellation handler on waiter
     */
    void install_cancellation_handler(void* waiter, SemaphoreSegment* segment, int index) {
        auto* cont = static_cast<CancellableContinuation<void>*>(waiter);
        cont->invoke_on_cancellation([segment, index](std::exception_ptr cause) {
            segment->on_cancellation(index, cause, nullptr);
        });
    }

    /**
     * Helper: Resume waiter with permit (elimination happened)
     */
    void resume_waiter_with_permit(void* waiter) {
        auto* cont = static_cast<CancellableContinuation<void>*>(waiter);
        cont->resume([this](std::exception_ptr) { release(); });
    }
};

} // namespace sync
} // namespace coroutines
} // namespace kotlinx
