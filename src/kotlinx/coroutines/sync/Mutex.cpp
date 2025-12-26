/**
 * @file Mutex.cpp
 * @brief Mutex implementation using lock-free segment queue.
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

static internal::Symbol& NO_OWNER() {
    static internal::Symbol instance("NO_OWNER");
    return instance;
}

static internal::Symbol& ON_LOCK_ALREADY_LOCKED_BY_OWNER() {
    static internal::Symbol instance("ALREADY_LOCKED_BY_OWNER");
    return instance;
}

// Constants for tryLock results
static constexpr int TRY_LOCK_SUCCESS = 0;
static constexpr int TRY_LOCK_FAILED = 1;
static constexpr int TRY_LOCK_ALREADY_LOCKED_BY_OWNER = 2;

// Constants for holdsLock results
static constexpr int HOLDS_LOCK_UNLOCKED = 0;
static constexpr int HOLDS_LOCK_YES = 1;
static constexpr int HOLDS_LOCK_ANOTHER_OWNER = 2;

/**
 * Private implementation of Mutex using SemaphoreAndMutexImpl.
 */
class MutexImpl : public SemaphoreAndMutexImpl, public Mutex {
private:
    std::atomic<void*> owner_;

public:
    MutexImpl(bool locked)
        : SemaphoreAndMutexImpl(1, locked ? 1 : 0)
        , owner_(locked ? nullptr : static_cast<void*>(&NO_OWNER()))
    {}

    bool is_locked() const override {
        return SemaphoreAndMutexImpl::available_permits() == 0;
    }

    bool holds_lock(void* owner) override {
        return holds_lock_impl(owner) == HOLDS_LOCK_YES;
    }

    void lock(void* owner) override {
        if (try_lock(owner)) return;
        lock_suspend(owner);
    }

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

    void unlock(void* owner) override {
        while (true) {
            if (!is_locked()) {
                throw std::logic_error("This mutex is not locked");
            }

            void* cur_owner = owner_.load(std::memory_order_acquire);
            if (cur_owner == static_cast<void*>(&NO_OWNER())) continue;

            if (cur_owner != owner && owner != nullptr) {
                std::ostringstream oss;
                oss << "This mutex is locked by " << cur_owner
                    << ", but " << owner << " is expected";
                throw std::logic_error(oss.str());
            }

            void* expected = cur_owner;
            if (!owner_.compare_exchange_strong(expected, static_cast<void*>(&NO_OWNER()),
                    std::memory_order_release, std::memory_order_relaxed)) {
                continue;
            }

            SemaphoreAndMutexImpl::release();
            return;
        }
    }

    selects::SelectClause2<void*, Mutex*>& get_on_lock() override {
        static selects::SelectClause2<void*, Mutex*>* dummy = nullptr;
        throw std::logic_error("Mutex.onLock is deprecated and not implemented");
        return *dummy;
    }

    std::string to_string() const {
        std::ostringstream oss;
        oss << "Mutex@" << std::hex << reinterpret_cast<uintptr_t>(this)
            << "[isLocked=" << (is_locked() ? "true" : "false")
            << ",owner=" << owner_.load(std::memory_order_acquire) << "]";
        return oss.str();
    }

private:
    int holds_lock_impl(void* owner) {
        while (true) {
            if (!is_locked()) return HOLDS_LOCK_UNLOCKED;
            void* cur_owner = owner_.load(std::memory_order_acquire);
            if (cur_owner == static_cast<void*>(&NO_OWNER())) continue;
            return (cur_owner == owner) ? HOLDS_LOCK_YES : HOLDS_LOCK_ANOTHER_OWNER;
        }
    }

    void lock_suspend(void* owner) {
        // Blocking version: spin-wait until we can acquire
        // TODO(suspend-plugin): proper suspend version would use suspend_cancellable_coroutine
        while (true) {
            if (SemaphoreAndMutexImpl::try_acquire()) {
                assert(owner_.load(std::memory_order_acquire) == static_cast<void*>(&NO_OWNER()));
                owner_.store(owner, std::memory_order_release);
                return;
            }
        }
    }

    int try_lock_impl(void* owner) {
        while (true) {
            if (SemaphoreAndMutexImpl::try_acquire()) {
                assert(owner_.load(std::memory_order_acquire) == static_cast<void*>(&NO_OWNER()));
                owner_.store(owner, std::memory_order_release);
                return TRY_LOCK_SUCCESS;
            } else {
                if (owner == nullptr) return TRY_LOCK_FAILED;
                switch (holds_lock_impl(owner)) {
                    case HOLDS_LOCK_YES:
                        return TRY_LOCK_ALREADY_LOCKED_BY_OWNER;
                    case HOLDS_LOCK_ANOTHER_OWNER:
                        return TRY_LOCK_FAILED;
                    case HOLDS_LOCK_UNLOCKED:
                        continue;
                }
            }
        }
    }

    template <typename R>
    void on_lock_reg_function(selects::SelectInstance<R>* select, void* owner) {
        if (owner != nullptr && holds_lock(owner)) {
            select->select_in_registration_phase(&ON_LOCK_ALREADY_LOCKED_BY_OWNER());
        } else {
            SemaphoreAndMutexImpl::on_acquire_reg_function(select, owner);
        }
    }

    void* on_lock_process_result(void* owner, void* result) {
        if (result == static_cast<void*>(&ON_LOCK_ALREADY_LOCKED_BY_OWNER())) {
            std::ostringstream oss;
            oss << "This mutex is already locked by the specified owner: " << owner;
            throw std::logic_error(oss.str());
        }
        return this;
    }
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
