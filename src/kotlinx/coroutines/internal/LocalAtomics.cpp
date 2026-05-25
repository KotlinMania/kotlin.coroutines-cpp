/**
 * Transliterated from: kotlinx-coroutines-core/native/src/internal/LocalAtomics.kt
 *
 * Kotlin file header (translated):
 *   package kotlinx.coroutines.internal
 *
 * Upstream:
 *   internal actual class LocalAtomicInt actual constructor(value: Int) {
 *       private var value = value
 *       actual fun get() = value
 *       actual fun set(value: Int) { this.value = value }
 *       actual fun decrementAndGet() = --value
 *   }
 *
 * The native upstream actually carries the value in a plain `var` because Kotlin/Native
 * is single-threaded by default at the time these queues access it. The C++ port uses
 * std::atomic<int> with relaxed ordering — same observable semantics, plus safety if a
 * future migration moves these into a multi-threaded path.
 */

#include <atomic>

namespace kotlinx::coroutines::internal {

class LocalAtomicInt {
public:
    explicit LocalAtomicInt(int value) : value_(value) {}

    void set(int value) { value_.store(value, std::memory_order_relaxed); }
    int get() const { return value_.load(std::memory_order_relaxed); }
    int decrement_and_get() {
        return value_.fetch_sub(1, std::memory_order_relaxed) - 1;
    }

private:
    std::atomic<int> value_;
};

} // namespace kotlinx::coroutines::internal
