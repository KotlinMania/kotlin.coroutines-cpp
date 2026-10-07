// port-lint: source libraries/stdlib/src/kotlin/time/Duration.kt
// Transliterated from: libraries/stdlib/src/kotlin/time/Duration.kt
// Copyright 2010-2025 JetBrains s.r.o. and Kotlin Programming Language contributors.
// Use governed by the Apache 2.0 license in license/LICENSE.txt.
#include "kotlin/time/Duration.hpp"

#include <algorithm>
#include <bit>
#include <cstdlib>
#include <limits>
#include <stdexcept>
#include <string>

namespace kotlin::time {
namespace {
// Transliterated from: libraries/stdlib/src/kotlin/time/Duration.kt:1584-1595
constexpr long long NANOS_IN_MILLIS = 1000000;
constexpr long long MAX_MILLIS = std::numeric_limits<long long>::max() / 2;
constexpr long long MAX_NANOS = MAX_MILLIS / NANOS_IN_MILLIS * NANOS_IN_MILLIS - 1;
constexpr long long MAX_NANOS_IN_MILLIS = MAX_NANOS / NANOS_IN_MILLIS;
constexpr long long INVALID_RAW_VALUE = 0x7FFFFFFFFFFFC0DE;
// Transliterated from: libraries/stdlib/native/src/kotlin/time/DurationNative.kt:11
constexpr bool DURATION_ASSERTIONS_ENABLED = true;

// Transliterated from: libraries/stdlib/src/kotlin/time/Duration.kt:1606-1607
long long nanos_to_millis(long long nanos) { return nanos / NANOS_IN_MILLIS; }
// Transliterated from: libraries/stdlib/src/kotlin/time/Duration.kt:1607
long long millis_to_nanos(long long millis) { return millis * NANOS_IN_MILLIS; }

// Transliterated from: libraries/stdlib/src/kotlin/time/Duration.kt:1405-1414
bool is_infinite_millis(long long value) { return value == MAX_MILLIS || value == -MAX_MILLIS; }
// Transliterated from: libraries/stdlib/src/kotlin/time/Duration.kt:1414
bool is_finite_millis(long long value) { return -MAX_MILLIS < value && value < MAX_MILLIS; }
// Transliterated from: libraries/stdlib/src/kotlin/time/Duration.kt:1392-1396
long long add_millis_without_overflow(long long value, long long other) {
    if (is_infinite_millis(value)) {
        return is_finite_millis(other) || (value < 0) == (other < 0) ? value : INVALID_RAW_VALUE;
    }
    if (is_infinite_millis(other)) return other;
    return std::clamp(value + other, -MAX_MILLIS, MAX_MILLIS);
}

// Transliterated from: libraries/stdlib/native/src/kotlin/time/DurationUnit.kt:11-40
// NOTE(port): The C++ enum stores its ordinal; this accesses its Kotlin/Native scale property.
double scale(DurationUnit unit) {
    switch (unit) {
        case DurationUnit::NANOSECONDS: return 1e0;
        case DurationUnit::MICROSECONDS: return 1e3;
        case DurationUnit::MILLISECONDS: return 1e6;
        case DurationUnit::SECONDS: return 1e9;
        case DurationUnit::MINUTES: return 60e9;
        case DurationUnit::HOURS: return 3600e9;
        case DurationUnit::DAYS: return 86400e9;
    }
    throw std::invalid_argument("Unknown duration unit");
}

// Transliterated from: libraries/stdlib/src/kotlin/time/DurationUnit.kt:74-98
// Multiplies nonnegative values, clamping the result to MAX_MILLIS on overflow.
long long multiply_non_negative_without_overflow(long long value, long long other) {
    if (value == 0) return 0;
    if (value == 1) return std::min(other, MAX_MILLIS);
    if (other == 1) return std::min(value, MAX_MILLIS);
    const auto bit_sum = 128 - std::countl_zero(static_cast<unsigned long long>(value))
                             - std::countl_zero(static_cast<unsigned long long>(other));
    if (bit_sum < 63) return value * other;
    if (bit_sum > 63) return MAX_MILLIS;
    return std::min(value * other, MAX_MILLIS);
}

// Transliterated from: libraries/stdlib/src/kotlin/time/DurationUnit.kt:100-112
// Number of milliseconds in one whole unit.
long long millis_multiplier(DurationUnit unit) {
    switch (unit) {
        case DurationUnit::DAYS: return 86400000;
        case DurationUnit::HOURS: return 3600000;
        case DurationUnit::MINUTES: return 60000;
        case DurationUnit::SECONDS: return 1000;
        case DurationUnit::MILLISECONDS: return 1;
        default: throw std::invalid_argument("Wrong unit for millis_multiplier");
    }
}

// Transliterated from: libraries/stdlib/src/kotlin/time/DurationUnit.kt:60-72
long long convert_duration_unit_to_milliseconds(long long value, DurationUnit unit) {
    return multiply_non_negative_without_overflow(value, millis_multiplier(unit));
}

// Transliterated from: libraries/stdlib/native/src/kotlin/time/DurationUnit.kt:52-59
long long convert_duration_unit_overflow(long long value, DurationUnit source_unit, DurationUnit target_unit) {
    const auto source_scale = scale(source_unit);
    const auto target_scale = scale(target_unit);
    const auto source_compare_target = (source_scale > target_scale) - (source_scale < target_scale);
    if (source_compare_target > 0) {
        // NOTE(port): Unsigned multiplication and bit_cast preserve Kotlin's
        // wrapped Long result; signed multiplication in C++ would be undefined.
        const auto scale = static_cast<long long>(source_scale / target_scale);
        return std::bit_cast<long long>(static_cast<unsigned long long>(value) * static_cast<unsigned long long>(scale));
    }
    if (source_compare_target < 0) return value / static_cast<long long>(target_scale / source_scale);
    return value;
}

// Transliterated from: libraries/stdlib/native/src/kotlin/time/DurationUnit.kt:62-77
long long convert_duration_unit(long long value, DurationUnit source_unit, DurationUnit target_unit) {
    const auto source_scale = scale(source_unit);
    const auto target_scale = scale(target_unit);
    const auto source_compare_target = (source_scale > target_scale) - (source_scale < target_scale);
    if (source_compare_target > 0) {
        const auto scale = static_cast<long long>(source_scale / target_scale);
        // NOTE(port): Preserve the wrapped result before Kotlin's overflow test.
        const auto result = std::bit_cast<long long>(static_cast<unsigned long long>(value) * static_cast<unsigned long long>(scale));
        if (result / scale == value) return result;
        if (value > 0) return std::numeric_limits<long long>::max();
        return std::numeric_limits<long long>::min();
    }
    if (source_compare_target < 0) return value / static_cast<long long>(target_scale / source_scale);
    return value;
}
} // namespace

// Transliterated from: libraries/stdlib/src/kotlin/time/Duration.kt:34-37
Duration::Duration(long long raw_value) : raw_value_(raw_value) {}
// Transliterated from: libraries/stdlib/src/kotlin/time/Duration.kt:46-56
// NOTE(port): Kotlin AssertionError maps to std::logic_error for representation invariants.
Duration Duration::from_raw_value(long long raw_value) {
    Duration duration(raw_value);
    if (DURATION_ASSERTIONS_ENABLED) {
        if (duration.is_in_nanos()) {
            if (duration.value() < -MAX_NANOS || duration.value() > MAX_NANOS)
                throw std::logic_error(std::to_string(duration.value()) + " ns is out of nanoseconds range");
        } else {
            if (!is_finite_millis(duration.value()) && !is_infinite_millis(duration.value()))
                throw std::logic_error(std::to_string(duration.value()) + " ms is out of milliseconds range");
            if (-MAX_NANOS_IN_MILLIS <= duration.value() && duration.value() <= MAX_NANOS_IN_MILLIS)
                throw std::logic_error(std::to_string(duration.value()) + " ms is denormalized");
        }
    }
    return duration;
}
// Transliterated from: libraries/stdlib/src/kotlin/time/Duration.kt:40
long long Duration::value() const { return raw_value_ >> 1; }
// Transliterated from: libraries/stdlib/src/kotlin/time/Duration.kt:41
int Duration::unit_discriminator() const { return static_cast<int>(raw_value_ & 1); }
// Transliterated from: libraries/stdlib/src/kotlin/time/Duration.kt:42
bool Duration::is_in_nanos() const { return unit_discriminator() == 0; }
// Transliterated from: libraries/stdlib/src/kotlin/time/Duration.kt:43
bool Duration::is_in_millis() const { return unit_discriminator() == 1; }
// Transliterated from: libraries/stdlib/src/kotlin/time/Duration.kt:44
DurationUnit Duration::storage_unit() const { return is_in_nanos() ? DurationUnit::NANOSECONDS : DurationUnit::MILLISECONDS; }

// Transliterated from: libraries/stdlib/src/kotlin/time/Duration.kt:1611
Duration Duration::duration_of(long long value, int discriminator) {
    // NOTE(port): Unsigned shifting preserves Kotlin Long bits without signed overflow.
    return from_raw_value(std::bit_cast<long long>((static_cast<unsigned long long>(value) << 1) + discriminator));
}
// Transliterated from: libraries/stdlib/src/kotlin/time/Duration.kt:1609
Duration Duration::duration_of_nanos(long long nanos) { return duration_of(nanos, 0); }
// Transliterated from: libraries/stdlib/src/kotlin/time/Duration.kt:1610
Duration Duration::duration_of_millis(long long millis) { return duration_of(millis, 1); }
// Transliterated from: libraries/stdlib/src/kotlin/time/Duration.kt:1612-1617
Duration Duration::duration_of_nanos_normalized(long long nanos) {
    return -MAX_NANOS <= nanos && nanos <= MAX_NANOS ? duration_of_nanos(nanos) : duration_of_millis(nanos_to_millis(nanos));
}
// Transliterated from: libraries/stdlib/src/kotlin/time/Duration.kt:1619-1624
Duration Duration::duration_of_millis_normalized(long long millis) {
    return -MAX_NANOS_IN_MILLIS <= millis && millis <= MAX_NANOS_IN_MILLIS
        ? duration_of_nanos(millis_to_nanos(millis)) : duration_of_millis(std::clamp(millis, -MAX_MILLIS, MAX_MILLIS));
}
// Transliterated from: libraries/stdlib/src/kotlin/time/Duration.kt:59-65
const Duration Duration::ZERO(0);
const Duration Duration::INFINITE = Duration::duration_of_millis(MAX_MILLIS);
// Transliterated from: libraries/stdlib/src/kotlin/time/Duration.kt:65
const Duration Duration::NEG_INFINITE = Duration::duration_of_millis(-MAX_MILLIS);

// Transliterated from: libraries/stdlib/src/kotlin/time/Duration.kt:353
Duration Duration::operator-() const { return duration_of(-value(), unit_discriminator()); }
// Transliterated from: libraries/stdlib/src/kotlin/time/Duration.kt:361-375
Duration Duration::operator+(Duration other) const {
    if (unit_discriminator() == other.unit_discriminator()) {
        if (is_in_nanos()) return duration_of_nanos_normalized(value() + other.value());
        const auto result = add_millis_without_overflow(value(), other.value());
        if (result == INVALID_RAW_VALUE) throw std::invalid_argument("Summing infinite durations of different signs yields an undefined result.");
        return is_infinite_millis(result) ? duration_of_millis(result) : duration_of_millis_normalized(result);
    }
    return is_in_millis() ? add_values_mixed_ranges(value(), other.value()) : add_values_mixed_ranges(other.value(), value());
}
// Transliterated from: libraries/stdlib/src/kotlin/time/Duration.kt:377-387
Duration Duration::add_values_mixed_ranges(long long millis, long long nanos) {
    const auto other_millis = nanos_to_millis(nanos);
    // The nanosecond range excludes infinities, so other_millis is finite.
    const auto result_millis = add_millis_without_overflow(millis, other_millis);
    if (-MAX_NANOS_IN_MILLIS <= result_millis && result_millis <= MAX_NANOS_IN_MILLIS) {
        const auto other_nano_remainder = nanos - millis_to_nanos(other_millis);
        return duration_of_nanos(millis_to_nanos(result_millis) + other_nano_remainder);
    }
    return duration_of_millis(result_millis);
}
// Transliterated from: libraries/stdlib/src/kotlin/time/Duration.kt:389-395
Duration Duration::operator-(Duration other) const { return *this + (-other); }
// Transliterated from: libraries/stdlib/src/kotlin/time/Duration.kt:404-441
Duration Duration::operator*(int scale) const {
    static_assert(sizeof(int) == 4);
    if (is_infinite()) {
        if (scale == 0) throw std::invalid_argument("Multiplying infinite duration by zero yields an undefined result.");
        if (scale > 0) return *this;
        return -*this;
    }
    if (scale == 0) return ZERO;
    const auto value = this->value();
    // NOTE(port): Unsigned arithmetic preserves Kotlin's wrapped Long product.
    const auto result = std::bit_cast<long long>(static_cast<unsigned long long>(value) * static_cast<unsigned long long>(scale));
    if (is_in_nanos()) {
        if (MAX_NANOS / std::numeric_limits<int>::min() <= value && value <= -MAX_NANOS / std::numeric_limits<int>::min()) {
            // Cannot overflow the nanosecond range for any scale.
            return duration_of_nanos(result);
        }
        // NOTE(port): Kotlin Long.MIN_VALUE / -1 wraps; C++ division would be undefined.
        if ((scale == -1 && result == std::numeric_limits<long long>::min() ? result : result / scale) == value) {
            return duration_of_nanos_normalized(result);
        }
        const auto millis = nanos_to_millis(value);
        const auto rem_nanos = value - millis_to_nanos(millis);
        const auto result_millis = std::bit_cast<long long>(static_cast<unsigned long long>(millis) * static_cast<unsigned long long>(scale));
        const auto total_millis = std::bit_cast<long long>(static_cast<unsigned long long>(result_millis) + static_cast<unsigned long long>(nanos_to_millis(rem_nanos * scale)));
        if ((scale == -1 && result_millis == std::numeric_limits<long long>::min() ? result_millis : result_millis / scale) == millis && (total_millis ^ result_millis) >= 0) {
            return duration_of_millis(std::clamp(total_millis, -MAX_MILLIS, MAX_MILLIS));
        }
        return ((value > 0) - (value < 0)) * ((scale > 0) - (scale < 0)) > 0 ? INFINITE : NEG_INFINITE;
    }
    if ((scale == -1 && result == std::numeric_limits<long long>::min() ? result : result / scale) == value) {
        return duration_of_millis(std::clamp(result, -MAX_MILLIS, MAX_MILLIS));
    }
    return ((value > 0) - (value < 0)) * ((scale > 0) - (scale < 0)) > 0 ? INFINITE : NEG_INFINITE;
}
// Transliterated from: libraries/stdlib/src/kotlin/time/Duration.kt:468-489
Duration Duration::operator/(int scale) const {
    if (scale == 0) {
        if (is_positive()) return INFINITE;
        if (is_negative()) return NEG_INFINITE;
        throw std::invalid_argument("Dividing zero duration by zero yields an undefined result.");
    }
    if (is_in_nanos()) return duration_of_nanos(value() / scale);
    if (is_infinite()) return *this * ((scale > 0) - (scale < 0));
    const auto result = value() / scale;
    if (-MAX_NANOS_IN_MILLIS <= result && result <= MAX_NANOS_IN_MILLIS) {
        const auto rem = millis_to_nanos(value() - result * scale) / scale;
        return duration_of_nanos(millis_to_nanos(result) + rem);
    }
    return duration_of_millis(result);
}
// Transliterated from: libraries/stdlib/src/kotlin/time/Duration.kt:515-524
Duration Duration::truncate_to(DurationUnit unit) const {
    const auto storage_unit = this->storage_unit();
    if (unit <= storage_unit || is_infinite()) return *this;
    const auto scale = convert_duration_unit(1, unit, storage_unit);
    const auto result = value() - value() % scale;
    return to_duration(result, storage_unit);
}
// Transliterated from: libraries/stdlib/src/kotlin/time/Duration.kt:538-539
Duration Duration::absolute_value() const { return is_negative() ? -*this : *this; }
// Transliterated from: libraries/stdlib/src/kotlin/time/Duration.kt:541-549
int Duration::compare_to(Duration other) const {
    const auto compare_bits = raw_value_ ^ other.raw_value_;
    if (compare_bits < 0 || (compare_bits & 1) == 0)
        return (raw_value_ > other.raw_value_) - (raw_value_ < other.raw_value_);
    // Same sign, different ranges: compare the unit discriminators.
    const auto r = unit_discriminator() - other.unit_discriminator();
    return is_negative() ? -r : r;
}
// Transliterated from: libraries/stdlib/src/kotlin/time/Duration.kt:527
bool Duration::is_negative() const { return raw_value_ < 0; }
// Transliterated from: libraries/stdlib/src/kotlin/time/Duration.kt:530
bool Duration::is_positive() const { return raw_value_ > 0; }
// Transliterated from: libraries/stdlib/src/kotlin/time/Duration.kt:533
bool Duration::is_infinite() const { return raw_value_ == INFINITE.raw_value_ || raw_value_ == NEG_INFINITE.raw_value_; }
// Transliterated from: libraries/stdlib/src/kotlin/time/Duration.kt:536
bool Duration::is_finite() const { return !is_infinite(); }
// Transliterated from: libraries/stdlib/src/kotlin/time/Duration.kt:680-686
long long Duration::to_long(DurationUnit unit) const {
    if (raw_value_ == INFINITE.raw_value_) return std::numeric_limits<long long>::max();
    if (raw_value_ == NEG_INFINITE.raw_value_) return std::numeric_limits<long long>::min();
    return convert_duration_unit(value(), storage_unit(), unit);
}
// Transliterated from: libraries/stdlib/src/kotlin/time/Duration.kt:772-775
long long Duration::in_whole_milliseconds() const {
    return is_in_millis() && is_finite() ? value() : to_long(DurationUnit::MILLISECONDS);
}
// Transliterated from: libraries/stdlib/src/kotlin/time/Duration.kt:984-996
Duration to_duration(long long value, DurationUnit unit) {
    const auto max_ns_in_unit = convert_duration_unit_overflow(MAX_NANOS, DurationUnit::NANOSECONDS, unit);
    if (-max_ns_in_unit <= value && value <= max_ns_in_unit) {
        return Duration::duration_of_nanos(convert_duration_unit_overflow(value, unit, DurationUnit::NANOSECONDS));
    }
    if (unit >= DurationUnit::MILLISECONDS) {
        const auto sign = (value > 0) - (value < 0);
        const auto magnitude = std::abs(std::max(value, std::numeric_limits<long long>::min() + 1));
        const auto millis = convert_duration_unit_to_milliseconds(magnitude, unit);
        return Duration::duration_of_millis(sign * millis);
    }
    return Duration::duration_of_millis(std::clamp(convert_duration_unit(value, unit, DurationUnit::MILLISECONDS), -MAX_MILLIS, MAX_MILLIS));
}
// Transliterated from: libraries/stdlib/src/kotlin/time/Duration.kt:86-92
Duration nanoseconds(long long value) { return to_duration(value, DurationUnit::NANOSECONDS); }
// Transliterated from: libraries/stdlib/src/kotlin/time/Duration.kt:142-148
Duration milliseconds(long long value) { return to_duration(value, DurationUnit::MILLISECONDS); }
// Transliterated from: libraries/stdlib/src/kotlin/time/Duration.kt:170-176
Duration seconds(long long value) { return to_duration(value, DurationUnit::SECONDS); }
} // namespace kotlin::time
