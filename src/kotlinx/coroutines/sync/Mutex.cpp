/**
 * @file Mutex.cpp
 * @brief Mutex implementation using lock-free segment queue.
 *
 * Transliterated from: kotlinx-coroutines-core/common/src/sync/Mutex.kt
 * Lines 130-313
 *
 * Contains the private MutexImpl class.
 */

#include "kotlinx/coroutines/sync/Mutex.hpp"
#include "kotlinx/coroutines/sync/SemaphoreAndMutexImpl.hpp"
#include "kotlinx/coroutines/CancellableContinuation.hpp"
#include "kotlinx/coroutines/CancellableContinuationImpl.hpp"
#include "kotlinx/coroutines/internal/Symbol.hpp"
#include <memory>
#include <atomic>
#include <cassert>
#include <sstream>
#include <iomanip>

namespace kotlinx {
namespace coroutines {
namespace sync {

// Line 304: private val NO_OWNER = Symbol("NO_OWNER")
static internal::Symbol& NO_OWNER() {
    static internal::Symbol instance("NO_OWNER");
    return instance;
}

// Line 305: private val ON_LOCK_ALREADY_LOCKED_BY_OWNER = Symbol("ALREADY_LOCKED_BY_OWNER")
static internal::Symbol& ON_LOCK_ALREADY_LOCKED_BY_OWNER() {
    static internal::Symbol instance("ALREADY_LOCKED_BY_OWNER");
    return instance;
}

// Line 307-309: Constants for tryLock results
static constexpr int TRY_LOCK_SUCCESS = 0;
static constexpr int TRY_LOCK_FAILED = 1;
static constexpr int TRY_LOCK_ALREADY_LOCKED_BY_OWNER = 2;

// Line 311-313: Constants for holdsLock results
static constexpr int HOLDS_LOCK_UNLOCKED = 0;
static constexpr int HOLDS_LOCK_YES = 1;
static constexpr int HOLDS_LOCK_ANOTHER_OWNER = 2;

/**
 * Line 130-302: MutexImpl
 *
 * internal open class MutexImpl(locked: Boolean) :
 *     SemaphoreAndMutexImpl(1, if (locked) 1 else 0), Mutex
 *
 * This is a private implementation class.
 */
class MutexImpl : public SemaphoreAndMutexImpl, public Mutex {
private:
    // Line 137: private val owner = atomic<Any?>(if (locked) null else NO_OWNER)
    std::atomic<void*> owner_;

    // Line 139-142: onSelectCancellationUnlockConstructor
    // TODO(port): implement select support

public:
    /**
     * Line 130: Constructor
     */
    MutexImpl(bool locked)
        : SemaphoreAndMutexImpl(1, locked ? 1 : 0)
        , owner_(locked ? nullptr : static_cast<void*>(&NO_OWNER()))
    {}

    /**
     * Line 144-145: override val isLocked: Boolean get() = availablePermits == 0
     */
    bool is_locked() const override {
        return SemaphoreAndMutexImpl::available_permits() == 0;
    }

    /**
     * Line 147: override fun holdsLock(owner: Any): Boolean
     */
    bool holds_lock(void* owner) override {
        return holds_lock_impl(owner) == HOLDS_LOCK_YES;
    }

    /**
     * Line 166-169: override suspend fun lock(owner: Any?)
     */
    void lock(void* owner) override {
        if (try_lock(owner)) return;
        lock_suspend(owner);
    }

    /**
     * Line 176-181: override fun tryLock(owner: Any?): Boolean
     */
    bool try_lock(void* owner) override {
        int result = try_lock_impl(owner);
        switch (result) {
            case TRY_LOCK_SUCCESS:
                return true;
            case TRY_LOCK_FAILED:
                return false;
            case TRY_LOCK_ALREADY_LOCKED_BY_OWNER:
                throw std::logic_error(
                    "This mutex is already locked by the specified owner");
            default:
                throw std::logic_error("unexpected tryLock result");
        }
    }

    /**
     * Line 206-221: override fun unlock(owner: Any?)
     */
    void unlock(void* owner) override {
        while (true) {
            // Line 209: check(isLocked)
            if (!is_locked()) {
                throw std::logic_error("This mutex is not locked");
            }

            // Line 211: val curOwner = this.owner.value
            void* cur_owner = owner_.load(std::memory_order_acquire);

            // Line 212: if (curOwner === NO_OWNER) continue
            if (cur_owner == static_cast<void*>(&NO_OWNER())) continue;

            // Line 214: check(curOwner === owner || owner == null)
            if (cur_owner != owner && owner != nullptr) {
                std::ostringstream oss;
                oss << "This mutex is locked by " << cur_owner
                    << ", but " << owner << " is expected";
                throw std::logic_error(oss.str());
            }

            // Line 216: if (!this.owner.compareAndSet(curOwner, NO_OWNER)) continue
            void* expected = cur_owner;
            if (!owner_.compare_exchange_strong(expected, static_cast<void*>(&NO_OWNER()),
                    std::memory_order_release, std::memory_order_relaxed)) {
                continue;
            }

            // Line 218: release()
            SemaphoreAndMutexImpl::release();
            return;
        }
    }

    /**
     * Line 224-229: override val onLock
     *
     * @Deprecated - not implemented
     */
    selects::SelectClause2<void*, Mutex*>& get_on_lock() override {
        // TODO(port): implement select support
        static selects::SelectClause2<void*, Mutex*>* dummy = nullptr;
        throw std::logic_error("Mutex.onLock is deprecated and not implemented");
        return *dummy;
    }

    /**
     * Line 301: override fun toString()
     */
    std::string to_string() const {
        std::ostringstream oss;
        oss << "Mutex@" << std::hex << reinterpret_cast<uintptr_t>(this)
            << "[isLocked=" << (is_locked() ? "true" : "false")
            << ",owner=" << owner_.load(std::memory_order_acquire) << "]";
        return oss.str();
    }

private:
    /**
     * Line 154-164: private fun holdsLockImpl(owner: Any?): Int
     */
    int holds_lock_impl(void* owner) {
        while (true) {
            // Line 157: if (!isLocked) return HOLDS_LOCK_UNLOCKED
            if (!is_locked()) return HOLDS_LOCK_UNLOCKED;

            // Line 158: val curOwner = this.owner.value
            void* cur_owner = owner_.load(std::memory_order_acquire);

            // Line 160: if (curOwner === NO_OWNER) continue
            if (cur_owner == static_cast<void*>(&NO_OWNER())) continue;

            // Line 162: return if (curOwner === owner) HOLDS_LOCK_YES else HOLDS_LOCK_ANOTHER_OWNER
            return (cur_owner == owner) ? HOLDS_LOCK_YES : HOLDS_LOCK_ANOTHER_OWNER;
        }
    }

    /**
     * Line 171-174: private suspend fun lockSuspend(owner: Any?)
     *
     * suspendCancellableCoroutineReusable<Unit> { cont ->
     *     val contWithOwner = CancellableContinuationWithOwner(cont, owner)
     *     acquire(contWithOwner)
     * }
     */
    void lock_suspend(void* owner) {
        // Line 172-174: Create wrapper with owner and acquire
        // CancellableContinuationWithOwner wraps the continuation to track owner
        // For this blocking version, we spin-wait until we can acquire
        // The proper suspend version would use suspend_cancellable_coroutine
        while (true) {
            if (SemaphoreAndMutexImpl::try_acquire()) {
                assert(owner_.load(std::memory_order_acquire) == static_cast<void*>(&NO_OWNER()));
                owner_.store(owner, std::memory_order_release);
                return;
            }
            // No permit available, would suspend in coroutine context
            // For blocking context, we spin briefly then retry
        }
    }

    /**
     * Line 183-204: private fun tryLockImpl(owner: Any?): Int
     */
    int try_lock_impl(void* owner) {
        while (true) {
            // Line 185-188: if (tryAcquire())
            if (SemaphoreAndMutexImpl::try_acquire()) {
                // Line 186: assert { this.owner.value === NO_OWNER }
                assert(owner_.load(std::memory_order_acquire) == static_cast<void*>(&NO_OWNER()));
                // Line 187: this.owner.value = owner
                owner_.store(owner, std::memory_order_release);
                return TRY_LOCK_SUCCESS;
            } else {
                // Line 193: if (owner == null) return TRY_LOCK_FAILED
                if (owner == nullptr) return TRY_LOCK_FAILED;

                // Line 194-201: Check if locked by same owner
                switch (holds_lock_impl(owner)) {
                    case HOLDS_LOCK_YES:
                        return TRY_LOCK_ALREADY_LOCKED_BY_OWNER;
                    case HOLDS_LOCK_ANOTHER_OWNER:
                        return TRY_LOCK_FAILED;
                    case HOLDS_LOCK_UNLOCKED:
                        continue; // Mutex unlocked, retry
                }
            }
        }
    }

    /**
     * Line 231-237: protected open fun onLockRegFunction(select: SelectInstance<*>, owner: Any?)
     */
    template <typename R>
    void on_lock_reg_function(selects::SelectInstance<R>* select, void* owner) {
        if (owner != nullptr && holds_lock(owner)) {
            select->select_in_registration_phase(&ON_LOCK_ALREADY_LOCKED_BY_OWNER());
        } else {
            // TODO(port): SelectInstanceWithOwner wrapper
            // onAcquireRegFunction(SelectInstanceWithOwner(select as SelectInstanceInternal<*>, owner), owner)
            SemaphoreAndMutexImpl::on_acquire_reg_function(select, owner);
        }
    }

    /**
     * Line 239-244: protected open fun onLockProcessResult(owner: Any?, result: Any?): Any?
     */
    void* on_lock_process_result(void* owner, void* result) {
        if (result == static_cast<void*>(&ON_LOCK_ALREADY_LOCKED_BY_OWNER())) {
            std::ostringstream oss;
            oss << "This mutex is already locked by the specified owner: " << owner;
            throw std::logic_error(oss.str());
        }
        return this;
    }

    // Line 247-279: CancellableContinuationWithOwner - TODO(port): owner tracking wrapper
    // Line 281-299: SelectInstanceWithOwner - TODO(port): select instance wrapper
};

Mutex* create_mutex(bool locked) {
    return new MutexImpl(locked);
}

std::shared_ptr<Mutex> make_mutex(bool locked) {
    return std::make_shared<MutexImpl>(locked);
}

} // namespace sync
} // namespace coroutines
} // namespace kotlinx
