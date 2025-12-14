// Kotlin source: kotlinx-coroutines-core/common/src/sync/Semaphore.kt
#include "kotlinx/coroutines/sync/Semaphore.hpp"
#include "kotlinx/coroutines/Continuation.hpp"
#include <atomic>
#include <stdexcept>

namespace kotlinx {
namespace coroutines {
namespace sync {

// Simple implementation of Semaphore
class SemaphoreImpl : public Semaphore {
public:
    SemaphoreImpl(int permits, int acquired_permits)
        : available_permits_(permits - acquired_permits) {
        if (permits < 0) {
            throw std::invalid_argument("Semaphore permits must be non-negative");
        }
        if (acquired_permits < 0 || acquired_permits > permits) {
            throw std::invalid_argument("acquired_permits must be between 0 and permits");
        }
    }

    int available_permits() const override {
        return available_permits_.load(std::memory_order_acquire);
    }

    void* acquire(Continuation<void*>* cont) override {
        // Try to acquire immediately
        if (try_acquire()) {
            return nullptr;  // Success, no suspension
        }
        // Would need to suspend - for now throw
        (void)cont;
        throw std::logic_error("Semaphore::acquire suspend path not implemented");
    }

    bool try_acquire() override {
        int current = available_permits_.load(std::memory_order_acquire);
        while (current > 0) {
            if (available_permits_.compare_exchange_weak(current, current - 1,
                    std::memory_order_acq_rel, std::memory_order_acquire)) {
                return true;
            }
        }
        return false;
    }

    void release() override {
        available_permits_.fetch_add(1, std::memory_order_release);
    }

private:
    std::atomic<int> available_permits_;
};

std::shared_ptr<Semaphore> create_semaphore(int permits, int acquired_permits) {
    return std::make_shared<SemaphoreImpl>(permits, acquired_permits);
}

} // namespace sync
} // namespace coroutines
} // namespace kotlinx
