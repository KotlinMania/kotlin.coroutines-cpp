/*
 * Copyright 2010-2026 JetBrains s.r.o. and Kotlin Programming Language contributors.
 * Use of this source code is governed by the Apache 2.0 license that can be found in the license/LICENSE.txt file.
 */
// port-lint: source kotlin-native/runtime/src/main/kotlin/generated/_ArraysNative.kt
// Transliterated from: kotlin-native/runtime/src/main/kotlin/generated/_ArraysNative.kt:922-1654
#pragma once
#include "../IntArray.hpp"
#include "Arrays.hpp"
#include "ArrayUtil.hpp"
namespace kotlin::collections {
/**
 * Returns a new array which is a copy of the specified range of the original array.
 * 
 * @param fromIndex the start of the range (inclusive) to copy.
 * @param toIndex the end of the range (exclusive) to copy.
 * 
 * @throws IndexOutOfBoundsException if [fromIndex] is less than zero or [toIndex] is greater than the size of this array.
 * @throws IllegalArgumentException if [fromIndex] is greater than [toIndex].
 */
// Transliterated from: kotlin-native/runtime/src/main/kotlin/generated/_ArraysNative.kt:1254-1257
template <typename T>
Array<T> copy_of_range(const Array<T>& array, std::int32_t from_index,
                       std::int32_t to_index) {
  check_copy_of_range_arguments(from_index, to_index, array.get_size());
  return copy_of_uninitialized_elements(array, from_index, to_index);
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
// Transliterated from: kotlin-native/runtime/src/main/kotlin/generated/_ArraysNative.kt:922-925
IntArray& copy_into(const IntArray& array, IntArray& destination, std::int32_t destination_offset, std::int32_t start_index, std::int32_t end_index);

// Transliterated from: kotlin-native/runtime/src/main/kotlin/generated/_ArraysNative.kt:922-925
IntArray& copy_into(const IntArray& array, IntArray& destination, std::int32_t destination_offset = 0, std::int32_t start_index = 0);

/**
 * Returns new array which is a copy of the original array.
 * 
 * @sample samples.collections.Arrays.CopyOfOperations.copyOf
 */
// Transliterated from: kotlin-native/runtime/src/main/kotlin/generated/_ArraysNative.kt:1079-1081
IntArray copy_of(const IntArray& array);

/**
 * Returns new array which is a copy of the original array, resized to the given [newSize].
 * The copy is either truncated or padded at the end with zero values if necessary.
 * 
 * - If [newSize] is less than the size of the original array, the copy array is truncated to the [newSize].
 * - If [newSize] is greater than the size of the original array, the extra elements in the copy array are filled with zero values.
 * 
 * @sample samples.collections.Arrays.CopyOfOperations.resizedPrimitiveCopyOf
 */
// Transliterated from: kotlin-native/runtime/src/main/kotlin/generated/_ArraysNative.kt:1163-1165
IntArray copy_of(const IntArray& array, std::int32_t new_size);

/**
 * Returns a new array which is a copy of the specified range of the original array.
 * 
 * @param fromIndex the start of the range (inclusive) to copy.
 * @param toIndex the end of the range (exclusive) to copy.
 * 
 * @throws IndexOutOfBoundsException if [fromIndex] is less than zero or [toIndex] is greater than the size of this array.
 * @throws IllegalArgumentException if [fromIndex] is greater than [toIndex].
 */
// Transliterated from: kotlin-native/runtime/src/main/kotlin/generated/_ArraysNative.kt:1296-1299
IntArray copy_of_range(const IntArray& array, std::int32_t from_index, std::int32_t to_index);

/**
 * Returns new array which is a copy of the original array's range between [fromIndex] (inclusive)
 * and [toIndex] (exclusive) with new elements filled with **lateinit** _uninitialized_ values.
 * Attempts to read _uninitialized_ values from this array work in implementation-dependent manner,
 * either throwing exception or returning some kind of implementation-specific default value.
 */
// Transliterated from: kotlin-native/runtime/src/main/kotlin/generated/_ArraysNative.kt:1425-1433
IntArray copy_of_uninitialized_elements(const IntArray& array, std::int32_t from_index, std::int32_t to_index);

/**
 * Returns new array which is a copy of the original array with new elements filled with **lateinit** _uninitialized_ values.
 * Attempts to read _uninitialized_ values from this array work in implementation-dependent manner,
 * either throwing exception or returning some kind of implementation-specific default value.
 */
// Transliterated from: kotlin-native/runtime/src/main/kotlin/generated/_ArraysNative.kt:1547-1549
IntArray copy_of_uninitialized_elements(const IntArray& array, std::int32_t new_size);

/**
 * Fills this array or its subrange with the specified [element] value.
 * 
 * @param fromIndex the start of the range (inclusive) to fill, 0 by default.
 * @param toIndex the end of the range (exclusive) to fill, size of this array by default.
 * 
 * @throws IndexOutOfBoundsException if [fromIndex] is less than zero or [toIndex] is greater than the size of this array.
 * @throws IllegalArgumentException if [fromIndex] is greater than [toIndex].
 */
// Transliterated from: kotlin-native/runtime/src/main/kotlin/generated/_ArraysNative.kt:1652-1654
void fill(IntArray& array, std::int32_t element, std::int32_t from_index, std::int32_t to_index);

// Transliterated from: kotlin-native/runtime/src/main/kotlin/generated/_ArraysNative.kt:1652-1654
void fill(IntArray& array, std::int32_t element, std::int32_t from_index = 0);

/**
 * Returns new array which is a copy of the original array, resized to the given [newSize].
 * The copy is either truncated or padded at the end with `null` values if necessary.
 * 
 * - If [newSize] is less than the size of the original array, the copy array is truncated to the [newSize].
 * - If [newSize] is greater than the size of the original array, the extra elements in the copy array are filled with `null` values.
 * 
 * @sample samples.collections.Arrays.CopyOfOperations.resizingCopyOf
 */
// Transliterated from: kotlin-native/runtime/src/main/kotlin/generated/_ArraysNative.kt:1241-1243
template <typename T>
Array<typename ::kotlin::detail::NullableArrayElement<T>::Type> copy_of(
    const Array<T>& array, std::int32_t new_size) {
  return copy_of_nulls(array, new_size);
}
}  // namespace kotlin::collections
