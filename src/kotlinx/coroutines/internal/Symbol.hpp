#pragma once
// port-lint: source internal/Symbol.kt
/** Transliterated from: kotlinx-coroutines-core/common/src/internal/Symbol.kt */
#include <any>
#include <string>
#include <type_traits>

namespace kotlinx::coroutines::internal {
/**
 * A symbol class used to define unique constants that are self-explanatory in a debugger.
 * This is an unstable API and is subject to change.
 */
// Transliterated from: kotlinx-coroutines-core/common/src/internal/Symbol.kt:10-15
class Symbol {
public:
    const std::string symbol;

    // Transliterated from: kotlinx-coroutines-core/common/src/internal/Symbol.kt:10-10
    explicit Symbol(std::string symbol_value);
    // Transliterated from: kotlinx-coroutines-core/common/src/internal/Symbol.kt:11-11
    std::string to_string() const;

    // Transliterated from: kotlinx-coroutines-core/common/src/internal/Symbol.kt:14-14
    // NOTE(port): This specialization uses the library's existing erased value
    // carrier. Empty std::any represents nullable Any's null value.
    std::any unbox(std::any value) const;

    // Transliterated from: kotlinx-coroutines-core/common/src/internal/Symbol.kt:14-14
    // NOTE(port): The raw erased ABI restores a nullable pointer type at the call site.
    template <typename T>
    T unbox(const void* value) const {
        static_assert(std::is_pointer_v<T>, "Raw Symbol unbox requires a nullable pointer type");
        if (value == this) return nullptr;
        return static_cast<T>(const_cast<void*>(value));
    }
};
} // namespace kotlinx::coroutines::internal
