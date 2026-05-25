/**
 * Transliterated from: kotlinx-coroutines-core/native/src/Debug.kt
 *
 * Kotlin file header (translated):
 *   package kotlinx.coroutines
 *
 * Upstream:
 *   internal actual val DEBUG: Boolean = false
 *   internal actual val Any.hexAddress: String
 *       get() = identityHashCode().toUInt().toString(16)
 *   internal actual val Any.classSimpleName: String
 *       get() = this::class.simpleName ?: "Unknown"
 *   internal actual inline fun assert(value: () -> Boolean) {}
 *
 * K/N evaluates `DEBUG = false` at runtime. The C++ port mirrors all four, using
 * `typeid(...).name()` for `class_simple_name` (matches Kotlin's `KClass.simpleName` as
 * closely as the host RTTI allows) and the object address for `hex_address`. The
 * `assert(predicate)` overload is intentionally a no-op (upstream K/N is too — debug
 * assertions ship only on the JVM target).
 */

#include <cstdint>
#include <functional>
#include <sstream>
#include <string>
#include <typeinfo>

namespace kotlinx::coroutines {

constexpr bool DEBUG = false;

template <typename T>
std::string hex_address(const T& obj) {
    auto address = reinterpret_cast<std::uintptr_t>(&obj);
    std::ostringstream out;
    out << std::hex << address;
    return out.str();
}

template <typename T>
std::string class_simple_name(const T& obj) {
    const char* name = typeid(obj).name();
    return name ? name : "Unknown";
}

inline void assert_predicate(const std::function<bool()>& /*value*/) {}

} // namespace kotlinx::coroutines
