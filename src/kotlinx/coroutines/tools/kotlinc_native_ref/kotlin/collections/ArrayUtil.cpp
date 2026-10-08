/*
 * Copyright 2010-2018 JetBrains s.r.o. Use of this source code is governed by the Apache 2.0 license
 * that can be found in the LICENSE file.
 */
// port-lint: source kotlin-native/runtime/src/main/kotlin/kotlin/collections/ArrayUtil.kt
// port-lint: source libraries/stdlib/native-wasm/src/kotlin/collections/Arrays.kt
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/collections/ArrayUtil.kt:75-82
#include "ArrayUtil.hpp"

namespace kotlin::collections {
// Transliterated from: libraries/stdlib/native-wasm/src/kotlin/collections/Arrays.kt:14-19
void check_copy_of_range_arguments(std::int32_t from_index, std::int32_t to_index, std::int32_t size) {
  if (to_index > size) {
    throw std::out_of_range("toIndex (" + std::to_string(to_index) +
                            ") is greater than size (" + std::to_string(size) + ").");
  }
  if (from_index > to_index) {
    throw std::invalid_argument("fromIndex (" + std::to_string(from_index) +
                                ") is greater than toIndex (" + std::to_string(to_index) + ").");
  }
}
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/collections/ArrayUtil.kt:75-82
void check_range_indexes(std::int32_t from_index, std::int32_t to_index, std::int32_t size) {
  if (from_index < 0 || to_index > size) {
    throw std::out_of_range("fromIndex: " + std::to_string(from_index) +
                            ", toIndex: " + std::to_string(to_index) +
                            ", size: " + std::to_string(size));
  }
  if (from_index > to_index) {
    throw std::invalid_argument("fromIndex: " + std::to_string(from_index) +
                                " > toIndex: " + std::to_string(to_index));
  }
}
}  // namespace kotlin::collections
