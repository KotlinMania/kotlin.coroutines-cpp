#pragma once
/**
 * @file SemaphoreSegment.hpp
 * @brief Segment implementation for Semaphore and Mutex.
 *
 * Transliterated from: kotlinx-coroutines-core/common/src/sync/Semaphore.kt
 * Lines 361-396
 */

#include <atomic>
#include "kotlinx/coroutines/Waiter.hpp"
#include <memory>
#include <cassert>
#include "kotlinx/coroutines/internal/ConcurrentLinkedList.hpp"
#include "kotlinx/coroutines/internal/Symbol.hpp"
#include "kotlinx/coroutines/internal/SystemProps.hpp"

namespace kotlinx {
namespace coroutines {
namespace sync {

inline internal::Symbol& PERMIT() {
    static internal::Symbol instance("PERMIT");
    return instance;
}

inline internal::Symbol& TAKEN() {
    static internal::Symbol instance("TAKEN");
    return instance;
}

inline internal::Symbol& BROKEN() {
    static internal::Symbol instance("BROKEN");
    return instance;
}

inline internal::Symbol& CANCELLED() {
    static internal::Symbol instance("CANCELLED");
    return instance;
}

inline int SEGMENT_SIZE() {
    static int value = internal::system_prop_int("kotlinx.coroutines.semaphore.segmentSize", 16);
    return value;
}

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
    // NOTE(port): The atomic Kotlin reference retains the actual waiter, including
    // after dequeue and through tryResume/completeResume. Cell preserves that ownership.
    struct Cell {
        void* state;
        std::shared_ptr<Waiter> waiter;
    };

    // Transliterated from: kotlinx-coroutines-core/common/src/sync/Semaphore.kt:361-363
    SemaphoreSegment(long id, SemaphoreSegment* prev, int pointers);
    // Transliterated from: kotlinx-coroutines-core/common/src/sync/Semaphore.kt:363-363
    int number_of_slots() const override;
    // Transliterated from: kotlinx-coroutines-core/common/src/sync/Semaphore.kt:366-366
    void* get(int index) const;
    // Transliterated from: kotlinx-coroutines-core/common/src/sync/Semaphore.kt:369-371
    void set(int index, void* value);
    // Transliterated from: kotlinx-coroutines-core/common/src/sync/Semaphore.kt:374-374
    bool cas(int index, void* expected, void* value, std::shared_ptr<Waiter> waiter = {});
    // Transliterated from: kotlinx-coroutines-core/common/src/sync/Semaphore.kt:377-377
    std::shared_ptr<Cell> get_and_set(int index, void* value);
    // Transliterated from: kotlinx-coroutines-core/common/src/sync/Semaphore.kt:381-386
    void on_cancellation(int index, std::exception_ptr cause,
                         std::shared_ptr<CoroutineContext> context) override;
    // Transliterated from: kotlinx-coroutines-core/common/src/sync/Semaphore.kt:388-388
    std::string to_string() const override;

private:
    std::unique_ptr<std::shared_ptr<Cell>[]> acquirers_;
};

inline SemaphoreSegment* create_segment(long id, SemaphoreSegment* prev) {
    return new SemaphoreSegment(id, prev, 0);
}

} // namespace sync
} // namespace coroutines
} // namespace kotlinx
