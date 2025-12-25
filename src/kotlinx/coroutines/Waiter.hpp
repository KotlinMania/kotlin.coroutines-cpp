#pragma once

#include <memory>

namespace kotlinx {
namespace coroutines {
namespace internal {
    // Forward declaration - defined in ConcurrentLinkedList.hpp
    class SegmentBase;
} // namespace internal

/**
 * All waiters (such as CancellableContinuationImpl and SelectInstance) in synchronization and
 * communication primitives, should implement this interface to make the code faster and easier to read.
 *
 * Transliterated from: kotlinx-coroutines-core/common/src/Waiter.kt
 */
struct Waiter {
    virtual ~Waiter() = default;

    /**
     * When this waiter is cancelled, Segment.onCancellation with
     * the specified segment and index should be called.
     * This function installs the corresponding cancellation handler.
     *
     * Corresponds to Kotlin's Segment<*> parameter.
     */
    virtual void invoke_on_cancellation(internal::SegmentBase* segment, int index) = 0;

    /**
     * C++ lifetime management: Returns a shared_ptr to this waiter if it supports
     * shared ownership (via enable_shared_from_this). Returns nullptr otherwise.
     *
     * This is used by channel segments to keep waiters alive while they're stored
     * in segment state. In Kotlin, the GC handles this automatically.
     */
    virtual std::shared_ptr<Waiter> shared_from_this_waiter() { return nullptr; }
};

} // namespace coroutines
} // namespace kotlinx
