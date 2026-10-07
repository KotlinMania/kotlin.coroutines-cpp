/*
 * Copyright 2010-2024 JetBrains s.r.o. and Kotlin Programming Language contributors.
 * Use of this source code is governed by the Apache 2.0 license that can be found in the license/LICENSE.txt file.
 */
// port-lint: source libraries/stdlib/native-wasm/src/kotlin/collections/Arrays.kt
// Transliterated from: libraries/stdlib/native-wasm/src/kotlin/collections/Arrays.kt:93-106
#pragma once

#include "../ArrayIntrinsics.hpp"
#include "Collections.hpp"

namespace kotlin::collections {
// NOTE(port): Concrete Any? instantiations of the consumed internal generic
// functions. Other typed array instantiations require their source array ABI.
// Transliterated from: libraries/stdlib/native-wasm/src/kotlin/collections/Arrays.kt:85-85
Array<std::any> array_of_nulls(const Array<std::any>& reference, std::int32_t size);
// Transliterated from: libraries/stdlib/native-wasm/src/kotlin/collections/Arrays.kt:87-87
Array<std::any> collection_to_array(const Collection<std::any>& collection);
// Transliterated from: libraries/stdlib/native-wasm/src/kotlin/collections/Arrays.kt:89-89
Array<std::any> collection_to_array(const Collection<std::any>& collection,
                                  Array<std::any> array);

// NOTE(port): Native returns the same array, leaving trailing slots unchanged.
// This is the actual Native body, not the JVM null-termination operation.
// Transliterated from: libraries/stdlib/native-wasm/src/kotlin/collections/Arrays.kt:91-91
template <typename T>
Array<T> terminate_collection_to_array([[maybe_unused]] std::int32_t collection_size,
                                       Array<T> array) {
  return array;
}

// Transliterated from: libraries/stdlib/native-wasm/src/kotlin/collections/Arrays.kt:98-106
template <typename E>
Array<typename ::kotlin::detail::NullableArrayElement<E>::Type> copy_of_nulls(
    const Array<E>& array, std::int32_t from_index, std::int32_t to_index) {
  const auto new_size = std::bit_cast<std::int32_t>(static_cast<std::uint32_t>(to_index) -
                                                  static_cast<std::uint32_t>(from_index));
  if (new_size < 0) {
    throw std::invalid_argument(std::to_string(from_index) + " > " + std::to_string(to_index));
  }
  auto result = ::kotlin::array_of_nulls<E>(new_size);
  copy_into(array, result, 0, from_index, std::min(to_index, array.get_size()));
  return result;
}

/**
 * Returns a new array which is a copy of the original array with new elements filled with null values.
 */
// Transliterated from: libraries/stdlib/native-wasm/src/kotlin/collections/Arrays.kt:96-96
template <typename E>
Array<typename ::kotlin::detail::NullableArrayElement<E>::Type> copy_of_nulls(
    const Array<E>& array, std::int32_t new_size) {
  return copy_of_nulls(array, 0, new_size);
}
}  // namespace kotlin::collections
