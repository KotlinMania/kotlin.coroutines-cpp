// Transliterated from Kotlin to C++
// Original: kotlinx-coroutines-core/common/src/internal/Symbol.kt
//
// TODO: This is a mechanical transliteration - semantics not fully implemented
// TODO: @JvmField annotation - JVM-specific, translate to comment
// TODO: Inline function with generic parameter needs template

#include <string>

namespace kotlinx {
namespace coroutines {
namespace internal {

/**
 * A symbol class that is used to define unique constants that are self-explanatory in debugger.
 *
 * @suppress **This is unstable API and it is subject to change.**
 */
class Symbol {
public:
    std::string symbol;

    explicit Symbol(const std::string& symbol) : symbol(symbol) {}

    std::string to_string() const {
        return "<" + symbol + ">";
    }

    // TODO: @Suppress("UNCHECKED_CAST", "NOTHING_TO_INLINE")
    template<typename T>
    T unbox(void* value) {
        if (value == this) {
            return static_cast<T>(nullptr);
        } else {
            return static_cast<T>(value);
        }
    }
};

} // namespace internal
} // namespace coroutines
} // namespace kotlinx
