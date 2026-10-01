#pragma once
// port-lint: source internal/LocalAtomics.common.kt
/**
 * @file LocalAtomics.hpp
 * @brief Local atomics for internal queue operations
 *
 * Transliterated from: kotlinx-coroutines-core/common/src/internal/LocalAtomics.common.kt
 *
 * Upstream:
 *   internal expect class LocalAtomicInt(value: Int) {
 *       fun get(): Int
 *       fun set(value: Int)
 *       fun decrementAndGet(): Int
 *   }
 */

#include <atomic>

namespace kotlinx::coroutines::internal {

class LocalAtomicInt {
public:
    explicit LocalAtomicInt(int value) : value_(value) {}

    int get() const { return value_.load(std::memory_order_relaxed); }
    void set(int value) { value_.store(value, std::memory_order_relaxed); }
    int decrement_and_get() {
        return value_.fetch_sub(1, std::memory_order_relaxed) - 1;
    }

private:
    std::atomic<int> value_;
};

} // namespace kotlinx::coroutines::internal
