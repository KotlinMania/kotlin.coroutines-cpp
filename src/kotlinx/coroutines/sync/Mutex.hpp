#pragma once
// port-lint: source sync/Mutex.kt
/**
 * @file Mutex.hpp
 * @brief Mutual exclusion for coroutines.
 *
 * Transliterated from: kotlinx-coroutines-core/common/src/sync/Mutex.kt
 * Lines 11-127 (interface and factory function)
 */

#include "kotlinx/coroutines/selects/Select.hpp"
#include <functional>
#include <memory>
#include <atomic>
#include <thread>
#include <cassert>
#include <string>
#include <stdexcept>

namespace kotlinx {
namespace coroutines {
namespace sync {

class Mutex; // Forward declaration

/**
 * Line 11-96: Mutex interface
 *
 * Mutual exclusion for coroutines.
 *
 * Mutex has two states: _locked_ and _unlocked_.
 * It is **non-reentrant**, that is invoking lock() even from the same thread/coroutine
 * that currently holds the lock still suspends the invoker.
 *
 * JVM API note:
 * Memory semantic of the Mutex is similar to `synchronized` block on JVM:
 * An unlock operation on a Mutex happens-before every subsequent successful lock on that Mutex.
 * Unsuccessful call to try_lock() do not have any memory effects.
 */
class Mutex {
public:
    virtual ~Mutex() = default;

    /**
     * Line 27: Returns true if this mutex is locked.
     */
    virtual bool is_locked() const = 0;

    /**
     * Line 30-39: Tries to lock this mutex, returning false if already locked.
     *
     * It is recommended to use with_lock() for safety reasons.
     *
     * @param owner Optional owner token for debugging. When owner is specified
     *        and this mutex is already locked with the same token (same identity),
     *        this function throws std::logic_error.
     */
    virtual bool try_lock(void* owner = nullptr) = 0;

    /**
     * Line 42-65: Locks this mutex, suspending caller until the lock is acquired.
     *
     * This suspending function is cancellable: if the Job of the current coroutine
     * is cancelled while waiting, this function immediately resumes with CancellationException.
     *
     * There is a **prompt cancellation guarantee**: even if ready to return but cancelled
     * while suspended, CancellationException will be thrown.
     *
     * This function is fair; suspended callers are resumed in first-in-first-out order.
     *
     * @param owner Optional owner token for debugging.
     */
    virtual void lock(void* owner = nullptr) = 0;

    /**
     * Line 77-82: Checks whether this mutex is locked by the specified owner.
     *
     * @return true when locked by the specified owner; false otherwise.
     */
    virtual bool holds_lock(void* owner) = 0;

    /**
     * Line 85-95: Unlocks this mutex.
     *
     * Throws std::logic_error if invoked on a mutex that is not locked or
     * was locked with a different owner token (by identity).
     *
     * @param owner Optional owner token for debugging.
     */
    virtual void unlock(void* owner = nullptr) = 0;

    /**
     * Line 68-74: Clause for select expression of lock suspending function.
     *
     * @Deprecated - Mutex.onLock deprecated without replacement.
     * For additional details please refer to kotlinx.coroutines #2794.
     */
    virtual selects::SelectClause2<void*, Mutex*>& get_on_lock() = 0;
};

/**
 * Line 105-106: Factory function
 *
 * Creates a Mutex instance.
 * The mutex created is fair: lock is granted in first come, first served order.
 *
 * @param locked initial state of the mutex.
 */
Mutex* create_mutex(bool locked = false);

// shared_ptr version
std::shared_ptr<Mutex> make_mutex(bool locked = false);

/**
 * Line 117-127: with_lock
 *
 * Executes the given action under this mutex's lock.
 *
 * @param owner Optional owner token for debugging.
 * @return the return value of the action.
 */
template<typename T, typename ActionFunc>
T with_lock(Mutex& mutex, void* owner, ActionFunc&& action) {
    mutex.lock(owner);
    try {
        T result = action();
        mutex.unlock(owner);
        return result;
    } catch (...) {
        mutex.unlock(owner);
        throw;
    }
}

// Void specialization
template<typename ActionFunc>
void with_lock_void(Mutex& mutex, void* owner, ActionFunc&& action) {
    mutex.lock(owner);
    try {
        action();
        mutex.unlock(owner);
    } catch (...) {
        mutex.unlock(owner);
        throw;
    }
}

// Convenience overload without owner
template<typename ActionFunc>
void with_lock_void(Mutex& mutex, ActionFunc&& action) {
    with_lock_void(mutex, nullptr, std::forward<ActionFunc>(action));
}

// ============================================================================
// MutexImpl - Line 130-302: Concrete implementation of Mutex
// ============================================================================

// Private constants (Lines 304-313)
namespace detail {
    // TryLock result codes
    constexpr int TRY_LOCK_SUCCESS = 0;
    constexpr int TRY_LOCK_FAILED = 1;
    constexpr int TRY_LOCK_ALREADY_LOCKED_BY_OWNER = 2;

    // HoldsLock result codes
    constexpr int HOLDS_LOCK_UNLOCKED = 0;
    constexpr int HOLDS_LOCK_YES = 1;
    constexpr int HOLDS_LOCK_ANOTHER_OWNER = 2;

    // Sentinel value for "no owner"
    inline void* NO_OWNER() {
        static int sentinel = 0;
        return &sentinel;
    }
}

/**
 * Line 130-302: MutexImpl
 *
 * Concrete implementation of Mutex interface.
 * Implements fair locking with owner tracking for debugging.
 *
 * After the lock is acquired, the corresponding owner is stored in owner_.
 * The unlock operation checks the owner and either re-sets it to NO_OWNER,
 * if there is no waiting request, or to the owner of the suspended lock
 * operation to be resumed, otherwise.
 */
class MutexImpl : public Mutex {
public:
    /**
     * Constructor - creates mutex in specified initial state.
     * @param locked If true, mutex starts in locked state
     */
    explicit MutexImpl(bool locked = false)
        : available_permits_(locked ? 0 : 1)
        , owner_(locked ? nullptr : detail::NO_OWNER())
    {}

    ~MutexImpl() override = default;

    // Line 144-145: isLocked property
    bool is_locked() const override {
        return available_permits_.load(std::memory_order_acquire) == 0;
    }

    // Line 147: holdsLock
    bool holds_lock(void* owner) override {
        return holds_lock_impl(owner) == detail::HOLDS_LOCK_YES;
    }

    // Line 166-169: lock with tryLock fast-path
    void lock(void* owner = nullptr) override {
        if (try_lock(owner)) return;
        lock_suspend(owner);
    }

    // Line 176-181: tryLock
    bool try_lock(void* owner = nullptr) override {
        int result = try_lock_impl(owner);
        switch (result) {
            case detail::TRY_LOCK_SUCCESS:
                return true;
            case detail::TRY_LOCK_FAILED:
                return false;
            case detail::TRY_LOCK_ALREADY_LOCKED_BY_OWNER:
                throw std::logic_error(
                    "This mutex is already locked by the specified owner");
            default:
                throw std::logic_error("unexpected tryLock result");
        }
    }

    // Line 206-221: unlock
    void unlock(void* owner = nullptr) override {
        while (true) {
            // Is this mutex locked?
            if (!is_locked()) {
                throw std::logic_error("This mutex is not locked");
            }
            // Read the owner, waiting until it is set in a spin-loop if required
            void* cur_owner = owner_.load(std::memory_order_acquire);
            if (cur_owner == detail::NO_OWNER()) continue; // spin-wait

            // Check the owner
            if (cur_owner != owner && owner != nullptr) {
                throw std::logic_error(
                    "This mutex is locked by another owner, but different owner expected");
            }

            // Try to clean the owner first using CAS to synchronize
            void* expected = cur_owner;
            if (!owner_.compare_exchange_weak(expected, detail::NO_OWNER(),
                    std::memory_order_release, std::memory_order_relaxed)) {
                continue; // retry
            }

            // Release the semaphore permit at the end
            available_permits_.fetch_add(1, std::memory_order_release);
            return;
        }
    }

    // Line 224-229: onLock select clause (deprecated)
    selects::SelectClause2<void*, Mutex*>& get_on_lock() override {
        // TODO: Implement select clause support
        throw std::runtime_error("onLock is deprecated and not implemented");
    }

    // Line 301: toString
    std::string to_string() const {
        void* cur_owner = owner_.load(std::memory_order_relaxed);
        return std::string("Mutex[isLocked=") +
               (is_locked() ? "true" : "false") +
               ",owner=" +
               (cur_owner == detail::NO_OWNER() ? "NO_OWNER" : "set") + "]";
    }

private:
    // Semaphore-like permit tracking (base class would be SemaphoreAndMutexImpl)
    std::atomic<int> available_permits_;

    // Line 137: Owner tracking for debugging
    std::atomic<void*> owner_;

    /**
     * Line 154-164: holdsLockImpl
     *
     * @return HOLDS_LOCK_UNLOCKED if mutex is unlocked
     *         HOLDS_LOCK_YES if mutex is held with the specified owner
     *         HOLDS_LOCK_ANOTHER_OWNER if mutex is held with a different owner
     */
    int holds_lock_impl(void* owner) {
        while (true) {
            // Is this mutex locked?
            if (!is_locked()) return detail::HOLDS_LOCK_UNLOCKED;

            void* cur_owner = owner_.load(std::memory_order_acquire);
            // Wait in a spin-loop until the owner is set
            if (cur_owner == detail::NO_OWNER()) continue; // spin-wait

            // Check the owner
            return (cur_owner == owner) ?
                   detail::HOLDS_LOCK_YES : detail::HOLDS_LOCK_ANOTHER_OWNER;
        }
    }

    /**
     * Line 183-204: tryLockImpl
     *
     * Attempts to acquire the lock atomically.
     */
    int try_lock_impl(void* owner) {
        while (true) {
            // Try to acquire the semaphore permit
            int expected = 1;
            if (available_permits_.compare_exchange_weak(expected, 0,
                    std::memory_order_acquire, std::memory_order_relaxed)) {
                // Successfully acquired
                assert(owner_.load(std::memory_order_relaxed) == detail::NO_OWNER());
                owner_.store(owner, std::memory_order_release);
                return detail::TRY_LOCK_SUCCESS;
            } else {
                // Permit acquisition failed
                // Check if locked by our owner
                if (owner == nullptr) return detail::TRY_LOCK_FAILED;

                switch (holds_lock_impl(owner)) {
                    case detail::HOLDS_LOCK_YES:
                        return detail::TRY_LOCK_ALREADY_LOCKED_BY_OWNER;
                    case detail::HOLDS_LOCK_ANOTHER_OWNER:
                        return detail::TRY_LOCK_FAILED;
                    case detail::HOLDS_LOCK_UNLOCKED:
                        continue; // retry
                }
            }
        }
    }

    /**
     * Line 171-174: Suspending lock operation
     * TODO: Implement proper coroutine suspension with waiter queue
     */
    void lock_suspend(void* owner) {
        // Spin-wait fallback (proper implementation would use coroutine suspension)
        while (!try_lock(owner)) {
            std::this_thread::yield();
        }
    }
};

// ============================================================================
// Factory functions implementation
// ============================================================================

inline Mutex* create_mutex(bool locked) {
    return new MutexImpl(locked);
}

inline std::shared_ptr<Mutex> make_mutex(bool locked) {
    return std::make_shared<MutexImpl>(locked);
}

} // namespace sync
} // namespace coroutines
} // namespace kotlinx

