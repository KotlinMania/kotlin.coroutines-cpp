/*
 * Copyright 2010-2024 JetBrains s.r.o. and Kotlin Programming Language contributors.
 * Use of this source code is governed by the Apache 2.0 license that can be found in the license/LICENSE.txt file.
 */
// port-lint: source kotlin-native/runtime/src/main/kotlin/kotlin/ArrayIntrinsics.kt
// port-lint: source kotlin-native/runtime/src/main/cpp/Arrays.cpp
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/ArrayIntrinsics.kt:31-35
#include "ArrayIntrinsics.hpp"

namespace kotlin {
namespace {
// NOTE(port): Native's generated immutable empty object is represented by one
// fixed-length compiler-owned array. Returning its handle preserves the storage;
// no valid element index permits mutation. Only the consumed Any? instantiation
// is supplied here, without claiming a general Native array representation.
// Transliterated from: kotlin-native/runtime/src/main/cpp/Arrays.cpp:118-118
const Array<std::any> THE_EMPTY_ARRAY =
    collections::array_of_uninitialized_elements<std::any>(0);
}  // namespace

// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/ArrayIntrinsics.kt:31-35
// Transliterated from: kotlin-native/runtime/src/main/cpp/Arrays.cpp:175-177
Array<std::any> empty_array() {
  return THE_EMPTY_ARRAY;
}
}  // namespace kotlin
