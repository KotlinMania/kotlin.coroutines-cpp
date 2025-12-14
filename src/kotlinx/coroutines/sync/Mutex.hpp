#pragma once
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

} // namespace sync
} // namespace coroutines
} // namespace kotlinx
