/*
 * Copyright 2010-2023 JetBrains s.r.o. Use of this source code is governed by the Apache 2.0 license
 * that can be found in the LICENSE file.
 */
// port-lint: source kotlin-native/runtime/src/main/kotlin/kotlin/native/internal/NativePtr.kt
// port-lint: source kotlin-native/runtime/src/main/kotlin/kotlin/Primitives.kt
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/native/internal/NativePtr.kt:29-34
#include "KClassPointerHash.hpp"
#include <bit>

namespace kotlin::native::internal::detail {
// NOTE(port): NativePtr's intrinsic conversion retains pointer bits in Long.
// Widen unsigned pointer storage, apply source unsigned shift/xor and truncate
// to Int bits. No Native object header or Kotlin runtime operation is involved.
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/native/internal/NativePtr.kt:29-34
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/Primitives.kt:1851-1852
std::int32_t hash_type_info_pointer(const void* type_info) {
  const auto pointer_bits = static_cast<std::uint64_t>(
      reinterpret_cast<std::uintptr_t>(type_info));
  return std::bit_cast<std::int32_t>(
      static_cast<std::uint32_t>((pointer_bits >> 32) ^ pointer_bits));
}
}  // namespace kotlin::native::internal::detail
