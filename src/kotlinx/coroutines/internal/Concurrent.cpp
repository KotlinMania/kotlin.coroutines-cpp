/**
 * Transliterated from: kotlinx-coroutines-core/native/src/internal/Concurrent.kt
 */
// port-lint: source kotlinx-coroutines-core/native/src/internal/Concurrent.kt
#include "kotlinx/coroutines/internal/Concurrent.hpp"

namespace kotlinx::coroutines::internal {

// Transliterated from: kotlinx-coroutines-core/native/src/internal/Concurrent.kt:9-9
void with_lock(ReentrantLock& lock, std::function<void()> action) {
    std::lock_guard<ReentrantLock> guard(lock);
    action();
}

}  // namespace kotlinx::coroutines::internal
