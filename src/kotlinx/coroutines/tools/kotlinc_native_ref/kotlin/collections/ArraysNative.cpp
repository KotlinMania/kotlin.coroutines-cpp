/*
 * Copyright 2010-2026 JetBrains s.r.o. and Kotlin Programming Language contributors.
 * Use of this source code is governed by the Apache 2.0 license that can be found in the license/LICENSE.txt file.
 */
// port-lint: source kotlin-native/runtime/src/main/kotlin/generated/_ArraysNative.kt
// Transliterated from: kotlin-native/runtime/src/main/kotlin/generated/_ArraysNative.kt:922-1654
#include "ArraysNative.hpp"
#include "ArrayUtil.hpp"
#include <algorithm>
#include <bit>
namespace kotlin::collections {
// Transliterated from: kotlin-native/runtime/src/main/kotlin/generated/_ArraysNative.kt:922-925
IntArray& copy_into(const IntArray& array, IntArray& destination, std::int32_t destination_offset, std::int32_t start_index, std::int32_t end_index) {
  const auto count = std::bit_cast<std::int32_t>(static_cast<std::uint32_t>(end_index) - static_cast<std::uint32_t>(start_index));
  array_copy(array, start_index, destination, destination_offset, count);
  return destination;
}

// Transliterated from: kotlin-native/runtime/src/main/kotlin/generated/_ArraysNative.kt:922-925
IntArray& copy_into(const IntArray& array, IntArray& destination, std::int32_t destination_offset, std::int32_t start_index) {
  return copy_into(array, destination, destination_offset, start_index, array.get_size());
}

// Transliterated from: kotlin-native/runtime/src/main/kotlin/generated/_ArraysNative.kt:1079-1081
IntArray copy_of(const IntArray& array) {
  return copy_of_uninitialized_elements(array, array.get_size());
}

// Transliterated from: kotlin-native/runtime/src/main/kotlin/generated/_ArraysNative.kt:1163-1165
IntArray copy_of(const IntArray& array, std::int32_t new_size) {
  return copy_of_uninitialized_elements(array, new_size);
}

// Transliterated from: kotlin-native/runtime/src/main/kotlin/generated/_ArraysNative.kt:1296-1299
IntArray copy_of_range(const IntArray& array, std::int32_t from_index, std::int32_t to_index) {
  check_copy_of_range_arguments(from_index, to_index, array.get_size());
  return copy_of_uninitialized_elements(array, from_index, to_index);
}

// Transliterated from: kotlin-native/runtime/src/main/kotlin/generated/_ArraysNative.kt:1425-1433
IntArray copy_of_uninitialized_elements(const IntArray& array, std::int32_t from_index, std::int32_t to_index) {
  const auto new_size = std::bit_cast<std::int32_t>(static_cast<std::uint32_t>(to_index) - static_cast<std::uint32_t>(from_index));
  if (new_size < 0) {
    throw std::invalid_argument(std::to_string(from_index) + " > " + std::to_string(to_index));
  }
  IntArray result(new_size);
  copy_into(array, result, 0, from_index, std::min(to_index, array.get_size()));
  return result;
}

// Transliterated from: kotlin-native/runtime/src/main/kotlin/generated/_ArraysNative.kt:1547-1549
IntArray copy_of_uninitialized_elements(const IntArray& array, std::int32_t new_size) {
  return copy_of_uninitialized_elements(array, 0, new_size);
}

// Transliterated from: kotlin-native/runtime/src/main/kotlin/generated/_ArraysNative.kt:1652-1654
void fill(IntArray& array, std::int32_t element, std::int32_t from_index, std::int32_t to_index) {
  array_fill(array, from_index, to_index, element);
}

// Transliterated from: kotlin-native/runtime/src/main/kotlin/generated/_ArraysNative.kt:1652-1654
void fill(IntArray& array, std::int32_t element, std::int32_t from_index) {
  fill(array, element, from_index, array.get_size());
}

}  // namespace kotlin::collections
