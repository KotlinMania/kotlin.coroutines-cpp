/**
 * Transliterated from: kotlinx-coroutines-core/concurrent/src/internal/OnDemandAllocatingPool.kt
 */
// port-lint: source kotlinx-coroutines-core/concurrent/src/internal/OnDemandAllocatingPool.kt
#include "kotlinx/coroutines/concurrent/internal/OnDemandAllocatingPool.hpp"

namespace kotlinx::coroutines::internal {
namespace {
// Transliterated from: kotlinx-coroutines-core/concurrent/src/internal/OnDemandAllocatingPool.kt:102-102
// NOTE(port): Unsigned storage preserves Kotlin Int bit patterns without signed-shift overflow.
constexpr std::uint32_t IS_CLOSED_MASK = std::uint32_t{1} << 31;

// KT-25023
// Transliterated from: kotlinx-coroutines-core/concurrent/src/internal/OnDemandAllocatingPool.kt:96-100
// NOTE(port): Nonlocal returns in the source inline call sites expand to the
// allocation/close loops in the header; this is the private helper's own body.
[[noreturn]] void loop(std::function<void()> block) {
    while (true) {
        block();
    }
}
}

// Transliterated from: kotlinx-coroutines-core/concurrent/src/internal/OnDemandAllocatingPool.kt:37-37
bool pool_is_closed(std::uint32_t value) {
    return (value & IS_CLOSED_MASK) != 0;
}

// Transliterated from: kotlinx-coroutines-core/concurrent/src/internal/OnDemandAllocatingPool.kt:29-34
std::uint32_t forbid_new_pool_elements(std::atomic<std::uint32_t>& control_state) {
    while (true) {
        auto current = control_state.load();
        if (pool_is_closed(current)) return 0;
        if (control_state.compare_exchange_strong(current, current | IS_CLOSED_MASK)) return current;
    }
}

// Transliterated from: kotlinx-coroutines-core/concurrent/src/internal/OnDemandAllocatingPool.kt:85-90
std::string pool_state_string(const std::vector<std::string>& elements, bool closed) {
    std::string elements_str = "[";
    for (std::size_t i = 0; i < elements.size(); ++i) {
        if (i) elements_str += ", ";
        elements_str += elements[i];
    }
    elements_str += "]";
    return elements_str + (closed ? "[closed]" : "");
}
} // namespace kotlinx::coroutines::internal
