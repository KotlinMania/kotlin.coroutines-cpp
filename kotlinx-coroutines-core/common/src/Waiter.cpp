// Transliterated from Kotlin to C++ (first-pass, mechanical syntax mapping)
// Original: kotlinx-coroutines-core/common/src/Waiter.kt
//
// TODO:
// - Segment template parameter needs internal infrastructure
// - CancellableContinuationImpl and SelectInstance integration

namespace kotlinx {
namespace coroutines {

// Forward declaration
template<typename T> class Segment;

/**
 * All waiters (such as [CancellableContinuationImpl] and [SelectInstance]) in synchronization and
 * communication primitives, should implement this interface to make the code faster and easier to read.
 */
// internal
class Waiter {
public:
    /**
     * When this waiter is cancelled, [Segment.onCancellation] with
     * the specified [segment] and [index] should be called.
     * This function installs the corresponding cancellation handler.
     */
    template<typename T>
    virtual void invoke_on_cancellation(Segment<T>* segment, int index) = 0;

    virtual ~Waiter() = default;
};

} // namespace coroutines
} // namespace kotlinx
