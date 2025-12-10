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
        // Simple spin-wait stub for now, TODO: implement true suspension
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
