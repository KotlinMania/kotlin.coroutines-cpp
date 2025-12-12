#pragma once
#include "kotlinx/coroutines/selects/Select.hpp"
#include <functional>

namespace kotlinx {
namespace coroutines {
namespace sync {

class Mutex; // Forward declaration

/**
 * Mutual exclusion for coroutines.
 *
 * Mutex provides coroutine-aware mutual exclusion with suspending semantics.
 * Unlike traditional mutexes, coroutine mutexes suspend the waiting coroutine
 * instead of blocking the thread, allowing other coroutines to run on the same thread.
 *
 * Mutex has two states: locked and unlocked.
 * It is **non-reentrant**, meaning invoking lock() even from the same thread/coroutine
 * that currently holds the lock still suspends the invoker.
 *
 * Memory semantics:
 * An unlock operation on a Mutex happens-before every subsequent successful lock
 * on that Mutex. Unsuccessful calls to try_lock() do not have any memory effects.
 *
 * Thread safety:
 * All operations are thread-safe and can be called from any thread or coroutine.
 */
class Mutex {
public:
    virtual ~Mutex() = default;

    /**
     * Returns `true` if this mutex is locked.
     */
    virtual bool is_locked() const = 0;

    /**
     * Tries to lock this mutex, returning `false` if this mutex is already locked.
     *
     * It is recommended to use [withLock] for safety reasons, so that the acquired lock is always
     * released at the end of your critical section, and [unlock] is never invoked before a successful
     * lock acquisition.
     *
     * @param owner Optional owner token for debugging. When `owner` is specified (non-nullptr value) and this mutex
     *        is already locked with the same token (same identity), this function throws [IllegalStateException].
     */
    virtual bool try_lock(void* owner = nullptr) = 0;

    /**
     * Locks this mutex, suspending caller until the lock is acquired (in other words, while the lock is held elsewhere).
     *
     * This suspending function is cancellable: if the [Job] of the current coroutine is cancelled while this
     * suspending function is waiting, this function immediately resumes with [CancellationException].
     * There is a **prompt cancellation guarantee**: even if this function is ready to return the result, but was cancelled
     * while suspended, [CancellationException] will be thrown. See [suspendCancellableCoroutine] for low-level details.
     * This function releases the lock if it was already acquired by this function before the [CancellationException]
     * was thrown.
     *
     * Note that this function does not check for cancellation when it is not suspended.
     * Use [yield] or [CoroutineScope.isActive] to periodically check for cancellation in tight loops if needed.
     *
     * Use [tryLock] to try acquiring the lock without waiting.
     *
     * This function is fair; suspended callers are resumed in first-in-first-out order.
     *
     * It is recommended to use [withLock] for safety reasons, so that the acquired lock is always
     * released at the end of the critical section, and [unlock] is never invoked before a successful
     * lock acquisition.
     *
     * @param owner Optional owner token for debugging. When `owner` is specified (non-nullptr value) and this mutex
     *        is already locked with the same token (same identity), this function throws [IllegalStateException].
     */
    virtual void lock(void* owner = nullptr) = 0;

    /**
     * Checks whether this mutex is locked by the specified owner.
     *
     * @return `true` when this mutex is locked by the specified owner;
     * `false` if the mutex is not locked or locked by another owner.
     */
    virtual bool holds_lock(void* owner) = 0;

    /**
     * Unlocks this mutex. Throws [IllegalStateException] if invoked on a mutex that is not locked or
     * was locked with a different owner token (by identity).
     *
     * It is recommended to use [withLock] for safety reasons, so that the acquired lock is always
     * released at the end of the critical section, and [unlock] is never invoked before a successful
     * lock acquisition.
     *
     * @param owner Optional owner token for debugging. When `owner` is specified (non-nullptr value) and this mutex
     *        was locked with the different token (by identity), this function throws [IllegalStateException].
     */
    virtual void unlock(void* owner = nullptr) = 0;

    /**
     * Clause for [select] expression of [lock] suspending function that selects when the mutex is locked.
     * Additional parameter for the clause in the `owner` (see [lock]) and when the clause is selected
     * the reference to this mutex is passed into the corresponding block.
     */
    // @Deprecated(level = DeprecationLevel.WARNING, message = "Mutex.onLock deprecated without replacement. " +
    //     "For additional details please refer to #2794") // WARNING since 1.6.0
    virtual selects::SelectClause2<void*, Mutex*>& get_on_lock() = 0;
};

/**
 * Creates a [Mutex] instance.
 * The mutex created is fair: lock is granted in first come, first served order.
 *
 * @param locked initial state of the mutex.
 */
Mutex* create_mutex(bool locked = false);

/**
 * Executes the given [action] under this mutex's lock.
 *
 * @param owner Optional owner token for debugging. When `owner` is specified (non-nullptr value) and this mutex
 *        is already locked with the same token (same identity), this function throws [IllegalStateException].
 *
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

// Void specialization or overload could be added here

} // namespace sync
} // namespace coroutines
} // namespace kotlinx
