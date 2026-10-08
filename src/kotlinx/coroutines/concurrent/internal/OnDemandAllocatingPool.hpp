/**
 * Transliterated from: kotlinx-coroutines-core/concurrent/src/internal/OnDemandAllocatingPool.kt
 */
#pragma once
// port-lint: source kotlinx-coroutines-core/concurrent/src/internal/OnDemandAllocatingPool.kt
#include <atomic>
#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

namespace kotlinx::coroutines::internal {
// Transliterated from: kotlinx-coroutines-core/concurrent/src/internal/OnDemandAllocatingPool.kt:29-37
std::uint32_t forbid_new_pool_elements(std::atomic<std::uint32_t>& control_state);
// Transliterated from: kotlinx-coroutines-core/concurrent/src/internal/OnDemandAllocatingPool.kt:37-37
bool pool_is_closed(std::uint32_t value);
// Transliterated from: kotlinx-coroutines-core/concurrent/src/internal/OnDemandAllocatingPool.kt:85-89
std::string pool_state_string(const std::vector<std::string>& elements, bool closed);

/**
 * A thread-safe resource pool.
 * [maxCapacity] is the maximum amount of elements.
 * [create] is the function that creates a new element.
 * This is only used in the Native implementation, but is part of the
 * concurrent source set in order to test it on the JVM.
 */
// Transliterated from: kotlinx-coroutines-core/concurrent/src/internal/OnDemandAllocatingPool.kt:14-93
// NOTE(port): T bindings must be instantiable by consumers, so their algorithms
// live in the header. The control-bit and list-formatting helpers are concrete.
// Atomic shared slots retain published value boxes during concurrent reads;
// reference-valued T keeps its existing borrowed or owning representation.
template<typename T>
class OnDemandAllocatingPool {
public:
    // Transliterated from: kotlinx-coroutines-core/concurrent/src/internal/OnDemandAllocatingPool.kt:14-23
    OnDemandAllocatingPool(int max_capacity, std::function<T(int)> create)
        : max_capacity_(max_capacity), create_(std::move(create)), elements_(max_capacity) {}

    /**
     * Request that a new element is created. Returns false if the pool is closed.
     * Returns true even when no element is created because maxCapacity is reached.
     * Rethrows exceptions thrown by create.
     * The source reserves the slot before invoking create.
     */
    // Transliterated from: kotlinx-coroutines-core/concurrent/src/internal/OnDemandAllocatingPool.kt:48-57
    bool allocate() {
        while (true) {
            auto ctl = control_state_.load();
            if (is_closed(ctl)) return false;
            if (ctl >= static_cast<std::uint32_t>(max_capacity_)) return true;
            if (control_state_.compare_exchange_strong(ctl, ctl + 1)) {
                std::shared_ptr<const T> element = std::make_shared<const T>(create_(static_cast<int>(ctl)));
                std::atomic_store(&elements_[ctl], std::move(element));
                return true;
            }
        }
    }

    /**
     * Prevent creation of new elements and return all elements present in the pool.
     * Thread-safe; only the first close call returns a nonempty list.
     */
    // Transliterated from: kotlinx-coroutines-core/concurrent/src/internal/OnDemandAllocatingPool.kt:71-82
    std::vector<T> close() {
        auto elements_existing = try_forbid_new_elements();
        std::vector<T> result;
        result.reserve(elements_existing);
        for (std::uint32_t i = 0; i < elements_existing; ++i) {
            // Wait for the reserved element to be created before returning it.
            while (true) {
                auto element = std::atomic_exchange(&elements_[i], std::shared_ptr<const T>{});
                if (element) {
                    result.push_back(*element);
                    break;
                }
            }
        }
        return result;
    }

    // for tests
    // Transliterated from: kotlinx-coroutines-core/concurrent/src/internal/OnDemandAllocatingPool.kt:85-90
    std::string state_representation() const {
        auto ctl = control_state_.load();
        std::vector<std::string> elements;
        const auto count = ctl & ~(std::uint32_t{1} << 31);
        for (std::uint32_t i = 0; i < count; ++i) {
            auto element = std::atomic_load(&elements_[i]);
            if (!element) elements.push_back("null");
            else if constexpr (std::is_same_v<T, std::string>) elements.push_back(*element);
            else if constexpr (requires { element->to_string(); }) elements.push_back(element->to_string());
            else elements.push_back(std::to_string(*element));
        }
        return pool_state_string(elements, is_closed(ctl));
    }

    // Transliterated from: kotlinx-coroutines-core/concurrent/src/internal/OnDemandAllocatingPool.kt:92-92
    std::string to_string() const {
        return "OnDemandAllocatingPool(" + state_representation() + ")";
    }

private:
    // Transliterated from: kotlinx-coroutines-core/concurrent/src/internal/OnDemandAllocatingPool.kt:29-34
    std::uint32_t try_forbid_new_elements() {
        return forbid_new_pool_elements(control_state_);
    }
    // Transliterated from: kotlinx-coroutines-core/concurrent/src/internal/OnDemandAllocatingPool.kt:37-37
    static bool is_closed(std::uint32_t value) { return pool_is_closed(value); }

    const int max_capacity_;
    std::function<T(int)> create_;
    // Number of existing elements plus the isClosed flag in the highest bit.
    // Once the flag is set, the value is guaranteed not to change anymore.
    std::atomic<std::uint32_t> control_state_{0};
    std::vector<std::shared_ptr<const T>> elements_;
};
} // namespace kotlinx::coroutines::internal
