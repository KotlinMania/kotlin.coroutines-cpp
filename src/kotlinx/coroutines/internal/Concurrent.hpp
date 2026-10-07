/**
 * Transliterated from: kotlinx-coroutines-core/native/src/internal/Concurrent.kt
 * Common extensions: kotlinx-coroutines-core/common/src/internal/Concurrent.common.kt
 */
#pragma once
// port-lint: source kotlinx-coroutines-core/native/src/internal/Concurrent.kt

#include <atomic>
#include <functional>
#include <mutex>
#include <unordered_set>
#include <utility>

namespace kotlinx::coroutines::internal {

// Transliterated from: kotlinx-coroutines-core/native/src/internal/Concurrent.kt:7-7
// NOTE(port): The Native synchronized object maps to the existing C++ recursive lock.
using ReentrantLock = std::recursive_mutex;

// Transliterated from: kotlinx-coroutines-core/native/src/internal/Concurrent.kt:9-9
template <typename Action>
decltype(auto) with_lock(ReentrantLock& lock, Action&& action) {
    std::lock_guard<ReentrantLock> guard(lock);
    return std::forward<Action>(action)();
}

// Transliterated from: kotlinx-coroutines-core/native/src/internal/Concurrent.kt:9-9
void with_lock(ReentrantLock& lock, std::function<void()> action);

// Transliterated from: kotlinx-coroutines-core/native/src/internal/Concurrent.kt:11-11
template <typename E>
std::unordered_set<E> identity_set(int expected_size) {
    // Native HashSet() does not use the expected size.
    return std::unordered_set<E>();
}

// Used only as a workaround for #3820 in StateFlow. Do not use elsewhere.
// Transliterated from: kotlinx-coroutines-core/native/src/internal/Concurrent.kt:15-28
// Transliterated from: kotlinx-coroutines-core/common/src/internal/Concurrent.common.kt:23-29
// NOTE(port): Reference values use the existing borrowed V* representation.
// Atomic access does not transfer ownership of their C++ objects.
template <typename V>
class WorkaroundAtomicReference {
public:
    // Transliterated from: kotlinx-coroutines-core/native/src/internal/Concurrent.kt:15-17
    explicit WorkaroundAtomicReference(V* value) : native_atomic_(value) {}

    // Transliterated from: kotlinx-coroutines-core/native/src/internal/Concurrent.kt:19-19
    V* get() const {
        return native_atomic_.load(std::memory_order_seq_cst);
    }

    // Transliterated from: kotlinx-coroutines-core/native/src/internal/Concurrent.kt:21-23
    void set(V* value) {
        native_atomic_.store(value, std::memory_order_seq_cst);
    }

    // Transliterated from: kotlinx-coroutines-core/native/src/internal/Concurrent.kt:25-25
    V* get_and_set(V* value) {
        return native_atomic_.exchange(value, std::memory_order_seq_cst);
    }

    // Transliterated from: kotlinx-coroutines-core/native/src/internal/Concurrent.kt:27-27
    bool compare_and_set(V* expected, V* value) {
        return native_atomic_.compare_exchange_strong(expected, value,
            std::memory_order_seq_cst, std::memory_order_seq_cst);
    }

private:
    // Transliterated from: kotlinx-coroutines-core/native/src/internal/Concurrent.kt:17-17
    // Native AtomicReference/Volatile accesses require sequential consistency.
    std::atomic<V*> native_atomic_;
};

// Transliterated from: kotlinx-coroutines-core/common/src/internal/Concurrent.common.kt:31-33
template <typename T>
T* get_value(const WorkaroundAtomicReference<T>& ref) {
    return ref.get();
}

// Transliterated from: kotlinx-coroutines-core/common/src/internal/Concurrent.common.kt:31-34
template <typename T>
void set_value(WorkaroundAtomicReference<T>& ref, T* value) {
    ref.set(value);
}

// Transliterated from: kotlinx-coroutines-core/common/src/internal/Concurrent.common.kt:36-40
template <typename T, typename Action>
void loop(WorkaroundAtomicReference<T>& ref, Action action) {
    while (true) {
        action(ref, get_value(ref));
    }
}

}  // namespace kotlinx::coroutines::internal
