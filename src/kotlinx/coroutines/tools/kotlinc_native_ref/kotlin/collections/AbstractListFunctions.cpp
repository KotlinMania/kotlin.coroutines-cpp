/*
 * Copyright 2010-2020 JetBrains s.r.o. and Kotlin Programming Language contributors.
 * Use of this source code is governed by the Apache 2.0 license that can be found in the license/LICENSE.txt file.
 */
// port-lint: source libraries/stdlib/src/kotlin/collections/AbstractList.kt
// Transliterated from: libraries/stdlib/src/kotlin/collections/AbstractList.kt:115-157
#include "AbstractListFunctions.hpp"
#include <bit>
#include <limits>
#include <stdexcept>
#include <string>

namespace kotlin::collections::abstract_list {
namespace {
// Transliterated from: libraries/stdlib/src/kotlin/collections/AbstractList.kt:146-146
constexpr std::int32_t MAX_ARRAY_SIZE = std::numeric_limits<std::int32_t>::max() - 8;
}
// Transliterated from: libraries/stdlib/src/kotlin/collections/AbstractList.kt:116-120
void check_element_index(std::int32_t index, std::int32_t size) {
  if (index < 0 || index >= size) {
    throw std::out_of_range("index: " + std::to_string(index) + ", size: " + std::to_string(size));
  }
}
// Transliterated from: libraries/stdlib/src/kotlin/collections/AbstractList.kt:122-126
void check_position_index(std::int32_t index, std::int32_t size) {
  if (index < 0 || index > size) {
    throw std::out_of_range("index: " + std::to_string(index) + ", size: " + std::to_string(size));
  }
}
// Transliterated from: libraries/stdlib/src/kotlin/collections/AbstractList.kt:128-135
void check_range_indexes(std::int32_t from_index, std::int32_t to_index, std::int32_t size) {
  if (from_index < 0 || to_index > size) {
    throw std::out_of_range("fromIndex: " + std::to_string(from_index) + ", toIndex: " +
                            std::to_string(to_index) + ", size: " + std::to_string(size));
  }
  if (from_index > to_index) {
    throw std::invalid_argument("fromIndex: " + std::to_string(from_index) + " > toIndex: " + std::to_string(to_index));
  }
}
// Transliterated from: libraries/stdlib/src/kotlin/collections/AbstractList.kt:137-144
void check_bounds_indexes(std::int32_t start_index, std::int32_t end_index, std::int32_t size) {
  if (start_index < 0 || end_index > size) {
    throw std::out_of_range("startIndex: " + std::to_string(start_index) + ", endIndex: " +
                            std::to_string(end_index) + ", size: " + std::to_string(size));
  }
  if (start_index > end_index) {
    throw std::invalid_argument("startIndex: " + std::to_string(start_index) + " > endIndex: " + std::to_string(end_index));
  }
}
// Transliterated from: libraries/stdlib/src/kotlin/collections/AbstractList.kt:149-157
std::int32_t new_capacity(std::int32_t old_capacity, std::int32_t min_capacity) {
  // overflow-conscious
  // NOTE(port): Kotlin Int additions/subtractions wrap. Use unsigned arithmetic
  // and preserve the resulting signed comparisons without C++ signed overflow.
  auto new_capacity = std::bit_cast<std::int32_t>(static_cast<std::uint32_t>(old_capacity) +
                                                static_cast<std::uint32_t>(old_capacity >> 1));
  if (std::bit_cast<std::int32_t>(static_cast<std::uint32_t>(new_capacity) -
                                 static_cast<std::uint32_t>(min_capacity)) < 0) {
    new_capacity = min_capacity;
  }
  if (std::bit_cast<std::int32_t>(static_cast<std::uint32_t>(new_capacity) -
                                 static_cast<std::uint32_t>(MAX_ARRAY_SIZE)) > 0) {
    new_capacity = min_capacity > MAX_ARRAY_SIZE ? std::numeric_limits<std::int32_t>::max() : MAX_ARRAY_SIZE;
  }
  return new_capacity;
}
}  // namespace kotlin::collections::abstract_list
