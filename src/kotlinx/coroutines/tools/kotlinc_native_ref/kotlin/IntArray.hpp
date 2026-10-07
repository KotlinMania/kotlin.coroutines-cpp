/*
 * Copyright 2010-2024 JetBrains s.r.o. and Kotlin Programming Language contributors.
 * Use of this source code is governed by the Apache 2.0 license that can be found in the license/LICENSE.txt file.
 */
// port-lint: source kotlin-native/runtime/src/main/kotlin/kotlin/Arrays.kt
// port-lint: source kotlin-native/runtime/src/main/kotlin/kotlin/collections/ArrayUtil.kt
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/Arrays.kt:251-328
#pragma once
#include "collections/PrimitiveIterators.hpp"
#include <memory>

namespace kotlin {
class IntArray;
namespace collections {
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/collections/ArrayUtil.kt:54-56
void array_fill(IntArray& array, std::int32_t from_index, std::int32_t to_index, std::int32_t value);
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/collections/ArrayUtil.kt:111-113
void array_copy(const IntArray& array, std::int32_t from_index, IntArray& destination,
                std::int32_t to_index, std::int32_t count);
}

/**
 * An array of ints.
 *
 * See [Kotlin language documentation](https://kotlinlang.org/docs/arrays.html)
 * for more information on arrays.
 */
// NOTE(port): This is compiler-owned fixed-length primitive storage with source
// array identity. It is not Native ArrayHeader allocation or GC-managed storage.
// Actual Native IntArray runtime declarations live at the Native array boundary.
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/Arrays.kt:258-322
class IntArray final {
 public:
  /**
   * Creates a new array of the specified [size], with all elements initialized to zero.
   * @throws RuntimeException if the specified [size] is negative.
   */
  // Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/Arrays.kt:258-266
  explicit IntArray(std::int32_t size);
  /**
   * Creates a new array of the specified [size], where each element is calculated by calling the specified
   * [init] function.
   *
   * The function [init] is called for each array element sequentially starting from the first one.
   * It should return the value for an array element given its index.
   *
   * @throws RuntimeException if the specified [size] is negative.
   */
  // Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/Arrays.kt:276-280
  template <typename Init>
  IntArray(std::int32_t size, Init init) : IntArray(size) {
    for (std::int32_t index = 0; index < size; ++index) {
      set(index, init(index));
    }
  }
  /**
   * Returns the array element at the given [index].
   *
   * This method can be called using the index operator:
   * ```
   * value = array[index]
   * ```
   *
   * If the [index] is out of bounds of this array, throws an [IndexOutOfBoundsException].
   */
  // Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/Arrays.kt:291-293
  std::int32_t get(std::int32_t index) const;
  /**
   * Sets the array element at the given [index] to the given [value].
   *
   * This method can be called using the index operator:
   * ```
   * array[index] = value
   * ```
   *
   * If the [index] is out of bounds of this array, throws an [IndexOutOfBoundsException].
   */
  // Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/Arrays.kt:305-307
  void set(std::int32_t index, std::int32_t value);
  /**
   * Returns the number of elements in the array.
   */
  // Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/Arrays.kt:312-313
  std::int32_t get_size() const;
  /** Creates a specialized [IntIterator] for iterating over the elements of the array. */
  // Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/Arrays.kt:316-317
  std::unique_ptr<collections::IntIterator> iterator() const;
 private:
  struct Storage;
  std::shared_ptr<Storage> storage_;
  // Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/Arrays.kt:319-321
  std::int32_t get_array_length() const;
  friend void collections::array_fill(IntArray&, std::int32_t, std::int32_t, std::int32_t);
  friend void collections::array_copy(const IntArray&, std::int32_t, IntArray&, std::int32_t, std::int32_t);
};
}  // namespace kotlin
