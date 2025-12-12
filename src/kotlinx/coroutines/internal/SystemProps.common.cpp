/**
 * @file SystemProps.common.cpp
 * @brief System property access utilities
 *
 * Transliterated from: kotlinx-coroutines-core/common/src/internal/SystemProps.common.kt
 *
 * Note: System properties are primarily used in JVM tests.
 * Other platforms typically use default values.
 */

#include <string>
#include <optional>
#include <stdexcept>
#include <climits>
#include <cstdlib>

namespace kotlinx {
    namespace coroutines {
        namespace internal {
            /**
 * Gets the system property indicated by the specified property name,
 * or returns nullopt if there is no property with that key.
 *
 * **Note: this function should be used in JVM tests only, other platforms use the default value.**
 */
            std::optional<std::string> system_prop(const std::string &property_name) {
                // On non-JVM platforms, return nullopt to use defaults
                // Could check environment variables as a fallback
                const char *env_value = std::getenv(property_name.c_str());
                if (env_value != nullptr) {
                    return std::string(env_value);
                }
                return std::nullopt;
            }

            /**
 * Gets the system property indicated by the specified property name,
 * or returns defaultValue if there is no property with that key.
 */
            bool system_prop_bool(const std::string &property_name, bool default_value) {
                std::optional<std::string> value = system_prop(property_name);
                if (!value.has_value()) return default_value;

                const std::string &str = value.value();
                if (str == "true" || str == "1" || str == "yes") return true;
                if (str == "false" || str == "0" || str == "no") return false;
                return default_value;
            }

            /**
 * Gets the system property indicated by the specified property name,
 * or returns defaultValue if there is no property with that key.
 */
            long system_prop_long(
                const std::string &property_name,
                long default_value,
                long min_value = 1,
                long max_value = LONG_MAX
            ) {
                std::optional<std::string> value = system_prop(property_name);
                if (!value.has_value()) return default_value;

                long parsed = default_value;
                try {
                    parsed = std::stol(value.value());
                } catch (...) {
                    throw std::runtime_error(
                        "System property '" + property_name + "' has unrecognized value '" + value.value() + "'");
                }

                if (parsed < min_value || parsed > max_value) {
                    throw std::runtime_error("System property '" + property_name + "' should be in range " +
                                             std::to_string(min_value) + ".." + std::to_string(max_value) +
                                             ", but is '" + std::to_string(parsed) + "'");
                }
                return parsed;
            }

            /**
 * Gets the system property indicated by the specified property name,
 * or returns defaultValue if there is no property with that key.
 */
            int system_prop_int(
                const std::string &property_name,
                int default_value,
                int min_value = 1,
                int max_value = INT_MAX
            ) {
                return static_cast<int>(system_prop_long(property_name, static_cast<long>(default_value),
                                                         static_cast<long>(min_value), static_cast<long>(max_value)));
            }

            /**
 * Gets the system property indicated by the specified property name,
 * or returns defaultValue if there is no property with that key.
 */
            std::string system_prop_string(const std::string &property_name, const std::string &default_value) {
                std::optional<std::string> value = system_prop(property_name);
                return value.value_or(default_value);
            }
        } // namespace internal
    } // namespace coroutines
} // namespace kotlinx