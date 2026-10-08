/*
 * Copyright 2010-2018 JetBrains s.r.o. Use of this source code is governed by the Apache 2.0 license
 * that can be found in the LICENSE file.
 */
// port-lint: source kotlin-native/runtime/src/main/kotlin/kotlin/collections/ArrayUtil.kt
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/collections/ArrayUtil.kt:34-93
#include "NativeArrayUtil.hpp"

namespace kotlin::collections {
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/collections/ArrayUtil.kt:34-36
void reset_at(ObjHeader* array, std::int32_t index) {
  Kotlin_Array_set(array, index, nullptr);
}
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/collections/ArrayUtil.kt:91-93
void reset_range(ObjHeader* array, std::int32_t from_index, std::int32_t to_index) {
  Kotlin_Array_fillImpl(array, from_index, to_index, nullptr);
}
}  // namespace kotlin::collections
