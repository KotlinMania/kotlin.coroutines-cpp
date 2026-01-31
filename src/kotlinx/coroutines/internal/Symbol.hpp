#pragma once

/**
 * Transliterated from: kotlinx-coroutines-core/common/src/internal/Symbol.kt
 */

#include <string>

namespace kotlinx::coroutines::internal {

/**
 * A symbol class that is used to define unique constants that are self-explanatory in debugger.
 *
 * @suppress **This is unstable API and it is subject to change.**
 */
class Symbol {
public:
    const std::string symbol;

    Symbol(std::string symbol) : symbol(std::move(symbol)) {}

    std::string to_string() const {
        return "<" + symbol + ">";
    }

    template <typename T>
    T unbox(const void* value) const {
        if (value == this) {
            return nullptr;
        }
        return static_cast<T>(const_cast<void*>(value));
    }
    
    // TODO(port): Implement proper T unbox logic matching Kotlin's semantics if needed.
    // The C++ version uses void* for 'Any?', so comparison is pointer identity.
};

} // namespace kotlinx::coroutines::internal
