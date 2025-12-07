/**
 * @file Semaphore.cpp
 * @brief Implementation of Semaphore.
 *
 * NOTE: The detailed API documentation, KDocs, and class definitions are located
 * in the companion header file: `include/kotlinx/coroutines/sync/Semaphore.hpp`.
 */

#include "kotlinx/coroutines/sync/Semaphore.hpp"
#include <stdexcept>
#include <iostream>

namespace kotlinx {
namespace coroutines {
namespace sync {

class SemaphoreImpl : public Semaphore {
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
        // Simple spin-wait stub for now, TODO: implement true suspension
        int expected;
        do {
            expected = available.load();
            while (expected <= 0) {
                 // busy wait
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

Semaphore* create_semaphore(int permits, int acquired_permits) {
    return new SemaphoreImpl(permits, acquired_permits);
}

} // namespace sync
} // namespace coroutines
} // namespace kotlinx
