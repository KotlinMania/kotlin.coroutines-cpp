/*
 * Copyright 2010-2018 JetBrains s.r.o. Use of this source code is governed by the Apache 2.0 license
 * that can be found in the LICENSE file.
 */
// port-lint: source kotlin-native/runtime/src/main/kotlin/kotlin/collections/ArrayUtil.kt
// port-lint: source kotlin-native/runtime/src/main/cpp/Arrays.cpp
// port-lint: source kotlin-native/runtime/src/main/kotlin/kotlin/Arrays.kt
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/collections/ArrayUtil.kt:34-97
#pragma once

#include "../../../../KotlinGCBridge.hpp"

// NOTE(port): These are the source Native external entry points, operating on
// actual GC-managed arrays. There is no conversion to compiler-owned Array<T>.
// Runtime definitions are mandatory. An object get receives the caller's root.
extern "C" {
// Transliterated from: kotlin-native/runtime/src/main/cpp/Arrays.cpp:123-125
ObjHeader* Kotlin_Array_get(const ObjHeader* array, std::int32_t index, ObjHeader** result);
// Transliterated from: kotlin-native/runtime/src/main/cpp/Arrays.cpp:131-133
void Kotlin_Array_set(ObjHeader* array, std::int32_t index, const ObjHeader* value);
// Transliterated from: kotlin-native/runtime/src/main/cpp/Arrays.cpp:139-142
std::int32_t Kotlin_Array_getArrayLength(const ObjHeader* array);
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/collections/ArrayUtil.kt:38-40
// Transliterated from: kotlin-native/runtime/src/main/cpp/Arrays.cpp:144-150
void Kotlin_Array_fillImpl(ObjHeader* array, std::int32_t from_index,
                           std::int32_t to_index, ObjHeader* value);
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/collections/ArrayUtil.kt:95-97
// Transliterated from: kotlin-native/runtime/src/main/cpp/Arrays.cpp:152-172
void Kotlin_Array_copyImpl(const ObjHeader* array, std::int32_t from_index,
                           ObjHeader* destination, std::int32_t to_index, std::int32_t count);
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/Arrays.kt:291-293
// Transliterated from: kotlin-native/runtime/src/main/cpp/Arrays.cpp:519-521
std::int32_t Kotlin_IntArray_get(const ObjHeader* array, std::int32_t index);
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/Arrays.kt:305-307
// Transliterated from: kotlin-native/runtime/src/main/cpp/Arrays.cpp:527-529
void Kotlin_IntArray_set(ObjHeader* array, std::int32_t index, std::int32_t value);
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/Arrays.kt:319-321
// Transliterated from: kotlin-native/runtime/src/main/cpp/Arrays.cpp:535-538
std::int32_t Kotlin_IntArray_getArrayLength(const ObjHeader* array);
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/collections/ArrayUtil.kt:54-56
// Transliterated from: kotlin-native/runtime/src/main/cpp/Arrays.cpp:552-554
void Kotlin_IntArray_fillImpl(ObjHeader* array, std::int32_t from_index,
                              std::int32_t to_index, std::int32_t value);
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/collections/ArrayUtil.kt:111-113
// Transliterated from: kotlin-native/runtime/src/main/cpp/Arrays.cpp:587-590
void Kotlin_IntArray_copyImpl(const ObjHeader* array, std::int32_t from_index,
                              ObjHeader* destination, std::int32_t to_index, std::int32_t count);

}

namespace kotlin::collections {
/**
 * Resets an array element at a specified index to some implementation-specific _uninitialized_ value.
 * In particular, references stored in this element are released and become available for garbage collection.
 * Attempts to read _uninitialized_ value work in implementation-dependent manner,
 * either throwing exception or returning some kind of implementation-specific default value.
 */
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/collections/ArrayUtil.kt:34-36
void reset_at(ObjHeader* array, std::int32_t index);
/**
 * Resets a range of array elements at a specified [fromIndex] (inclusive) to [toIndex] (exclusive) range of indices
 * to some implementation-specific _uninitialized_ value.
 * In particular, references stored in these elements are released and become available for garbage collection.
 * Attempts to read _uninitialized_ values work in implementation-dependent manner,
 * either throwing exception or returning some kind of implementation-specific default value.
 */
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/collections/ArrayUtil.kt:91-93
void reset_range(ObjHeader* array, std::int32_t from_index, std::int32_t to_index);
}  // namespace kotlin::collections
