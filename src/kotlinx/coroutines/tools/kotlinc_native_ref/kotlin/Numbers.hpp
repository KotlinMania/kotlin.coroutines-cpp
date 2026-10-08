// port-lint: source kotlin-native/runtime/src/main/kotlin/kotlin/Numbers.kt
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/Numbers.kt:12-265
#pragma once
#include <cstdint>
namespace kotlin {
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/Numbers.kt:17-17
/** Returns true if this number is a Not-a-Number (NaN) value. */
bool is_nan(double value);
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/Numbers.kt:30-30
/** Returns true if this value is infinitely large in magnitude. */
bool is_infinite(double value);
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/Numbers.kt:41-41
/** Returns true for finite values; false for NaN and infinity. */
bool is_finite(double value);
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/Numbers.kt:54-54
/** Returns IEEE 754 bits, using the canonical NaN representation for NaN values. */
std::int64_t to_bits(double value);
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/Numbers.kt:63-63
/** Returns IEEE 754 bits, preserving the exact NaN payload and sign. */
std::int64_t to_raw_bits(double value);
namespace double_companion {
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/Numbers.kt:70-70
/** Returns the floating-point value corresponding to this bit representation. */
double from_bits(std::int64_t bits);
}
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/Numbers.kt:24-24
/** Returns true if this number is a Not-a-Number (NaN) value. */
bool is_nan(float value);
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/Numbers.kt:36-36
/** Returns true if this value is infinitely large in magnitude. */
bool is_infinite(float value);
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/Numbers.kt:46-46
/** Returns true for finite values; false for NaN and infinity. */
bool is_finite(float value);
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/Numbers.kt:82-82
/** Returns IEEE 754 bits, using the canonical NaN representation for NaN values. */
std::int32_t to_bits(float value);
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/Numbers.kt:91-91
/** Returns IEEE 754 bits, preserving the exact NaN payload and sign. */
std::int32_t to_raw_bits(float value);
namespace float_companion {
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/Numbers.kt:98-98
/** Returns the floating-point value corresponding to this bit representation. */
float from_bits(std::int32_t bits);
}
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/Numbers.kt:111-111
/** Counts set bits in the binary representation of this number. */
std::int32_t count_one_bits(std::int32_t value);
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/Numbers.kt:124-125
/** Counts consecutive most significant zero bits; returns the width for zero. */
std::int32_t count_leading_zero_bits(std::int32_t value);
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/Numbers.kt:138-139
/** Counts consecutive least significant zero bits; returns the width for zero. */
std::int32_t count_trailing_zero_bits(std::int32_t value);
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/Numbers.kt:146-147
/** Returns only the most significant set bit, or zero for zero. */
std::int32_t take_highest_one_bit(std::int32_t value);
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/Numbers.kt:154-155
/** Returns only the least significant set bit, or zero for zero. */
std::int32_t take_lowest_one_bit(std::int32_t value);
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/Numbers.kt:168-169
/** Rotates bits left. Negative counts rotate right; multiples of the width preserve the value. */
std::int32_t rotate_left(std::int32_t value, std::int32_t bit_count);
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/Numbers.kt:183-184
/** Rotates bits right. Negative counts rotate left; multiples of the width preserve the value. */
std::int32_t rotate_right(std::int32_t value, std::int32_t bit_count);
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/Numbers.kt:192-192
/** Counts set bits in the binary representation of this number. */
std::int32_t count_one_bits(std::int64_t value);
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/Numbers.kt:205-206
/** Counts consecutive most significant zero bits; returns the width for zero. */
std::int32_t count_leading_zero_bits(std::int64_t value);
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/Numbers.kt:219-220
/** Counts consecutive least significant zero bits; returns the width for zero. */
std::int32_t count_trailing_zero_bits(std::int64_t value);
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/Numbers.kt:227-228
/** Returns only the most significant set bit, or zero for zero. */
std::int64_t take_highest_one_bit(std::int64_t value);
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/Numbers.kt:235-236
/** Returns only the least significant set bit, or zero for zero. */
std::int64_t take_lowest_one_bit(std::int64_t value);
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/Numbers.kt:249-250
/** Rotates bits left. Negative counts rotate right; multiples of the width preserve the value. */
std::int64_t rotate_left(std::int64_t value, std::int32_t bit_count);
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/Numbers.kt:264-265
/** Rotates bits right. Negative counts rotate left; multiples of the width preserve the value. */
std::int64_t rotate_right(std::int64_t value, std::int32_t bit_count);
}
