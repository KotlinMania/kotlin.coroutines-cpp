#include <string>
#include <optional>
#include "kotlinx/coroutines/core_fwd.hpp"
// Transliterated from Kotlin to C++
// Original: kotlinx-coroutines-core/common/src/sync/Mutex.kt
//
// TODO: This is a mechanical syntax transliteration. The following Kotlin constructs need proper C++ implementation:
// - suspend functions (marked but not implemented as C++20 coroutines)
// - struct (converted to abstract class)
// - Kotlin atomicfu (atomic<T> needs C++ std::atomic)
// - Default parameters (need overloads or std::optional)
// - Smart casts and when expressions
// - Nullable types (T* -> T* or std::optional<T>)
// - Extension functions (converted to free functions)
// - Kotlin contracts (contract { ... })
// - Lambda types and inline functions
// - @Suppress, @OptIn, @JvmField annotations (kept as comments)
// - Symbol type (using sentinel pointers)
// - check/require functions (converted to assertions/exceptions)

namespace kotlinx {
namespace coroutines {
namespace sync {

// import kotlinx.atomicfu.*
// import kotlinx.coroutines.*
// import kotlinx.coroutines.internal.*
// import kotlinx.coroutines.selects.*
// import kotlin.contracts.*
// import kotlin.coroutines.CoroutineContext
// import kotlin.jvm.*

/**
 * Mutual exclusion for coroutines.
 *
 * Mutex has two states: _locked_ and _unlocked_.
 * It is **non-reentrant**, that is invoking [lock] even from the same thread/coroutine that currently holds
 * the lock still suspends the invoker.
 *
 * JVM API note:
 * Memory semantic of the [Mutex] is similar to `synchronized` block on JVM:
 * An unlock operation on a [Mutex] happens-before every subsequent successful lock on that [Mutex].
 * Unsuccessful call to [tryLock] do not have any memory effects.
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
    virtual void lock(void* owner = nullptr) = 0; // TODO: suspend

    /**
     * Clause for [select] expression of [lock] suspending function that selects when the mutex is locked.
     * Additional parameter for the clause in the `owner` (see [lock]) and when the clause is selected
     * the reference to this mutex is passed into the corresponding block.
     */
    // @Deprecated(level = DeprecationLevel.WARNING, message = "Mutex.onLock deprecated without replacement. " +
    //     "For additional details please refer to #2794") // WARNING since 1.6.0
    virtual SelectClause2<void*, Mutex*>& get_on_lock() = 0;

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
};

/**
 * Creates a [Mutex] instance.
 * The mutex created is fair: lock is granted in first come, first served order.
 *
 * @param locked initial state of the mutex.
 */
// @Suppress("FunctionName")
Mutex* create_mutex(bool locked = false); // Forward declaration

/**
 * Executes the given [action] under this mutex's lock.
 *
 * @param owner Optional owner token for debugging. When `owner` is specified (non-nullptr value) and this mutex
 *        is already locked with the same token (same identity), this function throws [IllegalStateException].
 *
 * @return the return value of the action.
 */
// @OptIn(ExperimentalContracts::class)
template<typename T, typename ActionFunc>
T with_lock(Mutex& mutex, void* owner, ActionFunc&& action) {
    // TODO: suspend function semantics not implemented
    // TODO: Kotlin contract { callsInPlace(action, InvocationKind.EXACTLY_ONCE) } not applicable in C++
    // contract {
    //     callsInPlace(action, InvocationKind.EXACTLY_ONCE)
    // }
    mutex.lock(owner);
    try {
        return action();
    } catch (...) {
        mutex.unlock(owner);
        throw;
    }
}

class MutexImpl : SemaphoreAndMutexImpl, Mutex {
protected:
    /**
     * After the lock is acquired, the corresponding owner is stored in this field.
     * The [unlock] operation checks the owner and either re-sets it to [NO_OWNER],
     * if there is no waiting request, or to the owner of the suspended [lock] operation
     * to be resumed, otherwise.
     */
    std::atomic<void*> owner;

    OnCancellationConstructor on_select_cancellation_unlock_constructor;

public:
    explicit MutexImpl(bool locked)
        : SemaphoreAndMutexImpl(1, locked ? 1 : 0),
          owner(locked ? nullptr : kNoOwner),
          on_select_cancellation_unlock_constructor(
              [](SelectInstance<void*>* /*select*/, void* owner_, void* /*internal_result*/) {
                  return [owner_](std::exception_ptr, void*, CoroutineContext) {
                      // TODO: unlock(owner_)
                  };
              }
          ) {}

    bool is_locked() const override {
        return available_permits == 0;
    }

    bool holds_lock(void* owner_) override {
        return holds_lock_impl(owner_) == kHoldsLockYes;
    }

protected:
    /**
     * [HOLDS_LOCK_UNLOCKED] if the mutex is unlocked
     * [HOLDS_LOCK_YES] if the mutex is held with the specified [owner]
     * [HOLDS_LOCK_ANOTHER_OWNER] if the mutex is held with a different owner
     */
    int holds_lock_impl(void* owner_) {
        while (true) {
            // Is this mutex locked*
            if (!is_locked()) return kHoldsLockUnlocked;
            void* cur_owner = owner.load();
            // Wait in a spin-loop until the owner is set
            if (cur_owner == kNoOwner) continue; // <-- ATTENTION, BLOCKING PART HERE
            // Check the owner
            return (cur_owner == owner_) ? kHoldsLockYes : kHoldsLockAnotherOwner;
        }
    }

public:
    void lock(void* owner_ = nullptr) override {
        // TODO: suspend function semantics not implemented
        if (try_lock(owner_)) return;
        lock_suspend(owner_);
    }

private:
    void lock_suspend(void* owner_) {
        // TODO: suspend function semantics not implemented
        // TODO: suspendCancellableCoroutineReusable
        // CancellableContinuationWithOwner cont_with_owner(cont, owner_);
        // acquire(cont_with_owner);
    }

public:
    bool try_lock(void* owner_ = nullptr) override {
        int result = try_lock_impl(owner_);
        switch (result) {
            case kTryLockSuccess: return true;
            case kTryLockFailed: return false;
            case kTryLockAlreadyLockedByOwner:
                throw std::runtime_error("This mutex is already locked by the specified owner");
            default:
                throw std::runtime_error("unexpected");
        }
    }

private:
    int try_lock_impl(void* owner_) {
        while (true) {
            if (try_acquire()) {
                // TODO: assert { this->owner.load() == kNoOwner }
                owner.store(owner_);
                return kTryLockSuccess;
            } else {
                // The semaphore permit acquisition has failed.
                // However, we need to check that this mutex is not
                // locked by our owner.
                if (owner_ == nullptr) return kTryLockFailed;
                int holds = holds_lock_impl(owner_);
                switch (holds) {
                    // This mutex is already locked by our owner.
                    case kHoldsLockYes:
                        return kTryLockAlreadyLockedByOwner;
                    // This mutex is locked by another owner, `trylock(..)` must return `false`.
                    case kHoldsLockAnotherOwner:
                        return kTryLockFailed;
                    // This mutex is no longer locked, restart the operation.
                    case kHoldsLockUnlocked:
                        continue;
                }
            }
        }
    }

public:
    void unlock(void* owner_ = nullptr) override {
        while (true) {
            // Is this mutex locked*
            if (!is_locked()) {
                throw std::runtime_error("This mutex is not locked");
            }
            // Read the owner, waiting until it is set in a spin-loop if required.
            void* cur_owner = owner.load();
            if (cur_owner == kNoOwner) continue; // <-- ATTENTION, BLOCKING PART HERE
            // Check the owner.
            if (!(cur_owner == owner_ || owner_ == nullptr)) {
                throw std::runtime_error("This mutex is locked by different owner");
            }
            // Try to clean the owner first. We need to use CAS here to synchronize with concurrent `unlock(..)`-s.
            void* expected = cur_owner;
            if (!owner.compare_exchange_strong(expected, kNoOwner)) continue;
            // Release the semaphore permit at the end.
            release();
            return;
        }
    }

    // @Suppress("UNCHECKED_CAST", "OverridingDeprecatedMember", "OVERRIDE_DEPRECATION")
    SelectClause2<void*, Mutex*>& get_on_lock() override {
        // TODO: return SelectClause2Impl
        static SelectClause2Impl<void*, Mutex*> clause(
            this,
            /* regFunc = */ [](void* clause_obj, SelectInstance<void*>* select, void* param) {
                // TODO: cast and call on_lock_reg_function
            },
            /* processResFunc = */ [](void* clause_obj, void* param, void* result) -> void* {
                // TODO: call on_lock_process_result
                return nullptr;
            },
            &on_select_cancellation_unlock_constructor
        );
        return clause;
    }

protected:
    virtual void on_lock_reg_function(SelectInstance<void*>* select, void* owner_) {
        if (owner_ != nullptr && holds_lock(owner_)) {
            select->select_in_registration_phase(kOnLockAlreadyLockedByOwner);
        } else {
            // TODO: on_acquire_reg_function(SelectInstanceWithOwner(select, owner_), owner_);
        }
    }

    virtual void* on_lock_process_result(void* owner_, void* result) {
        if (result == kOnLockAlreadyLockedByOwner) {
            throw std::runtime_error("This mutex is already locked by the specified owner");
        }
        return this;
    }

    // @OptIn(InternalForInheritanceCoroutinesApi::class)
    class CancellableContinuationWithOwner : CancellableContinuation<void*>, Waiter {
    public:
        // @JvmField
        CancellableContinuationImpl<void*>* cont;
        // @JvmField
        void* owner;

        CancellableContinuationWithOwner(
            CancellableContinuationImpl<void*>* cont_,
            void* owner_
        ) : cont(cont_), owner(owner_) {}

        // TODO: Implement CancellableContinuation struct methods
        // TODO: Implement Waiter struct methods
    };

    template<typename Q>
    class SelectInstanceWithOwner : SelectInstanceInternal<Q> {
    public:
        // @JvmField
        SelectInstanceInternal<Q>* select;
        // @JvmField
        void* owner;

        SelectInstanceWithOwner(
            SelectInstanceInternal<Q>* select_,
            void* owner_
        ) : select(select_), owner(owner_) {}

        bool try_select(void* clause_object, void* result) override {
            // TODO: assert { owner.load() == kNoOwner }
            bool success = select->try_select(clause_object, result);
            if (success) {
                // TODO: set owner
            }
            return success;
        }

        void select_in_registration_phase(void* internal_result) override {
            // TODO: assert { owner.load() == kNoOwner }
            // TODO: set owner
            select->select_in_registration_phase(internal_result);
        }

        // TODO: Delegate other SelectInstanceInternal methods to select
    };

    // TODO: tostd::string override
};

// Factory function implementation
Mutex* create_mutex(bool locked) {
    return new MutexImpl(locked);
}

// Symbol-like markers
void* kNoOwner = reinterpret_cast<void*>(0x100);
void* kOnLockAlreadyLockedByOwner = reinterpret_cast<void*>(0x101);

// try_lock results
constexpr int kTryLockSuccess = 0;
constexpr int kTryLockFailed = 1;
constexpr int kTryLockAlreadyLockedByOwner = 2;

// holds_lock results
constexpr int kHoldsLockUnlocked = 0;
constexpr int kHoldsLockYes = 1;
constexpr int kHoldsLockAnotherOwner = 2;

} // namespace sync
} // namespace coroutines
} // namespace kotlinx
