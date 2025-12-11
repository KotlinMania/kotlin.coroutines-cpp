#pragma once

namespace kotlinx {
namespace coroutines {

/**
 * All waiters (such as CancellableContinuationImpl and SelectInstance) in synchronization and
 * communication primitives, should implement this interface to make the code faster and easier to read.
 */
struct Waiter {
    virtual ~Waiter() = default;
    
    /**
     * When this waiter is cancelled, Segment.onCancellation with
     * the specified segment and index should be called.
     * This function installs the corresponding cancellation handler.
     * 
     * Note: "segment" is type-erased to void* here to avoid circular dependency with Segment class.
     * It effectively corresponds to Segment<*> in Kotlin.
     */
    virtual void invoke_on_cancellation(void* segment, int index) = 0;
};

} // namespace coroutines
} // namespace kotlinx
