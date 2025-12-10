#pragma once
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
    const std::string symbol;

    explicit Symbol(const std::string& symbol) : symbol(symbol) {}

    std::string to_string() const {
        return "<" + symbol + ">";
    }

    template<typename T>
    T unbox(void* value) const {
        if (value == (void*)this) {
            return static_cast<T>(nullptr);
        } else {
            return static_cast<T>(value);
        }
    }
};

} // namespace internal
} // namespace coroutines
} // namespace kotlinx
