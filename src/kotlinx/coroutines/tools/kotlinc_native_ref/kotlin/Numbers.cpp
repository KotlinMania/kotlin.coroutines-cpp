// port-lint: source kotlin-native/runtime/src/main/kotlin/kotlin/Numbers.kt
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/Numbers.kt:12-265
#include "Numbers.hpp"
#include <bit>
#include <cmath>
namespace kotlin {
namespace {
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/Numbers.kt:118-118
// Transliterated from: kotlin-native/runtime/src/main/cpp/Operator.cpp:76-77
std::int32_t leading_zero_bits_nonzero(std::int32_t value) { return __builtin_clz(static_cast<std::uint32_t>(value)); }
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/Numbers.kt:132-132
// Transliterated from: kotlin-native/runtime/src/main/cpp/Operator.cpp:73-74
std::int32_t trailing_zero_bits_nonzero(std::int32_t value) { return __builtin_ctz(static_cast<std::uint32_t>(value)); }
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/Numbers.kt:199-199
// Transliterated from: kotlin-native/runtime/src/main/cpp/Operator.cpp:76-77
std::int32_t leading_zero_bits_nonzero(std::int64_t value) { return __builtin_clzll(static_cast<unsigned long long>(value)); }
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/Numbers.kt:213-213
// Transliterated from: kotlin-native/runtime/src/main/cpp/Operator.cpp:73-74
std::int32_t trailing_zero_bits_nonzero(std::int64_t value) { return __builtin_ctzll(static_cast<unsigned long long>(value)); }
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/Primitives.kt:1884-1884
const float FLOAT_NAN = -(0.0F / 0.0F);
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/Primitives.kt:2295-2295
const double DOUBLE_NAN = -(0.0 / 0.0);
}
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/Numbers.kt:17-17
// Transliterated from: kotlin-native/runtime/src/main/cpp/Operator.cpp:46-47,63-64
bool is_nan(double value) { return std::isnan(value); }
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/Numbers.kt:30-30
// Transliterated from: kotlin-native/runtime/src/main/cpp/Operator.cpp:46-47,63-64
bool is_infinite(double value) { return std::isinf(value); }
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/Numbers.kt:41-41
bool is_finite(double value) { return (to_raw_bits(value) & 0x7fffffffffffffffLL) < 0x7ff0000000000000LL; }
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/Numbers.kt:54-54
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/Primitives.kt:1884-1884,2295-2295
std::int64_t to_bits(double value) { return is_nan(value) ? to_raw_bits(DOUBLE_NAN) : to_raw_bits(value); }
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/Numbers.kt:63-63
// NOTE(port): REINTERPRET lowers to a C++ bit_cast of the actual scalar value.
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/Primitives.kt:2674-2676
std::int64_t to_raw_bits(double value) { return std::bit_cast<std::int64_t>(value); }
namespace {
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/Numbers.kt:74-74
// NOTE(port): The source typed reinterpret intrinsic is scalar bit_cast.
double from_bits(std::int64_t bits) { return std::bit_cast<double>(bits); }
}
namespace double_companion {
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/Numbers.kt:70-70
double from_bits(std::int64_t bits) { return kotlin::from_bits(bits); }
}
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/Numbers.kt:24-24
// Transliterated from: kotlin-native/runtime/src/main/cpp/Operator.cpp:46-47,63-64
bool is_nan(float value) { return std::isnan(value); }
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/Numbers.kt:36-36
// Transliterated from: kotlin-native/runtime/src/main/cpp/Operator.cpp:46-47,63-64
bool is_infinite(float value) { return std::isinf(value); }
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/Numbers.kt:46-46
bool is_finite(float value) { return (to_raw_bits(value) & 0x7fffffff) < 0x7f800000; }
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/Numbers.kt:82-82
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/Primitives.kt:1884-1884,2295-2295
std::int32_t to_bits(float value) { return is_nan(value) ? to_raw_bits(FLOAT_NAN) : to_raw_bits(value); }
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/Numbers.kt:91-91
// NOTE(port): REINTERPRET lowers to a C++ bit_cast of the actual scalar value.
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/Primitives.kt:2261-2263
std::int32_t to_raw_bits(float value) { return std::bit_cast<std::int32_t>(value); }
namespace {
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/Numbers.kt:102-102
// NOTE(port): The source typed reinterpret intrinsic is scalar bit_cast.
float from_bits(std::int32_t bits) { return std::bit_cast<float>(bits); }
}
namespace float_companion {
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/Numbers.kt:98-98
float from_bits(std::int32_t bits) { return kotlin::from_bits(bits); }
}
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/Numbers.kt:111-111
// Transliterated from: kotlin-native/runtime/src/main/cpp/Operator.cpp:70-71
std::int32_t count_one_bits(std::int32_t value) { return __builtin_popcount(static_cast<std::uint32_t>(value)); }
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/Numbers.kt:124-125
std::int32_t count_leading_zero_bits(std::int32_t value) { return value == 0 ? 32 : leading_zero_bits_nonzero(value); }
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/Numbers.kt:138-139
std::int32_t count_trailing_zero_bits(std::int32_t value) { return value == 0 ? 32 : trailing_zero_bits_nonzero(value); }
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/Numbers.kt:146-147
// NOTE(port): Unsigned arithmetic preserves source wrap/masked shifts without C++ UB.
std::int32_t take_highest_one_bit(std::int32_t value) { return value == 0 ? 0 : std::bit_cast<std::int32_t>(std::uint32_t{1} << (32 - 1 - leading_zero_bits_nonzero(value))); }
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/Numbers.kt:154-155
// NOTE(port): Unsigned arithmetic preserves source wrap/masked shifts without C++ UB.
std::int32_t take_lowest_one_bit(std::int32_t value) { const auto bits = static_cast<std::uint32_t>(value); return std::bit_cast<std::int32_t>(bits & (std::uint32_t{0} - bits)); }
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/Numbers.kt:168-169
// NOTE(port): Unsigned arithmetic preserves source wrap/masked shifts without C++ UB.
std::int32_t rotate_left(std::int32_t value, std::int32_t bit_count) { const auto bits = static_cast<std::uint32_t>(value); const auto count = static_cast<std::uint32_t>(bit_count); return std::bit_cast<std::int32_t>((bits << (count & 31U)) | (bits >> ((0U - count) & 31U))); }
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/Numbers.kt:183-184
// NOTE(port): Unsigned arithmetic preserves source wrap/masked shifts without C++ UB.
std::int32_t rotate_right(std::int32_t value, std::int32_t bit_count) { const auto bits = static_cast<std::uint32_t>(value); const auto count = static_cast<std::uint32_t>(bit_count); return std::bit_cast<std::int32_t>((bits << ((0U - count) & 31U)) | (bits >> (count & 31U))); }
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/Numbers.kt:192-192
// Transliterated from: kotlin-native/runtime/src/main/cpp/Operator.cpp:70-71
std::int32_t count_one_bits(std::int64_t value) { return __builtin_popcountll(static_cast<std::uint64_t>(value)); }
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/Numbers.kt:205-206
std::int32_t count_leading_zero_bits(std::int64_t value) { return value == 0 ? 64 : leading_zero_bits_nonzero(value); }
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/Numbers.kt:219-220
std::int32_t count_trailing_zero_bits(std::int64_t value) { return value == 0 ? 64 : trailing_zero_bits_nonzero(value); }
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/Numbers.kt:227-228
// NOTE(port): Unsigned arithmetic preserves source wrap/masked shifts without C++ UB.
std::int64_t take_highest_one_bit(std::int64_t value) { return value == 0 ? 0 : std::bit_cast<std::int64_t>(std::uint64_t{1} << (64 - 1 - leading_zero_bits_nonzero(value))); }
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/Numbers.kt:235-236
// NOTE(port): Unsigned arithmetic preserves source wrap/masked shifts without C++ UB.
std::int64_t take_lowest_one_bit(std::int64_t value) { const auto bits = static_cast<std::uint64_t>(value); return std::bit_cast<std::int64_t>(bits & (std::uint64_t{0} - bits)); }
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/Numbers.kt:249-250
// NOTE(port): Unsigned arithmetic preserves source wrap/masked shifts without C++ UB.
std::int64_t rotate_left(std::int64_t value, std::int32_t bit_count) { const auto bits = static_cast<std::uint64_t>(value); const auto count = static_cast<std::uint32_t>(bit_count); return std::bit_cast<std::int64_t>((bits << (count & 63U)) | (bits >> ((0U - count) & 63U))); }
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/Numbers.kt:264-265
// NOTE(port): Unsigned arithmetic preserves source wrap/masked shifts without C++ UB.
std::int64_t rotate_right(std::int64_t value, std::int32_t bit_count) { const auto bits = static_cast<std::uint64_t>(value); const auto count = static_cast<std::uint32_t>(bit_count); return std::bit_cast<std::int64_t>((bits << ((0U - count) & 63U)) | (bits >> (count & 63U))); }
}
