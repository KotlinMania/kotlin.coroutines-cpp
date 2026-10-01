#pragma once
// port-lint: source internal/Synchronized.common.kt
/**
 * @file SynchronizedObject.hpp
 * @brief Synchronization primitive for K/N compatibility
 *
 * Transliterated from: kotlinx-coroutines-core/common/src/internal/Synchronized.common.kt
 *
 * A basic synchronization object that can be used with std::lock_guard.
 */

#include <mutex>

namespace kotlinx {
namespace coroutines {
namespace internal {

/**
 * Synchronization object compatible with synchronized() blocks.
 * In C++, this is simply a std::mutex wrapper.
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
 * Executes the given block while holding the lock on this object.
 *
 * Transliterated from:
 * internal expect inline fun <T> synchronized(lock: SynchronizedObject, block: () -> T): T
 */
template<typename T, typename Block>
T synchronized(SynchronizedObject& lock, Block block) {
    std::lock_guard<SynchronizedObject> guard(lock);
    return block();
}

template<typename T, typename Block>
T synchronized(SynchronizedObject* lock, Block block) {
    std::lock_guard<SynchronizedObject> guard(*lock);
    return block();
}

template<typename T, typename Block>
inline T synchronized_impl(SynchronizedObject* lock, Block block) {
    return synchronized<T>(lock, std::move(block));
}

} // namespace internal
} // namespace coroutines
} // namespace kotlinx
