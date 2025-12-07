// Transliterated from Kotlin to C++
// Original: kotlinx-coroutines-core/common/src/CloseableCoroutineDispatcher.kt
//
// TODO: expect/actual mechanism not available in C++ - use virtual/abstract or platform-specific includes
// TODO: AutoCloseable interface maps to C++ RAII or explicit close() method

namespace kotlinx {
namespace coroutines {

/**
 * [CoroutineDispatcher] that provides a method to close it,
 * causing the rejection of any new tasks and cleanup of all underlying resources
 * associated with the current dispatcher.
 * Examples of closeable dispatchers are dispatchers backed by `java.lang.Executor` and
 * by `kotlin.native.Worker`.
 *
 * **The `CloseableCoroutineDispatcher` class is not stable for inheritance in 3rd party libraries**, as new methods
 * might be added to this interface in the future, but is stable for use.
 */
// TODO: @ExperimentalCoroutinesApi - no C++ equivalent
// TODO: expect abstract class - platform-specific, use pure virtual
// TODO: AutoCloseable - C++ RAII or explicit close method
class CloseableCoroutineDispatcher : public CoroutineDispatcher {
public:
    // Default constructor
    CloseableCoroutineDispatcher() {}

    // Virtual destructor for proper cleanup
    virtual ~CloseableCoroutineDispatcher() {}

    /**
     * Initiate the closing sequence of the coroutine dispatcher.
     * After a successful call to [close], no new tasks will be accepted to be [dispatched][dispatch].
     * The previously-submitted tasks will still be run, but [close] is not guaranteed to wait for them to finish.
     *
     * Invocations of `close` are idempotent and thread-safe.
     */
    // TODO: abstract override fun - pure virtual
    virtual void close() = 0;
};

} // namespace coroutines
} // namespace kotlinx
