#pragma once
/**
 * Transliterated from: kotlinx-coroutines-core/common/src/internal/Symbol.kt
 *
 * Kotlin file header (translated):
 *   package kotlinx.coroutines.internal
 */

#include <string>
#include <utility>

namespace kotlinx::coroutines::internal {

/**
 * A symbol class that is used to define unique constants that are self-explanatory in debugger.
 *
 * @suppress **This is unstable API and it is subject to change.**
 *
 * Upstream:
 *   internal class Symbol(@JvmField val symbol: String) {
 *       override fun toString(): String = "<$symbol>"
 *       @Suppress("UNCHECKED_CAST", "NOTHING_TO_INLINE")
 *       inline fun <T> unbox(value: Any?): T = if (value === this) null as T else value as T
 *   }
 */
class Symbol {
public:
    const std::string symbol;

    explicit Symbol(std::string symbol_value) : symbol(std::move(symbol_value)) {}

    /** Upstream: override fun toString(): String = "<$symbol>" */
    std::string to_string() const { return "<" + symbol + ">"; }

    /**
     * Upstream: inline fun <T> unbox(value: Any?): T =
     *              if (value === this) null as T else value as T
     *
     * In the C++ port `Any?` is modelled as `void*`. Identity equality (`===`) maps to
     * pointer equality against `this`. The `null as T` branch returns a default-constructed
     * `T`; the value branch casts the pointer payload to the requested concrete type.
     */
    template <typename T>
    T unbox(const void* value) const {
        if (value == this) return T{};
        return static_cast<T>(const_cast<void*>(value));
    }
};

} // namespace kotlinx::coroutines::internal
