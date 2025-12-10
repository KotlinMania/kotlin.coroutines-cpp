/**
 * @file Waiter.cpp
 * @brief Waiter interface for synchronization primitives
 *
 * Transliterated from: kotlinx-coroutines-core/common/src/Waiter.kt
 *
 * TODO:
 * - Segment template parameter needs infrastructure
 * - CancellableContinuationImpl and SelectInstance integration
 */

namespace kotlinx {
namespace coroutines {

// Forward declaration
class SegmentBase;

/**
 * All waiters (such as CancellableContinuationImpl and SelectInstance) in synchronization and
 * communication primitives, should implement this interface to make the code faster and easier to read.
 *
 * Note: In C++ we use type-erased SegmentBase instead of templated Segment<T> because
 * C++ doesn't allow virtual template functions.
 */
class Waiter {
public:
    /**
     * When this waiter is cancelled, Segment::on_cancellation() with
     * the specified segment and index should be called.
     * This function installs the corresponding cancellation handler.
     *
     * TODO: In Kotlin this is a generic function with Segment<*> parameter.
     * In C++ we use type-erased SegmentBase.
     */
    virtual void invoke_on_cancellation(SegmentBase* segment, int index) = 0;

    virtual ~Waiter() = default;
};

} // namespace coroutines
} // namespace kotlinx
