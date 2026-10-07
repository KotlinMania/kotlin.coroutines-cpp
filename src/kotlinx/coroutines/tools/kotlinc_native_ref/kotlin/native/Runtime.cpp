/*
 * Copyright 2010-2018 JetBrains s.r.o. Use of this source code is governed by the Apache 2.0 license
 * that can be found in the LICENSE file.
 */
// port-lint: source kotlin-native/runtime/src/main/kotlin/kotlin/native/Runtime.kt
// port-lint: source kotlin-native/runtime/src/main/cpp/Natives.cpp
// Transliterated from: kotlin-native/runtime/src/main/cpp/Natives.cpp:40-49
#include "Runtime.hpp"
#include <bit>

namespace kotlin::native {
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/native/Runtime.kt:97-100
// Transliterated from: kotlin-native/runtime/src/main/cpp/Natives.cpp:40-49
std::int32_t identity_hash_code(const void* object) {
  // NOTE: `Any?.identityHashCode()` is used in Blackhole implementations of both kotlinx-benchmark and
  //        K/N's own benchmarks. These usages rely on this being an intrinsic property of the object.
  //        So, calling `obj.identityHashCode()` should be seen by the optimizer as reading the entire
  //        `obj` memory, and any changes to `obj` beforehand couldn't be optimized away. Additionally,
  //        it should be very cheap to call in order not to pollute the time measurements.
  // Here we will use different mechanism for stable hashcode, using meta-objects
  // if moving collector will be used.
  // NOTE(port): Preserve KInt's signed low 32 bits without a narrowing warning.
  return std::bit_cast<std::int32_t>(
      static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(object)));
}
}  // namespace kotlin::native
