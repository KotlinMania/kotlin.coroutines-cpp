#pragma once
#include <atomic>
#include <functional>
#include <memory>
#include <exception>

namespace kotlinx {
namespace coroutines {
namespace sync {

class Semaphore {
public:
    virtual ~Semaphore() = default;
    virtual int get_available_permits() const = 0;
    virtual void acquire() = 0; // suspend
    virtual bool try_acquire() = 0;
    virtual void release() = 0;
};

Semaphore* create_semaphore(int permits, int acquired_permits = 0);

template<typename ActionFunc>
auto with_permit(Semaphore& semaphore, ActionFunc&& action) {
    semaphore.acquire();
    try {
        return action();
    } catch (...) {
        semaphore.release();
        throw;
    }
}

} // namespace sync
} // namespace coroutines
} // namespace kotlinx
