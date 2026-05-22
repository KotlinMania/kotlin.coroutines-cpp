/**
 * Transliterated from: kotlinx-coroutines-core/common/src/internal/Synchronized.common.kt
 *
 * Kotlin file header (translated):
 *   package kotlinx.coroutines.internal
 *
 * Upstream:
 *   @InternalCoroutinesApi public expect open class SynchronizedObject()
 *   @InternalCoroutinesApi public expect inline fun <T> synchronizedImpl(
 *       lock: SynchronizedObject, block: () -> T): T
 *   @InternalCoroutinesApi public inline fun <T> synchronized(
 *       lock: SynchronizedObject, block: () -> T): T = synchronizedImpl(lock, block)
 *
 * The C++ port resolves both `expect`s: `SynchronizedObject` owns a recursive_mutex, and
 * `synchronized_impl` is a std::lock_guard scope. Kotlin's
 * `contract { callsInPlace(block, EXACTLY_ONCE) }` is an invocation-count hint with no
 * C++ equivalent — std::lock_guard already enforces exactly-once entry/exit dynamically.
 */

#include <mutex>
#include <utility>

namespace kotlinx::coroutines::internal {

/**
 * Upstream:
 *   @InternalCoroutinesApi public expect open class SynchronizedObject()
 */
class SynchronizedObject {
public:
    SynchronizedObject() = default;
    virtual ~SynchronizedObject() = default;

    SynchronizedObject(const SynchronizedObject&) = delete;
    SynchronizedObject& operator=(const SynchronizedObject&) = delete;

    void lock() { mutex_.lock(); }
    void unlock() { mutex_.unlock(); }
    bool try_lock() { return mutex_.try_lock(); }

private:
    std::recursive_mutex mutex_;
};

/**
 * Upstream:
 *   @InternalCoroutinesApi public expect inline fun <T> synchronizedImpl(
 *       lock: SynchronizedObject, block: () -> T): T
 */
template <typename T, typename Block>
inline T synchronized_impl(SynchronizedObject* lock, Block block) {
    std::lock_guard<SynchronizedObject> guard(*lock);
    return block();
}

/**
 * Upstream:
 *   @InternalCoroutinesApi public inline fun <T> synchronized(
 *       lock: SynchronizedObject, block: () -> T): T = synchronizedImpl(lock, block)
 */
template <typename T, typename Block>
inline T synchronized(SynchronizedObject* lock, Block block) {
    return synchronized_impl<T>(lock, std::move(block));
}

} // namespace kotlinx::coroutines::internal
