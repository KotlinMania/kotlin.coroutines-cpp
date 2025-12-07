// Transliterated from Kotlin to C++
// Original: kotlinx-coroutines-core/native/src/internal/Concurrent.kt
//
// TODO: actual typealias for platform types
// TODO: actual inline function
// TODO: kotlinx.atomicfu.locks API
// TODO: kotlin.concurrent.Volatile annotation
// TODO: kotlin.concurrent.AtomicReference

namespace kotlinx {
namespace coroutines {
namespace internal {

// TODO: Remove imports, fully qualify or add includes:
// import kotlinx.atomicfu.*
// import kotlinx.cinterop.*
// import kotlinx.atomicfu.locks.withLock as withLock2

// TODO: internal actual typealias
using ReentrantLock = kotlinx::atomicfu::locks::SynchronizedObject;

// TODO: internal actual inline function
template<typename T>
inline T with_lock(ReentrantLock& lock, std::function<T()> action) {
    // TODO: lock.withLock2(action)
    return action();
}

// TODO: internal actual function
template<typename E>
std::unordered_set<E> identity_set(int expected_size) {
    return std::unordered_set<E>();
}

// TODO: internal actual typealias
// TODO: kotlin.concurrent.Volatile - use std::atomic or volatile
using BenignDataRace = /* kotlin::concurrent::Volatile */ int; // placeholder

// TODO: internal actual class
template<typename V>
class WorkaroundAtomicReference {
private:
    std::atomic<V> native_atomic;

public:
    WorkaroundAtomicReference(V value) : native_atomic(value) {}

    V get() {
        return native_atomic.load();
    }

    void set(V value) {
        native_atomic.store(value);
    }

    V get_and_set(V value) {
        return native_atomic.exchange(value);
    }

    bool compare_and_set(V expected, V value) {
        return native_atomic.compare_exchange_strong(expected, value);
    }
};

} // namespace internal
} // namespace coroutines
} // namespace kotlinx
