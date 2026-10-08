#pragma once
// port-lint: source sync/Semaphore.kt
/**
 * @file Semaphore.hpp
 * @brief Counting semaphore for coroutines.
 *
 * Transliterated from: kotlinx-coroutines-core/common/src/sync/Semaphore.kt
 * Lines 12-87 (interface and factory function)
 */

#include <atomic>
#include <functional>
#include <memory>
#include <exception>
#include <type_traits>
#include "kotlinx/coroutines/ContinuationImpl.hpp"
#include "kotlinx/coroutines/dsl/Suspend.hpp"
#include <algorithm>
#include <stdexcept>
#include "kotlinx/coroutines/Continuation.hpp"

namespace kotlinx {
namespace coroutines {
namespace sync {

/**
 * Line 12-59: Semaphore interface
 *
 * A counting semaphore for coroutines that logically maintains a number of
 * available permits. Each acquire() takes a single permit or suspends until
 * it is available. Each release() adds a permit, potentially releasing a
 * suspended acquirer. Semaphore is fair and maintains a FIFO order of acquirers.
 *
 * Semaphores are mostly used to limit the number of coroutines that have
 * access to particular resource. Semaphore with `permits = 1` is essentially
 * a Mutex.
 */
// Transliterated from: kotlinx-coroutines-core/common/src/sync/Semaphore.kt:21-59
class Semaphore {
public:
    virtual ~Semaphore() = default;

    /**
     * Line 24-25: Returns the current number of permits available in this semaphore.
     */
    // Transliterated from: kotlinx-coroutines-core/common/src/sync/Semaphore.kt:25-25
    virtual int available_permits() const = 0;

    /**
     * Line 28-44: Acquires a permit from this semaphore, suspending until one is available.
     *
     * All suspending acquirers are processed in first-in-first-out (FIFO) order.
     *
     * This suspending function is cancellable: if the Job of the current coroutine
     * is cancelled while this suspending function is waiting, this function
     * immediately resumes with CancellationException.
     *
     * There is a **prompt cancellation guarantee**: even if this function is ready
     * to return the result, but was cancelled while suspended, CancellationException
     * will be thrown.
     *
     * @param cont The continuation for suspend/resume
     * @return COROUTINE_SUSPENDED or nullptr (Unit)
     */
    // Transliterated from: kotlinx-coroutines-core/common/src/sync/Semaphore.kt:44-44
    virtual void* acquire(Continuation<void*>* cont) = 0;

    /**
     * Line 47-51: Tries to acquire a permit from this semaphore without suspension.
     *
     * @return true if a permit was acquired, false otherwise.
     */
    // Transliterated from: kotlinx-coroutines-core/common/src/sync/Semaphore.kt:51-51
    virtual bool try_acquire() = 0;

    /**
     * Line 54-58: Releases a permit, returning it into this semaphore.
     *
     * Resumes the first suspending acquirer if there is one at the point of
     * invocation. Throws std::logic_error if the number of release invocations
     * is greater than the number of preceding acquire.
     */
    // Transliterated from: kotlinx-coroutines-core/common/src/sync/Semaphore.kt:58-58
    virtual void release() = 0;

    // NOTE(port): Ordinary C++ callers block on the same FIFO suspend operation.
    void acquire();
};

/**
 * Line 67-68: Factory function
 *
 * Creates new Semaphore instance.
 * @param permits the number of permits available in this semaphore.
 * @param acquired_permits the number of already acquired permits,
 *        should be between 0 and permits (inclusively).
 */
// Transliterated from: kotlinx-coroutines-core/common/src/sync/Semaphore.kt:68-68
std::shared_ptr<Semaphore> create_semaphore(int permits, int acquired_permits = 0);

/**
 * Line 77-87: withPermit
 *
 * Executes the given action, acquiring a permit from this semaphore at the
 * beginning and releasing it after the action is completed.
 *
 * @return the return value of the action.
 */
// NOTE(port): Blocking adapter; the suspend API below preserves the upstream contract.
template<typename T, typename ActionFunc>
T with_permit(Semaphore& semaphore, ActionFunc&& action) {
    semaphore.acquire();
    T result = [&]() -> T {
        try { return action(); }
        catch (...) { semaphore.release(); throw; }
    }();
    semaphore.release();
    return result;
}

// NOTE(port): Blocking Unit adapter for ordinary C++ callers.
template<typename ActionFunc>
void with_permit_void(Semaphore& semaphore, ActionFunc&& action) {
    semaphore.acquire();
    try { action(); }
    catch (...) { semaphore.release(); throw; }
    semaphore.release();
}

/**
 * Executes the given action, acquiring a permit at the beginning and releasing it
 * after the action completes. The receiving continuation owns the returned T box;
 * Unit is nullptr. The caller retains the semaphore through completion.
 */
// Transliterated from: kotlinx-coroutines-core/common/src/sync/Semaphore.kt:77-87
template<typename ActionFunc>
void* with_permit(Semaphore& semaphore, ActionFunc&& action, Continuation<void*>* completion) {
    using Action = std::decay_t<ActionFunc>;
    using T = std::invoke_result_t<Action&>;
    // Transliterated from: kotlinx-coroutines-core/common/src/sync/Semaphore.kt:77-87
    class Frame final : public ContinuationImpl {
    public:
        Frame(Semaphore& semaphore, Action action, Continuation<void*>* completion)
            : ContinuationImpl(internal::retain_continuation(completion)),
              semaphore_(semaphore), action_(std::move(action)) {}
        void retain() { self_ref_ = shared_from_this(); }
        // Transliterated from: kotlinx-coroutines-core/common/src/sync/Semaphore.kt:81-87
        void* invoke_suspend(Result<void*> result) override {
            try {
                coroutine_begin(this)
                coroutine_yield(this, semaphore_.acquire(this));
                void* value = execute_action();
                self_ref_.reset();
                return value;
            } catch (...) {
                self_ref_.reset();
                throw;
            }
        }
    private:
        // Transliterated from: kotlinx-coroutines-core/common/src/sync/Semaphore.kt:82-87
        void* execute_action() {
            if constexpr (std::is_void_v<T>) {
                try { action_(); }
                catch (...) { semaphore_.release(); throw; }
                semaphore_.release();
                return nullptr;
            } else {
                std::unique_ptr<T> value;
                try { value = std::make_unique<T>(action_()); }
                catch (...) { semaphore_.release(); throw; }
                semaphore_.release();
                return value.release();
            }
        }
        void* _label = nullptr;
        Semaphore& semaphore_;
        Action action_;
        std::shared_ptr<BaseContinuationImpl> self_ref_;
    };
    auto frame = std::make_shared<Frame>(semaphore, std::forward<ActionFunc>(action), completion);
    frame->retain();
    return frame->start(Result<void*>::success(nullptr));
}

} // namespace sync
} // namespace coroutines
} // namespace kotlinx
