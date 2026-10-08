/*
 * Copyright 2010-2024 JetBrains s.r.o. and Kotlin Programming Language contributors.
 * Use of this source code is governed by the Apache 2.0 license that can be found in the license/LICENSE.txt file.
 */
// port-lint: source kotlin-native/runtime/src/main/kotlin/kotlin/ArrayIntrinsics.kt
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/ArrayIntrinsics.kt:12-20
#pragma once

#include "collections/ArrayUtil.hpp"

namespace kotlin {
// NOTE(port): Concrete Any? instantiation consumed by compiler collection
// conversion. Other Array<T> instantiations require their source array ABI;
// this compiler-owned specialization does not replace Native Kotlin_emptyArray.
// The return value is statically allocated and immutable;
// we can treat it as non-escaping
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/ArrayIntrinsics.kt:31-35
Array<std::any> empty_array();

namespace detail {
// NOTE(port): Kotlin T? is idempotent. Pointer, shared-handle, optional and
// type-erased Any values already have a null representation in compiler storage.
// This type mapping does not define Native object allocation or array layout.
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/ArrayIntrinsics.kt:18-20
template <typename T> struct NullableArrayElement { using Type = std::optional<T>; };
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/ArrayIntrinsics.kt:18-20
template <typename T> struct NullableArrayElement<std::optional<T>> { using Type = std::optional<T>; };
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/ArrayIntrinsics.kt:18-20
template <typename T> struct NullableArrayElement<T*> { using Type = T*; };
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/ArrayIntrinsics.kt:18-20
template <typename T> struct NullableArrayElement<std::shared_ptr<T>> { using Type = std::shared_ptr<T>; };
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/ArrayIntrinsics.kt:18-20
template <> struct NullableArrayElement<std::any> { using Type = std::any; };
}  // namespace detail

/**
 * Returns an array of objects of the given type with the given [size], initialized with null values.
 *
 * @throws RuntimeException if the specified [size] is negative.
 */
// NOTE(port): The Native intrinsic's allocation yields null object-reference
// slots. Compiler-owned C++ storage explicitly initializes its nullable values;
// its uninitialized slots are distinct from initialized nulls. No runtime object
// is converted to this storage and no Native allocator or GC barrier is replaced.
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/ArrayIntrinsics.kt:18-20
template <typename T>
Array<typename detail::NullableArrayElement<T>::Type> array_of_nulls(std::int32_t size) {
  using Element = typename detail::NullableArrayElement<T>::Type;
  auto result = collections::array_of_uninitialized_elements<Element>(size);
  collections::array_fill(result, 0, size, Element{});
  return result;
}
}  // namespace kotlin
