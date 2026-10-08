/*
 * Copyright 2010-2024 JetBrains s.r.o. and Kotlin Programming Language contributors.
 * Use of this source code is governed by the Apache 2.0 license that can be found in the license/LICENSE.txt file.
 */
// port-lint: source kotlin-native/runtime/src/main/kotlin/kotlin/native/internal/TypeInfoHolder.kt
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/native/internal/TypeInfoHolder.kt:12-14
#pragma once



namespace kotlin::native::internal {
namespace detail { struct CompilerClassInfo; }
// NOTE(port): The consumed NativePtr borrows compiler-emitted C++ metadata.
// It is distinct from Native TypeInfo and transfers no ownership.
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/native/internal/TypeInfoHolder.kt:12-14
class [[clang::annotate("kotlin.class:kotlin.native.internal:TypeInfoHolder:interface")]] TypeInfoHolder {
 public:
  virtual ~TypeInfoHolder() = default;
  // Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/native/internal/TypeInfoHolder.kt:13-13
  virtual const detail::CompilerClassInfo* type_info() const = 0;
 protected:
  TypeInfoHolder() = default;
};
}  // namespace kotlin::native::internal
