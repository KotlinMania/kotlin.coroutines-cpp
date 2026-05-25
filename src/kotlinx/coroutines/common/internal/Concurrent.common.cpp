// port-lint: source internal/Concurrent.common.kt
/**
 * Transliterated from: kotlinx-coroutines-core/common/src/internal/Concurrent.common.kt
 *
 * Kotlin file header (translated):
 *   package kotlinx.coroutines.internal
 *
 * Upstream declares five `expect` shapes:
 *
 *   internal expect class ReentrantLock() { fun tryLock(): Boolean; fun unlock() }
 *   internal expect inline fun <T> ReentrantLock.withLock(action: () -> T): T
 *   internal expect fun <E> identitySet(expectedSize: Int): MutableSet<E>
 *   @OptionalExpectation @Target(FIELD) internal expect annotation class BenignDataRace()
 *   internal expect class WorkaroundAtomicReference<V>(value: V) { ... }
 *
 * The C++ port resolves each `expect` to a real std-backed implementation. `BenignDataRace`
 * is a JVM/native code-generation annotation with no C++ equivalent; in C++ memory ordering
 * is expressed on the access site via std::atomic and std::memory_order, so the annotation
 * has no companion declaration here.
 */

#include <atomic>
#include <cstddef>
#include <mutex>
#include <unordered_set>
#include <utility>

namespace kotlinx::coroutines::internal {

/**
 * Upstream:
 *   internal expect class ReentrantLock() { fun tryLock(): Boolean; fun unlock() }
 */
class ReentrantLock {
public:
    ReentrantLock() = default;
    ReentrantLock(const ReentrantLock&) = delete;
    ReentrantLock& operator=(const ReentrantLock&) = delete;

    bool try_lock() { return mutex_.try_lock(); }
    void unlock() { mutex_.unlock(); }
    void lock() { mutex_.lock(); }

private:
    std::recursive_mutex mutex_;
};

/**
 * Upstream:
 *   internal expect inline fun <T> ReentrantLock.withLock(action: () -> T): T
 *
 * The Kotlin `inline fun` becomes a C++ function template — the call site still pays no
 * abstraction cost beyond the std::lock_guard itself.
 */
template <typename T, typename Action>
inline T with_lock(ReentrantLock& lock, Action action) {
    std::lock_guard<ReentrantLock> guard(lock);
    return action();
}

/**
 * Upstream:
 *   internal expect fun <E> identitySet(expectedSize: Int): MutableSet<E>
 *
 * Identity comparison on Kotlin's JVM uses an IdentityHashMap-backed set; on K/N the
 * default `==` is already reference equality. In C++ the "identity set" is a hash set of
 * raw pointers with the default pointer-hash.
 */
template <typename E>
inline std::unordered_set<E*> identity_set(std::size_t expected_size) {
    std::unordered_set<E*> set;
    set.reserve(expected_size);
    return set;
}

/**
 * Upstream:
 *   internal expect class WorkaroundAtomicReference<V>(value: V) {
 *       fun get(): V
 *       fun set(value: V)
 *       fun getAndSet(value: V): V
 *       fun compareAndSet(expected: V, value: V): Boolean
 *   }
 *
 * Used as a StateFlow workaround for kotlinx.coroutines#3820. The compare-and-set semantics
 * are acquire/release on a value-equality match — StateFlow's call sites rely on the
 * happens-before edge between the write that publishes the value and the read that
 * observes it.
 */
template <typename V>
class WorkaroundAtomicReference {
public:
    explicit WorkaroundAtomicReference(V value) : slot_(std::move(value)) {}

    V get() const {
        std::lock_guard<std::mutex> guard(mutex_);
        return slot_;
    }

    void set(V value) {
        std::lock_guard<std::mutex> guard(mutex_);
        slot_ = std::move(value);
    }

    V get_and_set(V value) {
        std::lock_guard<std::mutex> guard(mutex_);
        V previous = std::move(slot_);
        slot_ = std::move(value);
        return previous;
    }

    bool compare_and_set(const V& expected, V value) {
        std::lock_guard<std::mutex> guard(mutex_);
        if (!(slot_ == expected)) return false;
        slot_ = std::move(value);
        return true;
    }

private:
    mutable std::mutex mutex_;
    V slot_;
};

/**
 * Upstream:
 *   internal var <T> WorkaroundAtomicReference<T>.value: T
 *       get() = this.get(); set(value) = this.set(value)
 */
template <typename T>
inline T get_value(const WorkaroundAtomicReference<T>& ref) {
    return ref.get();
}

template <typename T>
inline void set_value(WorkaroundAtomicReference<T>& ref, T value) {
    ref.set(std::move(value));
}

/**
 * Upstream:
 *   internal inline fun <T> WorkaroundAtomicReference<T>.loop(
 *       action: WorkaroundAtomicReference<T>.(value: T) -> Unit
 *   ) {
 *       while (true) { action(value) }
 *   }
 */
template <typename T, typename Action>
inline void loop(WorkaroundAtomicReference<T>& ref, Action action) {
    while (true) {
        action(ref, get_value(ref));
    }
}

} // namespace kotlinx::coroutines::internal
