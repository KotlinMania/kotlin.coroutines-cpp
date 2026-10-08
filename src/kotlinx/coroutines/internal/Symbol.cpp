// port-lint: source internal/Symbol.kt
/** Transliterated from: kotlinx-coroutines-core/common/src/internal/Symbol.kt */
#include "kotlinx/coroutines/internal/Symbol.hpp"
#include <utility>

namespace kotlinx::coroutines::internal {
// Transliterated from: kotlinx-coroutines-core/common/src/internal/Symbol.kt:10-10
Symbol::Symbol(std::string symbol_value) : symbol(std::move(symbol_value)) {}

// Transliterated from: kotlinx-coroutines-core/common/src/internal/Symbol.kt:11-11
std::string Symbol::to_string() const { return "<" + symbol + ">"; }

// Transliterated from: kotlinx-coroutines-core/common/src/internal/Symbol.kt:14-14
std::any Symbol::unbox(std::any value) const {
    if (auto identity = std::any_cast<Symbol*>(&value); identity && *identity == this)
        return std::any{};
    if (auto identity = std::any_cast<const Symbol*>(&value); identity && *identity == this)
        return std::any{};
    return value;
}
} // namespace kotlinx::coroutines::internal
