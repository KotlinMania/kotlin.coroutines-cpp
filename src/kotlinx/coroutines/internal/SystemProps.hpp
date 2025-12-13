#pragma once
/**
 * @file SystemProps.hpp
 * @brief System property access utilities (common declarations).
 *
 * Transliterated from: kotlinx-coroutines-core/common/src/internal/SystemProps.common.kt
 */

#include <climits>
#include <optional>
#include <string>

namespace kotlinx {
namespace coroutines {
namespace internal {

// Kotlin: internal fun systemProp(propertyName: String): String?
std::optional<std::string> system_prop(const std::string& property_name);

// Kotlin: internal fun systemProp(propertyName: String, defaultValue: Boolean): Boolean
bool system_prop_bool(const std::string& property_name, bool default_value);

// Kotlin: internal fun systemProp(propertyName: String, defaultValue: Long, minValue: Long, maxValue: Long): Long
long system_prop_long(
    const std::string& property_name,
    long default_value,
    long min_value = 1,
    long max_value = LONG_MAX
);

// Kotlin: internal fun systemProp(propertyName: String, defaultValue: Int, minValue: Int, maxValue: Int): Int
int system_prop_int(
    const std::string& property_name,
    int default_value,
    int min_value = 1,
    int max_value = INT_MAX
);

// Kotlin: internal fun systemProp(propertyName: String, defaultValue: String): String
std::string system_prop_string(const std::string& property_name, const std::string& default_value);

} // namespace internal
} // namespace coroutines
} // namespace kotlinx

