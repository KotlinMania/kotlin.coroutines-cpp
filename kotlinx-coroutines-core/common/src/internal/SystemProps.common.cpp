#include "kotlinx/coroutines/core_fwd.hpp"
// Transliterated from Kotlin to C++
// Original: kotlinx-coroutines-core/common/src/internal/SystemProps.common.kt
//
// TODO: This is a mechanical transliteration - semantics not fully implemented
// TODO: @JvmName, @JvmMultifileClass annotations - JVM-specific
// TODO: expect function needs platform-specific implementation
// TODO: std::string* (nullable) needs std::optional<std::string> or char*

#include <string>
#include <optional>
#include <stdexcept>
#include <climits>

namespace kotlinx {
namespace coroutines {
namespace {

/**
 * Gets the system property indicated by the specified [property name][propertyName],
 * or returns [defaultValue] if there is no property with that key.
 *
 * **Note: this function should be used in JVM tests only, other platforms use the default value.**
 */
inline bool system_prop(const std::string& property_name, bool default_value) {
    std::optional<std::string> value = system_prop(property_name);
    if (!value.has_value()) return default_value;
    // TODO: to_boolean() conversion
    return default_value;
}

/**
 * Gets the system property indicated by the specified [property name][propertyName],
 * or returns [defaultValue] if there is no property with that key. It also checks that the result
 * is between [minValue] and [maxValue] (inclusively), throws [IllegalStateException] if it is not.
 *
 * **Note: this function should be used in JVM tests only, other platforms use the default value.**
 */
inline int system_prop(
    const std::string& property_name,
    int default_value,
    int min_value = 1,
    int max_value = INT_MAX
) {
    return static_cast<int>(system_prop(property_name, static_cast<long>(default_value),
                                       static_cast<long>(min_value), static_cast<long>(max_value)));
}

/**
 * Gets the system property indicated by the specified [property name][propertyName],
 * or returns [defaultValue] if there is no property with that key. It also checks that the result
 * is between [minValue] and [maxValue] (inclusively), throws [IllegalStateException] if it is not.
 *
 * **Note: this function should be used in JVM tests only, other platforms use the default value.**
 */
inline long system_prop(
    const std::string& property_name,
    long default_value,
    long min_value = 1,
    long max_value = LONG_MAX
) {
    std::optional<std::string> value = system_prop(property_name);
    if (!value.has_value()) return default_value;

    // TODO: to_long_or_null() conversion
    long parsed = default_value; // placeholder
    try {
        parsed = std::stol(value.value());
    } catch (...) {
        throw std::runtime_error("System property '" + property_name + "' has unrecognized value '" + value.value() + "'");
    }

    if (parsed < min_value || parsed > max_value) {
        throw std::runtime_error("System property '" + property_name + "' should be in range " +
                               std::to_string(min_value) + ".." + std::to_string(max_value) +
                               ", but is '" + std::to_string(parsed) + "'");
    }
    return parsed;
}

/**
 * Gets the system property indicated by the specified [property name][propertyName],
 * or returns [defaultValue] if there is no property with that key.
 *
 * **Note: this function should be used in JVM tests only, other platforms use the default value.**
 */
inline std::string system_prop(const std::string& property_name, const std::string& default_value) {
    std::optional<std::string> value = system_prop(property_name);
    return value.value_or(default_value);
}

/**
 * Gets the system property indicated by the specified [property name][propertyName],
 * or returns `nullptr` if there is no property with that key.
 *
 * **Note: this function should be used in JVM tests only, other platforms use the default value.**
 */
// TODO: expect function - needs platform-specific implementation
std::optional<std::string> system_prop(const std::string& property_name);

} // namespace internal
} // namespace coroutines
} // namespace kotlinx
