// port-lint: source libraries/stdlib/src/kotlin/time/Duration.kt
// Transliterated from: libraries/stdlib/src/kotlin/time/Duration.kt:12-44
#pragma once

namespace kotlin::time {

// Transliterated from: libraries/stdlib/src/kotlin/time/DurationUnit.kt:12-46
// The smallest time unit is NANOSECONDS; DAYS is exactly 24 HOURS.
enum class DurationUnit { NANOSECONDS, MICROSECONDS, MILLISECONDS, SECONDS, MINUTES, HOURS, DAYS };

// Transliterated from: libraries/stdlib/src/kotlin/time/Duration.kt:12-44
// Stores durations with nanosecond precision up to ±146 years and millisecond
// precision up to ±146 million years. Values outside that range are infinite.
class Duration {
public:
    // Transliterated from: libraries/stdlib/src/kotlin/time/Duration.kt:59-65
    static const Duration ZERO;
    static const Duration INFINITE;

    // Transliterated from: libraries/stdlib/src/kotlin/time/Duration.kt:353
    Duration operator-() const;
    // Transliterated from: libraries/stdlib/src/kotlin/time/Duration.kt:355-375
    // Returns the sum; opposite infinities produce an undefined result and throw.
    Duration operator+(Duration other) const;
    // Transliterated from: libraries/stdlib/src/kotlin/time/Duration.kt:389-395
    // Returns the difference; subtracting infinities of the same sign throws.
    Duration operator-(Duration other) const;
    // Transliterated from: libraries/stdlib/src/kotlin/time/Duration.kt:397-441
    // Multiplies by an integer; infinity multiplied by zero is undefined.
    Duration operator*(int scale) const;
    // Transliterated from: libraries/stdlib/src/kotlin/time/Duration.kt:462-489
    // Divides by an integer; zero divided by zero is undefined.
    Duration operator/(int scale) const;
    // Transliterated from: libraries/stdlib/src/kotlin/time/Duration.kt:538-549
    // Returns the nonnegative absolute value and compares the encoded ranges.
    Duration absolute_value() const;
    int compare_to(Duration other) const;
    // Transliterated from: libraries/stdlib/src/kotlin/time/Duration.kt:527-536
    bool is_negative() const;
    bool is_positive() const;
    bool is_infinite() const;
    bool is_finite() const;
    // Transliterated from: libraries/stdlib/src/kotlin/time/Duration.kt:667-686
    // Truncates fractional units toward zero; infinity saturates to long long.
    long long to_long(DurationUnit unit) const;
    // Transliterated from: libraries/stdlib/src/kotlin/time/Duration.kt:759-775
    long long in_whole_milliseconds() const;

private:
    // Transliterated from: libraries/stdlib/src/kotlin/time/Duration.kt:515-524
    Duration truncate_to(DurationUnit unit) const;
    // Transliterated from: libraries/stdlib/src/kotlin/time/Duration.kt:65
    static const Duration NEG_INFINITE;
    // Transliterated from: libraries/stdlib/src/kotlin/time/Duration.kt:34-37
    explicit Duration(long long raw_value);
    // Transliterated from: libraries/stdlib/src/kotlin/time/Duration.kt:46-56
    static Duration from_raw_value(long long raw_value);
    long long value() const;
    int unit_discriminator() const;
    bool is_in_nanos() const;
    bool is_in_millis() const;
    DurationUnit storage_unit() const;
    // Transliterated from: libraries/stdlib/src/kotlin/time/Duration.kt:1609-1624
    static Duration duration_of(long long value, int discriminator);
    static Duration duration_of_nanos(long long nanos);
    static Duration duration_of_millis(long long millis);
    static Duration duration_of_nanos_normalized(long long nanos);
    static Duration duration_of_millis_normalized(long long millis);
    // Transliterated from: libraries/stdlib/src/kotlin/time/Duration.kt:377-387
    static Duration add_values_mixed_ranges(long long millis, long long nanos);
    friend Duration to_duration(long long value, DurationUnit unit);
    long long raw_value_;
};

// Transliterated from: libraries/stdlib/src/kotlin/time/Duration.kt:978-996
Duration to_duration(long long value, DurationUnit unit);
// Transliterated from: libraries/stdlib/src/kotlin/time/Duration.kt:86-92
Duration nanoseconds(long long value);
// Transliterated from: libraries/stdlib/src/kotlin/time/Duration.kt:142-148
Duration milliseconds(long long value);
// Transliterated from: libraries/stdlib/src/kotlin/time/Duration.kt:170-176
Duration seconds(long long value);

} // namespace kotlin::time
