/*
 * Copyright 2010-2018 JetBrains s.r.o. Use of this source code is governed by the Apache 2.0 license
 * that can be found in the LICENSE file.
 */
// port-lint: source kotlin-native/runtime/src/main/kotlin/kotlin/collections/ArrayUtil.kt
// port-lint: source libraries/stdlib/native-wasm/src/kotlin/collections/Arrays.kt
// port-lint: source kotlin-native/runtime/src/main/kotlin/generated/_ArraysNative.kt
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/collections/ArrayUtil.kt:13-97
#pragma once

#include "../Array.hpp"
#include "../IntArray.hpp"
#include <algorithm>
#include <bit>
#include <concepts>
#include <optional>

namespace kotlin::collections {

// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/collections/ArrayUtil.kt:75-82
void check_range_indexes(std::int32_t from_index, std::int32_t to_index, std::int32_t size);

// Transliterated from: libraries/stdlib/native-wasm/src/kotlin/collections/Arrays.kt:14-19
void check_copy_of_range_arguments(std::int32_t from_index, std::int32_t to_index, std::int32_t size);

namespace detail {
// NOTE(port): Access the existing compiler-owned array's actual slots, including
// its uninitialized slots. This does not implement Native ArrayHeader layout,
// GC allocation, or UpdateHeapRef. Native reference arrays still require those
// runtime operations; no runtime object is reinterpreted as this C++ storage.
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/Array.kt:41-43
class ArrayStorageAccess {
 public:
  // Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/Array.kt:41-43
  template <typename T>
  static Array<T> allocate(std::int32_t size) {
    return Array<T>(size, {});
  }
  // Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/collections/ArrayUtil.kt:34-36
  template <typename T>
  static std::optional<T>& slot(Array<T>& array, std::int32_t index) {
    return array.storage_->at(static_cast<std::size_t>(index));
  }
  // Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/collections/ArrayUtil.kt:97-97
  template <typename T>
  static const std::optional<T>& slot(const Array<T>& array, std::int32_t index) {
    return array.storage_->at(static_cast<std::size_t>(index));
  }
};

// NOTE(port): This is the source object-array fill loop over compiler-owned
// slots. A disengaged optional models the reset-to-uninitialized operation,
// while an engaged nullable value remains an initialized value.
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/collections/ArrayUtil.kt:38-40
// Transliterated from: kotlin-native/runtime/src/main/cpp/Arrays.cpp:144-150
template <typename T>
void array_fill_slots(Array<T>& array, std::int32_t from_index,
                      std::int32_t to_index, const std::optional<T>& value) {
  check_range_indexes(from_index, to_index, array.get_size());
  for (std::int32_t index = from_index; index < to_index; ++index) {
    ArrayStorageAccess::slot(array, index) = value;
  }
}
}  // namespace detail

/**
 * Returns an array of objects of the given type with the given [size], initialized with _uninitialized_ values.
 * Attempts to read _uninitialized_ values from this array work in implementation-dependent manner,
 * either throwing exception or returning some kind of implementation-specific default value.
 */
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/collections/ArrayUtil.kt:20-25
template <typename E>
Array<E> array_of_uninitialized_elements(std::int32_t size) {
  if (size < 0) throw std::invalid_argument("capacity must be non-negative.");
  return detail::ArrayStorageAccess::allocate<E>(size);
}

/**
 * Resets an array element at a specified index to some implementation-specific _uninitialized_ value.
 * In particular, references stored in this element are released and become available for garbage collection.
 * Attempts to read _uninitialized_ value work in implementation-dependent manner,
 * either throwing exception or returning some kind of implementation-specific default value.
 */
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/collections/ArrayUtil.kt:34-36
template <typename E>
void reset_at(Array<E>& array, std::int32_t index) {
  detail::ArrayStorageAccess::slot(array, index).reset();
}

// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/collections/ArrayUtil.kt:38-40
// Transliterated from: kotlin-native/runtime/src/main/cpp/Arrays.cpp:144-150
template <typename T>
void array_fill(Array<T>& array, std::int32_t from_index, std::int32_t to_index, T value) {
  detail::array_fill_slots(array, from_index, to_index, std::optional<T>(std::move(value)));
}

/**
 * Resets a range of array elements at a specified [fromIndex] (inclusive) to [toIndex] (exclusive) range of indices
 * to some implementation-specific _uninitialized_ value.
 * In particular, references stored in these elements are released and become available for garbage collection.
 * Attempts to read _uninitialized_ values work in implementation-dependent manner,
 * either throwing exception or returning some kind of implementation-specific default value.
 */
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/collections/ArrayUtil.kt:91-93
template <typename E>
void reset_range(Array<E>& array, std::int32_t from_index, std::int32_t to_index) {
  detail::array_fill_slots<E>(array, from_index, to_index, std::nullopt);
}

// NOTE(port): Preserve Native's direction and bounds order, copying slots
// without reading their values. Uninitialized slots therefore remain so in a
// resized copy. The source UpdateHeapRef becomes real C++ slot assignment for
// this compiler storage; this does not replace the Native heap write barrier.
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/collections/ArrayUtil.kt:95-97
// Transliterated from: kotlin-native/runtime/src/main/cpp/Arrays.cpp:152-172
template <typename T, typename DestinationT>
  requires (std::same_as<T, DestinationT> || std::same_as<std::optional<T>, DestinationT>)
void array_copy(const Array<T>& array, std::int32_t from_index, Array<DestinationT>& destination,
                std::int32_t to_index, std::int32_t count) {
  if (count < 0 || from_index < 0 ||
      static_cast<std::uint32_t>(count) + static_cast<std::uint32_t>(from_index) >
          static_cast<std::uint32_t>(array.get_size()) ||
      to_index < 0 ||
      static_cast<std::uint32_t>(count) + static_cast<std::uint32_t>(to_index) >
          static_cast<std::uint32_t>(destination.get_size())) {
    throw std::out_of_range("");
  }
  if (from_index >= to_index) {
    for (std::int32_t index = 0; index < count; ++index) {
      detail::ArrayStorageAccess::slot(destination, to_index + index) =
          detail::ArrayStorageAccess::slot(array, from_index + index);
    }
  } else {
    for (std::int32_t index = count - 1; index >= 0; --index) {
      detail::ArrayStorageAccess::slot(destination, to_index + index) =
          detail::ArrayStorageAccess::slot(array, from_index + index);
    }
  }
}

/**
 * Copies this array or its subrange into the [destination] array and returns that array.
 *
 * It's allowed to pass the same array in the [destination] and even specify the subrange so that it overlaps with the destination range.
 *
 * @param destination the array to copy to.
 * @param destinationOffset the position in the [destination] array to copy to, 0 by default.
 * @param startIndex the beginning (inclusive) of the subrange to copy, 0 by default.
 * @param endIndex the end (exclusive) of the subrange to copy, size of this array by default.
 *
 * @throws IndexOutOfBoundsException or [IllegalArgumentException] when [startIndex] or [endIndex] is out of range of this array indices or when `startIndex > endIndex`.
 * @throws IndexOutOfBoundsException when the subrange doesn't fit into the [destination] array starting at the specified [destinationOffset],
 * or when that index is out of the [destination] array indices range.
 *
 * @return the [destination] array.
 */
// Transliterated from: kotlin-native/runtime/src/main/kotlin/generated/_ArraysNative.kt:849-853
template <typename T, typename DestinationT>
  requires (std::same_as<T, DestinationT> || std::same_as<std::optional<T>, DestinationT>)
Array<DestinationT>& copy_into(const Array<T>& array, Array<DestinationT>& destination,
                    std::int32_t destination_offset, std::int32_t start_index,
                    std::int32_t end_index) {
  // NOTE(port): Kotlin Int subtraction wraps; avoid C++ signed overflow.
  const auto count = std::bit_cast<std::int32_t>(static_cast<std::uint32_t>(end_index) -
                                               static_cast<std::uint32_t>(start_index));
  array_copy(array, start_index, destination, destination_offset, count);
  return destination;
}
// Transliterated from: kotlin-native/runtime/src/main/kotlin/generated/_ArraysNative.kt:849-853
template <typename T, typename DestinationT>
  requires (std::same_as<T, DestinationT> || std::same_as<std::optional<T>, DestinationT>)
Array<DestinationT>& copy_into(const Array<T>& array, Array<DestinationT>& destination,
                    std::int32_t destination_offset = 0, std::int32_t start_index = 0) {
  return copy_into(array, destination, destination_offset, start_index, array.get_size());
}

/**
 * Returns new array which is a copy of the original array's range between [fromIndex] (inclusive)
 * and [toIndex] (exclusive) with new elements filled with **lateinit** _uninitialized_ values.
 * Attempts to read _uninitialized_ values from this array work in implementation-dependent manner,
 * either throwing exception or returning some kind of implementation-specific default value.
 */
// Transliterated from: kotlin-native/runtime/src/main/kotlin/generated/_ArraysNative.kt:1377-1385
template <typename T>
Array<T> copy_of_uninitialized_elements(const Array<T>& array,
                                        std::int32_t from_index, std::int32_t to_index) {
  const auto new_size = std::bit_cast<std::int32_t>(static_cast<std::uint32_t>(to_index) -
                                                  static_cast<std::uint32_t>(from_index));
  if (new_size < 0) {
    throw std::invalid_argument(std::to_string(from_index) + " > " + std::to_string(to_index));
  }
  auto result = array_of_uninitialized_elements<T>(new_size);
  copy_into(array, result, 0, from_index, std::min(to_index, array.get_size()));
  return result;
}

/**
 * Returns new array which is a copy of the original array with new elements filled with **lateinit** _uninitialized_ values.
 * Attempts to read _uninitialized_ values from this array work in implementation-dependent manner,
 * either throwing exception or returning some kind of implementation-specific default value.
 */
// Transliterated from: kotlin-native/runtime/src/main/kotlin/generated/_ArraysNative.kt:1520-1522
template <typename T>
Array<T> copy_of_uninitialized_elements(const Array<T>& array, std::int32_t new_size) {
  return copy_of_uninitialized_elements(array, 0, new_size);
}
}  // namespace kotlin::collections
