#pragma once
/**
 * @file Concurrent.hpp
 * @brief Concurrent utilities for kotlinx.coroutines
 *
 * Transliterated from: kotlinx-coroutines-core/native/src/internal/Concurrent.kt
 *
 * Platform-specific concurrency primitives including:
 * - ReentrantLock (mutex wrapper)
 * - WorkaroundAtomicReference (atomic reference wrapper)
 * - identitySet factory
 */

#include <mutex>
#include <atomic>
#include <unordered_set>
#include <functional>

namespace kotlinx {
namespace coroutines {
namespace internal {

/**
 * ReentrantLock - wraps std::recursive_mutex for reentrant locking
 * Kotlin: internal actual typealias ReentrantLock = kotlinx.atomicfu.locks.SynchronizedObject
 */
using ReentrantLock = std::recursive_mutex;

/**
 * withLock - executes action while holding the lock
 * Kotlin: internal actual inline fun <T> ReentrantLock.withLock(action: () -> T): T
 */
template<typename T>
T with_lock(ReentrantLock& lock, std::function<T()> action) {
    std::lock_guard<ReentrantLock> guard(lock);
    return action();
}

// Overload for void return
inline void with_lock(ReentrantLock& lock, std::function<void()> action) {
    std::lock_guard<ReentrantLock> guard(lock);
    action();
}

/**
 * Creates an identity-based mutable set
 * Kotlin: internal actual fun <E> identitySet(expectedSize: Int): MutableSet<E>
 */
template<typename E>
std::unordered_set<E> identity_set(int expected_size = 16) {
    std::unordered_set<E> set;
    set.reserve(expected_size);
    return set;
}

/**
 * BenignDataRace - marker for intentional data races (no-op in C++)
 * Kotlin: internal actual typealias BenignDataRace = kotlin.concurrent.Volatile
 */
// In C++, use std::atomic for volatile-like semantics

/**
 * WorkaroundAtomicReference - atomic reference wrapper
 * Kotlin: internal actual class WorkaroundAtomicReference<V>
 *
 * Provides atomic operations on reference types using std::atomic<T*>
 */
template<typename V>
class WorkaroundAtomicReference {
private:
    std::atomic<V*> native_atomic;

public:
    explicit WorkaroundAtomicReference(V* value) : native_atomic(value) {}

    V* get() const {
        return native_atomic.load(std::memory_order_acquire);
    }

    void set(V* value) {
        native_atomic.store(value, std::memory_order_release);
    }

    V* get_and_set(V* value) {
        return native_atomic.exchange(value, std::memory_order_acq_rel);
    }

    bool compare_and_set(V* expected, V* value) {
        return native_atomic.compare_exchange_strong(expected, value,
            std::memory_order_acq_rel, std::memory_order_acquire);
    }
};

} // namespace internal
} // namespace coroutines
} // namespace kotlinx
