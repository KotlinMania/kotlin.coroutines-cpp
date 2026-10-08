/**
 * Transliterated from: libraries/stdlib/src/kotlin/time/longSaturatedMath.kt
 */
#ifndef KOTLIN_TIME_LONG_SATURATED_MATH_HPP_
#define KOTLIN_TIME_LONG_SATURATED_MATH_HPP_
#include "kotlin/time/Duration.hpp"

namespace kotlin::time {
// Transliterated from: libraries/stdlib/src/kotlin/time/longSaturatedMath.kt:12-26
long long saturating_add(long long value, DurationUnit unit, Duration duration);
// Transliterated from: libraries/stdlib/src/kotlin/time/longSaturatedMath.kt:45-50
Duration saturating_diff(long long value_ns, long long origin, DurationUnit unit);
// Transliterated from: libraries/stdlib/src/kotlin/time/longSaturatedMath.kt:52-61
Duration saturating_origins_diff(long long origin1, long long origin2, DurationUnit unit);
// Transliterated from: libraries/stdlib/src/kotlin/time/longSaturatedMath.kt:78-80
bool is_saturated(long long value);
} // namespace kotlin::time
#endif
