/**
 * Transliterated from: libraries/stdlib/src/kotlin/time/longSaturatedMath.kt
 */
#include "kotlin/time/LongSaturatedMath.hpp"
#include <bit>
#include <limits>
#include <stdexcept>

namespace kotlin::time {
namespace {
// NOTE(port): Kotlin Long arithmetic wraps. Perform the operations on the
// same unsigned bits so C++ signed overflow never precedes the source checks.
long long wrapping_add(long long left, long long right) {
    return std::bit_cast<long long>(std::bit_cast<unsigned long long>(left) +
                                    std::bit_cast<unsigned long long>(right));
}
// NOTE(port): Unsigned subtraction preserves Kotlin's wrapped Long result.
long long wrapping_subtract(long long left, long long right) {
    return std::bit_cast<long long>(std::bit_cast<unsigned long long>(left) -
                                    std::bit_cast<unsigned long long>(right));
}
// Transliterated from: libraries/stdlib/src/kotlin/time/longSaturatedMath.kt:28-31
long long check_infinite_sum_defined(long long value, Duration duration, long long duration_in_unit) {
    if (duration.is_infinite() && (value ^ duration_in_unit) < 0)
        throw std::invalid_argument("Summing infinities of different signs");
    return value;
}
// Transliterated from: libraries/stdlib/src/kotlin/time/longSaturatedMath.kt:33-41
long long saturating_add_in_halves(long long value, DurationUnit unit, Duration duration) {
    auto half = duration / 2;
    auto half_in_unit = half.to_long(unit);
    if (is_saturated(half_in_unit)) return half_in_unit;
    return saturating_add(saturating_add(value, unit, half), unit, duration - half);
}
// Transliterated from: libraries/stdlib/src/kotlin/time/longSaturatedMath.kt:43
Duration infinity_of_sign(long long value) {
    // NOTE(port): NEG_INFINITE is private in the existing Duration interface;
    // its defined unary negation yields the same encoded negative infinity.
    return value < 0 ? -Duration::INFINITE : Duration::INFINITE;
}
// Transliterated from: libraries/stdlib/src/kotlin/time/longSaturatedMath.kt:63-76
Duration saturating_finite_diff(long long value1, long long value2, DurationUnit unit) {
    auto result = wrapping_subtract(value1, value2);
    if (((result ^ value1) & ~(result ^ value2)) < 0) {
        if (unit < DurationUnit::MILLISECONDS) {
            // NOTE(port): Route the source conversion through the existing
            // Duration conversion dependency instead of duplicating its units.
            auto units_in_milli = milliseconds(1).to_long(unit);
            auto result_ms = value1 / units_in_milli - value2 / units_in_milli;
            auto result_unit = value1 % units_in_milli - value2 % units_in_milli;
            return milliseconds(result_ms) + to_duration(result_unit, unit);
        }
        return -infinity_of_sign(result);
    }
    return to_duration(result, unit);
}
} // namespace

// Transliterated from: libraries/stdlib/src/kotlin/time/longSaturatedMath.kt:12-26
long long saturating_add(long long value, DurationUnit unit, Duration duration) {
    auto duration_in_unit = duration.to_long(unit);
    if (is_saturated(value)) return check_infinite_sum_defined(value, duration, duration_in_unit);
    if (is_saturated(duration_in_unit)) return saturating_add_in_halves(value, unit, duration);
    auto result = wrapping_add(value, duration_in_unit);
    if (((value ^ result) & (duration_in_unit ^ result)) < 0)
        return value < 0 ? std::numeric_limits<long long>::min() : std::numeric_limits<long long>::max();
    return result;
}
// Transliterated from: libraries/stdlib/src/kotlin/time/longSaturatedMath.kt:45-50
Duration saturating_diff(long long value_ns, long long origin, DurationUnit unit) {
    if (is_saturated(origin)) return -infinity_of_sign(origin);
    return saturating_finite_diff(value_ns, origin, unit);
}
// Transliterated from: libraries/stdlib/src/kotlin/time/longSaturatedMath.kt:52-61
Duration saturating_origins_diff(long long origin1, long long origin2, DurationUnit unit) {
    if (is_saturated(origin2)) {
        if (origin1 == origin2) return Duration::ZERO;
        return -infinity_of_sign(origin2);
    }
    if (is_saturated(origin1)) return infinity_of_sign(origin1);
    return saturating_finite_diff(origin1, origin2, unit);
}
// Transliterated from: libraries/stdlib/src/kotlin/time/longSaturatedMath.kt:78-80
bool is_saturated(long long value) {
    return ((std::bit_cast<unsigned long long>(value) - 1) | 1) ==
        static_cast<unsigned long long>(std::numeric_limits<long long>::max());
}
} // namespace kotlin::time
