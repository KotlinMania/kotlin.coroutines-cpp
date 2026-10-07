/*
 * Copyright 2010-2023 JetBrains s.r.o. Use of this source code is governed by the Apache 2.0 license
 * that can be found in the LICENSE file.
 */
// port-lint: source kotlin-native/runtime/src/main/kotlin/kotlin/native/internal/NativePtr.kt
// port-lint: source kotlin-native/runtime/src/main/kotlin/kotlin/Primitives.kt
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/native/internal/NativePtr.kt:34-34
#pragma once
#include <cstdint>

struct TypeInfo;

namespace kotlin::native::internal::detail {
// NOTE(port): The consumed pointer hash is a nongeneric algorithm used by the
// generic KClassImpl body. It neither reads metadata nor owns the pointer.
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/native/internal/NativePtr.kt:34-34
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/Primitives.kt:1851-1852
std::int32_t hash_type_info_pointer(const void* type_info);
}  // namespace kotlin::native::internal::detail
