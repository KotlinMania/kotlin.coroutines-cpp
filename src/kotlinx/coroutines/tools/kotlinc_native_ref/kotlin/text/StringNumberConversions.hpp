/*
 * Copyright 2010-2018 JetBrains s.r.o. Use of this source code is governed by the Apache 2.0 license
 * that can be found in the LICENSE file.
 */
// port-lint: source kotlin-native/runtime/src/main/kotlin/kotlin/text/StringNumberConversions.kt
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/text/StringNumberConversions.kt:48-55
#pragma once

#include <cstdint>
#include <string>

namespace kotlin::text {
/**
 * Returns a string representation of this [Long] value in the specified [radix].
 *
 * @throws IllegalArgumentException when [radix] is not a valid radix for number to string conversion.
 */
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/text/StringNumberConversions.kt:55-55
std::u16string to_string(std::int64_t value, std::int32_t radix);
}  // namespace kotlin::text
