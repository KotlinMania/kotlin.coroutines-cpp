/*
 * Copyright 2010-2018 JetBrains s.r.o. Use of this source code is governed by the Apache 2.0 license
 * that can be found in the LICENSE file.
 */
// port-lint: source kotlin-native/runtime/src/main/kotlin/kotlin/text/StringNumberConversions.kt
// port-lint: source libraries/stdlib/native-wasm/src/kotlin/text/Char.kt
// port-lint: source kotlin-native/runtime/src/main/cpp/ToString.cpp
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/text/StringNumberConversions.kt:43-55
#include "StringNumberConversions.hpp"
#include <climits>
#include <stdexcept>

namespace kotlin::text {
namespace {
// Transliterated from: libraries/stdlib/native-wasm/src/kotlin/text/Char.kt:236-236
constexpr std::int32_t CHAR_MIN_RADIX = 2;
// Transliterated from: libraries/stdlib/native-wasm/src/kotlin/text/Char.kt:237-237
constexpr std::int32_t CHAR_MAX_RADIX = 36;

/**
 * Checks whether the given [radix] is valid radix for string to number and number to string conversion.
 */
// Transliterated from: libraries/stdlib/native-wasm/src/kotlin/text/Char.kt:228-233
std::int32_t check_radix(std::int32_t radix) {
  if (radix < CHAR_MIN_RADIX || radix > CHAR_MAX_RADIX) {
    throw std::invalid_argument("radix " + std::to_string(radix) +
                                " was not in valid range " + std::to_string(CHAR_MIN_RADIX) +
                                ".." + std::to_string(CHAR_MAX_RADIX));
  }
  return radix;
}

// Transliterated from: kotlin-native/runtime/src/main/cpp/ToString.cpp:31-37
char int_to_digit(std::uint32_t value) {
  if (value < 10) {
    return static_cast<char>('0' + value);
  } else {
    return static_cast<char>('a' + (value - 10));
  }
}

// Radix is checked on the Kotlin side.
// NOTE(port): Specialize the source private generic algorithm for its consumed
// Long entry. ASCII digits/sign become the same UTF-16 code units in compiler
// string storage; no Native string allocation/root is claimed by this return.
// Transliterated from: kotlin-native/runtime/src/main/cpp/ToString.cpp:40-66
std::u16string to_string_radix(std::int64_t value, std::int32_t radix) {
  if (value == 0) return u"0";
  // In the worst case, we convert to binary, with sign.
  char cstring[sizeof(std::int64_t) * CHAR_BIT + 2];
  const bool negative = value < 0;
  if (!negative) value = -value;

  std::int32_t length = 0;
  while (value < 0) {
    cstring[length++] = int_to_digit(static_cast<std::uint32_t>(-(value % radix)));
    value /= radix;
  }
  if (negative) cstring[length++] = '-';
  for (std::int32_t i = 0, j = length - 1; i < j; ++i, --j) {
    const char tmp = cstring[i];
    cstring[i] = cstring[j];
    cstring[j] = tmp;
  }
  cstring[length] = '\0';
  return std::u16string(cstring, cstring + length);
}

// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/text/StringNumberConversions.kt:43-46
// Transliterated from: kotlin-native/runtime/src/main/cpp/ToString.cpp:104-106
std::u16string long_to_string(std::int64_t value, std::int32_t radix) {
  return to_string_radix(value, radix);
}
}  // namespace

// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/text/StringNumberConversions.kt:55-55
std::u16string to_string(std::int64_t value, std::int32_t radix) {
  return long_to_string(value, check_radix(radix));
}
}  // namespace kotlin::text
