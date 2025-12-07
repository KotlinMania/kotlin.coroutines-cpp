#include <string>
#include <optional>
#include <functional>
#include "kotlinx/coroutines/core_fwd.hpp"
// Transliterated from Kotlin to C++
// Original: kotlinx-coroutines-core/common/src/sync/Semaphore.kt
//
// TODO: This is a mechanical syntax transliteration. The following Kotlin constructs need proper C++ implementation:
// - suspend functions (marked but not implemented as C++20 coroutines)
// - struct (converted to abstract class)
// - Kotlin atomicfu (atomic<T> needs C++ std::atomic)
// - AtomicRef, AtomicArray types
// - Default parameters (need overloads or std::optional)
// - require() function (converted to exceptions)
// - Extension functions (converted to free functions)
// - Kotlin contracts (contract { ... })
// - Lambda types and inline functions
// - when expressions (converted to if-else or switch)
// - @Suppress, @OptIn, @JsName annotations (kept as comments)
// - Symbol type (using sentinel pointers)
// - Segment-based lock-free queues
// - systemProp for configuration
// - repeat construct (converted to for loop)
// - assert blocks

namespace kotlinx {
namespace coroutines {
namespace sync {

// import kotlinx.atomicfu.*
// import kotlinx.coroutines.*
// import kotlinx.coroutines.internal.*
// import kotlinx.coroutines.selects.*
// import kotlin.contracts.*
// import kotlin.coroutines.*
// import kotlin.js.*
// import kotlin.math.*

/**
 * A counting semaphore for coroutines that logically maintains a number of available permits.
 * Each [acquire] takes a single permit or suspends until it is available.
 * Each [release] adds a permit, potentially releasing a suspended acquirer.
 * Semaphore is fair and maintains a FIFO order of acquirers.
 *
 * Semaphores are mostly used to limit the number of coroutines that have access to particular resource.
 * Semaphore with `permits = 1` is essentially a [Mutex].
 **/
class Semaphore {
public:
    virtual ~Semaphore() = default;

    /**
     * Returns the current number of permits available in this semaphore.
     */
    virtual int get_available_permits() const = 0;

    /**
     * Acquires a permit from this semaphore, suspending until one is available.
     * All suspending acquirers are processed in first-in-first-out (FIFO) order.
     *
     * This suspending function is cancellable: if the [Job] of the current coroutine is cancelled while this
     * suspending function is waiting, this function immediately resumes with [CancellationException].
     * There is a **prompt cancellation guarantee**: even if this function is ready to return the result, but was cancelled
     * while suspended, [CancellationException] will be thrown. See [suspendCancellableCoroutine] for low-level details.
     * This function releases the semaphore if it was already acquired by this function before the [CancellationException]
     * was thrown.
     *
     * Note that this function does not check for cancellation when it does not suspend.
     * Use [CoroutineScope.isActive] or [CoroutineScope.ensureActive] to periodically
     * check for cancellation in tight loops if needed.
     *
     * Use [tryAcquire] to try to acquire a permit of this semaphore without suspension.
     */
    virtual void acquire() = 0; // TODO: suspend

    /**
     * Tries to acquire a permit from this semaphore without suspension.
     *
     * @return `true` if a permit was acquired, `false` otherwise.
     */
    virtual bool try_acquire() = 0;

    /**
     * Releases a permit, returning it into this semaphore. Resumes the first
     * suspending acquirer if there is one at the point of invocation.
     * Throws [IllegalStateException] if the number of [release] invocations is greater than the number of preceding [acquire].
     */
    virtual void release() = 0;
};

/**
 * Creates new [Semaphore] instance.
 * @param permits the number of permits available in this semaphore.
 * @param acquiredPermits the number of already acquired permits,
 *        should be between `0` and `permits` (inclusively).
 */
// @Suppress("FunctionName")
Semaphore* create_semaphore(int permits, int acquired_permits = 0); // Forward declaration

/**
 * Executes the given [action], acquiring a permit from this semaphore at the beginning
 * and releasing it after the [action] is completed.
 *
 * @return the return value of the [action].
 */
// @OptIn(ExperimentalContracts::class)
template<typename T, typename ActionFunc>
T with_permit(Semaphore& semaphore, ActionFunc&& action) {
    // TODO: suspend function semantics not implemented
    // TODO: Kotlin contract { callsInPlace(action, InvocationKind.EXACTLY_ONCE) } not applicable in C++
    // contract {
    //     callsInPlace(action, InvocationKind.EXACTLY_ONCE)
    // }
    semaphore.acquire();
    try {
        return action();
    } catch (...) {
        semaphore.release();
        throw;
    }
}

// @Suppress("UNCHECKED_CAST")
class SemaphoreAndMutexImpl {
protected:
    /*
       The queue of waiting acquirers is essentially an infinite array based on the list of segments
       (see `SemaphoreSegment`); each segment contains a fixed number of slots. To determine a slot for each enqueue
       and dequeue operation, we increment the corresponding counter at the beginning of the operation
       and use the value before the increment as a slot number. This way, each enqueue-dequeue pair
       works with an individual cell. We use the corresponding segment pointers to find the required ones.

       Here is a state machine for cells. Note that only one `acquire` and at most one `release` operation
       can deal with each cell, and that `release` uses `getAndSet(PERMIT)` to perform transitions for performance reasons
       so that the state `PERMIT` represents different logical states.

         +------+ `acquire` suspends   +------+   `release` tries    +--------+                    // if `cont.tryResume(..)` succeeds, then
         | NULL | -------------------> | cont | -------------------> | PERMIT | (cont RETRIEVED)   // the corresponding `acquire` operation gets
         +------+                      +------+   to resume `cont`   +--------+                    // a permit and the `release` one completes.
            |                             |
            |                             | `acquire` request is cancelled and the continuation is
            | `release` comes             | replaced with a special `CANCEL` token to avoid memory leaks
            | to the slot before          V
            | `acquire` and puts    +-----------+   `release` has    +--------+
            | a permit into the     | CANCELLED | -----------------> | PERMIT | (RELEASE FAILED)
            | slot, waiting for     +-----------+        failed      +--------+
            | `acquire` after
            | that.
            |
            |           `acquire` gets   +-------+
            |        +-----------------> | TAKEN | (ELIMINATION HAPPENED)
            V        |    the permit     +-------+
        +--------+   |
        | PERMIT | -<
        +--------+  |
                    |  `release` has waited a bounded time,   +--------+
                    +---------------------------------------> | BROKEN | (BOTH RELEASE AND ACQUIRE FAILED)
                           but `acquire` has not come         +--------+
    */

    int permits;

    std::atomic<SemaphoreSegment*> head;
    std::atomic<long> deq_idx;
    std::atomic<SemaphoreSegment*> tail;
    std::atomic<long> enq_idx;

    /**
     * This counter indicates the number of available permits if it is positive,
     * or the negated number of waiters on this semaphore otherwise.
     * Note, that 32-bit counter is enough here since the maximal number of available
     * permits is [permits] which is [Int], and the maximum number of waiting acquirers
     * cannot be greater than 2^31 in any real application.
     */
    std::atomic<int> available_permits_;

    std::function<void(std::exception_ptr, void*, CoroutineContext)> on_cancellation_release;

public:
    SemaphoreAndMutexImpl(int permits_, int acquired_permits)
        : permits(permits_),
          deq_idx(0),
          enq_idx(0),
          available_permits_(permits_ - acquired_permits),
          on_cancellation_release([this](std::exception_ptr, void*, CoroutineContext) { release(); })
    {
        if (!(permits_ > 0)) {
            throw std::invalid_argument("Semaphore should have at least 1 permit");
        }
        if (!(acquired_permits >= 0 && acquired_permits <= permits_)) {
            throw std::invalid_argument("The number of acquired permits should be in range");
        }
        SemaphoreSegment* s = new SemaphoreSegment(0, nullptr, 2);
        head.store(s);
        tail.store(s);
    }

    int get_available_permits() const {
        return std::max(available_permits_.load(), 0);
    }

    bool try_acquire() {
        while (true) {
            // Get the current number of available permits.
            int p = available_permits_.load();
            // Is the number of available permits greater
            // than the maximal one because of an incorrect
            // `release()` call without a preceding `acquire()`?
            // Change it to `permits` and start from the beginning.
            if (p > permits) {
                coerce_available_permits_at_maximum();
                continue;
            }
            // Try to decrement the number of available
            // permits if it is greater than zero.
            if (p <= 0) return false;
            if (available_permits_.compare_exchange_strong(p, p - 1)) return true;
        }
    }

    void acquire() {
        // TODO: suspend function semantics not implemented
        // Decrement the number of available permits.
        int p = dec_permits();
        // Is the permit acquired*
        if (p > 0) return; // permit acquired
        // Try to suspend otherwise.
        acquire_slow_path();
    }

private:
    void acquire_slow_path() {
        // TODO: suspend function semantics not implemented
        // TODO: suspendCancellableCoroutineReusable
    }

    // @JsName("acquireCont")
protected:
    void acquire(CancellableContinuation<void*>& waiter) {
        acquire_internal(
            &waiter,
            [this](void* cont) { return add_acquire_to_queue(static_cast<Waiter*>(cont)); },
            [this](void* cont) {
                auto* c = static_cast<CancellableContinuation<void*>*>(cont);
                c->resume(nullptr, on_cancellation_release);
            }
        );
    }

    // @JsName("acquireInternal")
private:
    template<typename W>
    void acquire_internal(
        W* waiter,
        std::function<bool(W*)> suspend_func,
        std::function<void(W*)> on_acquired_func
    ) {
        while (true) {
            // Decrement the number of available permits at first.
            int p = dec_permits();
            // Is the permit acquired*
            if (p > 0) {
                on_acquired_func(waiter);
                return;
            }
            // Permit has not been acquired, try to suspend.
            if (suspend_func(waiter)) return;
        }
    }

protected:
    // We do not fully support `onAcquire` as it is needed only for `Mutex.onLock`.
    // @Suppress("UNUSED_PARAMETER")
    void on_acquire_reg_function(SelectInstance<void*>* select, void* ignored_param) {
        acquire_internal(
            select,
            [this](SelectInstance<void*>* s) { return add_acquire_to_queue(static_cast<Waiter*>(s)); },
            [](SelectInstance<void*>* s) { s->select_in_registration_phase(nullptr); }
        );
    }

private:
    /**
     * Decrements the number of available permits
     * and ensures that it is not greater than [permits]
     * at the point of decrement. The last may happen
     * due to an incorrect `release()` call without
     * a preceding `acquire()`.
     */
    int dec_permits() {
        while (true) {
            // Decrement the number of available permits.
            int p = available_permits_.fetch_sub(1);
            // Is the number of available permits greater
            // than the maximal one due to an incorrect
            // `release()` call without a preceding `acquire()`?
            if (p > permits) continue;
            // The number of permits is correct, return it.
            return p;
        }
    }

public:
    void release() {
        while (true) {
            // Increment the number of available permits.
            int p = available_permits_.fetch_add(1);
            // Is this `release` call correct and does not
            // exceed the maximal number of permits*
            if (p >= permits) {
                // Revert the number of available permits
                // back to the correct one and fail with error.
                coerce_available_permits_at_maximum();
                throw std::runtime_error("The number of released permits cannot be greater than permits");
            }
            // Is there a waiter that should be resumed*
            if (p >= 0) return;
            // Try to resume the first waiter, and
            // restart the operation if either this
            // first waiter is cancelled or
            // due to `SYNC` resumption mode.
            if (try_resume_next_from_queue()) return;
        }
    }

private:
    /**
     * Changes the number of available permits to
     * [permits] if it became greater due to an
     * incorrect [release] call.
     */
    void coerce_available_permits_at_maximum() {
        while (true) {
            int cur = available_permits_.load();
            if (cur <= permits) break;
            if (available_permits_.compare_exchange_strong(cur, permits)) break;
        }
    }

    /**
     * Returns `false` if the received permit cannot be used and the calling operation should restart.
     */
    bool add_acquire_to_queue(Waiter* waiter) {
        // TODO: Segment-based queue implementation
        return false;
    }

    // @Suppress("UNCHECKED_CAST")
    bool try_resume_next_from_queue() {
        // TODO: Segment-based queue implementation
        return false;
    }

    bool try_resume_acquire(void* obj) {
        // TODO: Type-based dispatch for CancellableContinuation vs SelectInstance
        return false;
    }
};

class SemaphoreImpl : SemaphoreAndMutexImpl, Semaphore {
public:
    SemaphoreImpl(int permits, int acquired_permits)
        : SemaphoreAndMutexImpl(permits, acquired_permits) {}

    int get_available_permits() const override {
        return SemaphoreAndMutexImpl::get_available_permits();
    }

    void acquire() override {
        SemaphoreAndMutexImpl::acquire();
    }

    bool try_acquire() override {
        return SemaphoreAndMutexImpl::try_acquire();
    }

    void release() override {
        SemaphoreAndMutexImpl::release();
    }
};

SemaphoreSegment* create_segment(long id, SemaphoreSegment* prev) {
    return new SemaphoreSegment(id, prev, 0);
}

class SemaphoreSegment : Segment<SemaphoreSegment> {
public:
    std::vector<std::atomic<void*>> acquirers;

    SemaphoreSegment(long id, SemaphoreSegment* prev, int pointers)
        : Segment<SemaphoreSegment>(id, prev, pointers),
          acquirers(kSegmentSize) {
        for (int i = 0; i < kSegmentSize; ++i) {
            acquirers[i].store(nullptr);
        }
    }

    int get_number_of_slots() const override { return kSegmentSize; }

    // @Suppress("NOTHING_TO_INLINE")
    void* get(int index) const {
        return acquirers[index].load();
    }

    // @Suppress("NOTHING_TO_INLINE")
    void set(int index, void* value) {
        acquirers[index].store(value);
    }

    // @Suppress("NOTHING_TO_INLINE")
    bool cas(int index, void* expected, void* value) {
        return acquirers[index].compare_exchange_strong(expected, value);
    }

    // @Suppress("NOTHING_TO_INLINE")
    void* get_and_set(int index, void* value) {
        return acquirers[index].exchange(value);
    }

    // Cleans the acquirer slot located by the specified index
    // and removes this segment physically if all slots are cleaned.
    void on_cancellation(int index, std::exception_ptr cause, CoroutineContext context) override {
        // Clean the slot
        set(index, kCancelled);
        // Remove this segment if needed
        on_slot_cleaned();
    }

    // TODO: tostd::string override
};

// Configuration constants
// TODO: systemProp implementation
constexpr int kMaxSpinCycles = 100; // systemProp("kotlinx.coroutines.semaphore.maxSpinCycles", 100)
constexpr int kSegmentSize = 16; // systemProp("kotlinx.coroutines.semaphore.segmentSize", 16)

// Symbol-like markers
void* kPermit = reinterpret_cast<void*>(0x200);
void* kTaken = reinterpret_cast<void*>(0x201);
void* kBroken = reinterpret_cast<void*>(0x202);
void* kCancelled = reinterpret_cast<void*>(0x203);

// Factory function implementation
Semaphore* create_semaphore(int permits, int acquired_permits) {
    return new SemaphoreImpl(permits, acquired_permits);
}

} // namespace sync
} // namespace coroutines
} // namespace kotlinx
