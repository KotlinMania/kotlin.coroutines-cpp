#pragma once
/**
 * @file SemaphoreSegment.hpp
 * @brief Segment implementation for Semaphore and Mutex.
 *
 * Transliterated from: kotlinx-coroutines-core/common/src/sync/Semaphore.kt
 * Lines 361-396
 */

#include <atomic>
#include <array>
#include <memory>
#include <cassert>
#include "kotlinx/coroutines/internal/ConcurrentLinkedList.hpp"
#include "kotlinx/coroutines/internal/Symbol.hpp"
#include "kotlinx/coroutines/internal/SystemProps.hpp"

namespace kotlinx {
namespace coroutines {
namespace sync {

// Line 390-395: Symbols for cell states
// Line 391: private val PERMIT = Symbol("PERMIT")
inline internal::Symbol& PERMIT() {
    static internal::Symbol instance("PERMIT");
    return instance;
}

// Line 392: private val TAKEN = Symbol("TAKEN")
inline internal::Symbol& TAKEN() {
    static internal::Symbol instance("TAKEN");
    return instance;
}

// Line 393: private val BROKEN = Symbol("BROKEN")
inline internal::Symbol& BROKEN() {
    static internal::Symbol instance("BROKEN");
    return instance;
}

// Line 394: private val CANCELLED = Symbol("CANCELLED")
inline internal::Symbol& CANCELLED() {
    static internal::Symbol instance("CANCELLED");
    return instance;
}

// Line 395: private val SEGMENT_SIZE = systemProp("kotlinx.coroutines.semaphore.segmentSize", 16)
inline int SEGMENT_SIZE() {
    static int value = internal::system_prop_int("kotlinx.coroutines.semaphore.segmentSize", 16);
    return value;
}

// Line 390: private val MAX_SPIN_CYCLES = systemProp("kotlinx.coroutines.semaphore.maxSpinCycles", 100)
inline int MAX_SPIN_CYCLES() {
    static int value = internal::system_prop_int("kotlinx.coroutines.semaphore.maxSpinCycles", 100);
    return value;
}

/**
 * Line 361-389: SemaphoreSegment
 *
 * Segment class for the semaphore queue.
 * Each segment contains SEGMENT_SIZE slots for waiting acquirers.
 */
class SemaphoreSegment : public internal::Segment<SemaphoreSegment> {
public:
    // Line 362: val acquirers = atomicArrayOfNulls<Any?>(SEGMENT_SIZE)
    // We use a fixed-size array of atomic pointers
    static constexpr int MAX_SEGMENT_SIZE = 64; // Upper bound
    std::array<std::atomic<void*>, MAX_SEGMENT_SIZE> acquirers;
    int actual_segment_size;

    SemaphoreSegment(long id, SemaphoreSegment* prev, int pointers)
        : Segment<SemaphoreSegment>(id, prev, pointers)
        , actual_segment_size(SEGMENT_SIZE())
    {
        for (int i = 0; i < actual_segment_size; ++i) {
            acquirers[i].store(nullptr, std::memory_order_relaxed);
        }
    }

    // Line 363: override val numberOfSlots: Int get() = SEGMENT_SIZE
    int number_of_slots() const override {
        return actual_segment_size;
    }

    // Line 366: inline fun get(index: Int): Any? = acquirers[index].value
    void* get(int index) const {
        return acquirers[index].load(std::memory_order_acquire);
    }

    // Line 370: inline fun set(index: Int, value: Any?)
    void set(int index, void* value) {
        acquirers[index].store(value, std::memory_order_release);
    }

    // Line 374: inline fun cas(index: Int, expected: Any?, value: Any?): Boolean
    bool cas(int index, void* expected, void* value) {
        return acquirers[index].compare_exchange_strong(expected, value,
            std::memory_order_release, std::memory_order_relaxed);
    }

    // Line 377: inline fun getAndSet(index: Int, value: Any?)
    void* get_and_set(int index, void* value) {
        return acquirers[index].exchange(value, std::memory_order_acq_rel);
    }

    /**
     * Line 381-386: onCancellation
     *
     * Cleans the acquirer slot located by the specified index
     * and removes this segment physically if all slots are cleaned.
     */
    void on_cancellation(int index, std::exception_ptr cause,
        std::shared_ptr<CoroutineContext> context) override
    {
        // Line 383: set(index, CANCELLED)
        set(index, static_cast<void*>(&CANCELLED()));
        // Line 385: onSlotCleaned()
        on_slot_cleaned();
    }

    // Line 388
    std::string to_string() const {
        return "SemaphoreSegment[id=" + std::to_string(id) + "]";
    }
};

// Line 359: Factory function for creating segments
inline SemaphoreSegment* create_segment(long id, SemaphoreSegment* prev) {
    return new SemaphoreSegment(id, prev, 0);
}

} // namespace sync
} // namespace coroutines
} // namespace kotlinx
