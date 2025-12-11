#pragma once
#include "kotlinx/coroutines/sync/Semaphore.hpp"
#include <atomic>
#include <stdexcept>
#include <thread>
#include <chrono> // Fix namespace issue

namespace kotlinx {
namespace coroutines {
namespace sync {

class SemaphoreImpl : public Semaphore {
protected:
    int permits;
    std::atomic<int> available;

public:
    SemaphoreImpl(int permits, int acquired_permits) 
        : permits(permits), available(permits - acquired_permits) {
        if (permits <= 0) throw std::invalid_argument("Semaphore permits must be > 0");
    }

    int get_available_permits() const override {
        return available.load();
    }

    void acquire() override {
        /*
         * TODO: STUB - Semaphore acquire uses busy-wait instead of suspension
         *
         * Kotlin source: Semaphore.acquire() in Semaphore.kt
         *
         * What's missing:
         * - Should be a suspend function: suspend fun acquire()
         * - When no permits available, should suspend using suspendCancellableCoroutine
         * - Resume when release() is called and permits become available
         * - Requires: waiters queue to track suspended acquirers
         * - Requires: proper Continuation<void*>* parameter (Kotlin-style suspend)
         * - Should support fairness (FIFO ordering of waiters)
         *
         * Current behavior: Busy-waits (spins) with yield() until permit available
         *   - Burns CPU cycles
         *   - Not coroutine-friendly (blocks the thread)
         * Correct behavior: Suspend coroutine, allowing other coroutines to run
         *
         * Dependencies:
         * - suspendCancellableCoroutine integration
         * - Waiters queue (similar to channel senders/receivers queues)
         * - Atomic state management for lock-free acquire/release coordination
         *
         * Impact: High - current implementation blocks thread instead of suspending
         *
         * Workaround: Use in non-coroutine context where blocking is acceptable,
         *   or use try_acquire() with manual retry logic
         */
        int expected;
        do {
            expected = available.load();
            while (expected <= 0) {
                 // busy wait
                 std::this_thread::yield();
                 expected = available.load();
            }
        } while (!available.compare_exchange_weak(expected, expected - 1));
    }

    bool try_acquire() override {
        int expected = available.load();
        if (expected <= 0) return false;
        return available.compare_exchange_strong(expected, expected - 1);
    }

    void release() override {
        available.fetch_add(1);
    }
};

} // namespace sync
} // namespace coroutines
} // namespace kotlinx
