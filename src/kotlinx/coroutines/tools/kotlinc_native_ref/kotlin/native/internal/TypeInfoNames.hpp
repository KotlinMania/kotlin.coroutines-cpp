/*
 * Copyright 2010-2024 JetBrains s.r.o. and Kotlin Programming Language contributors.
 * Use of this source code is governed by the Apache 2.0 license that can be found in the license/LICENSE.txt file.
 */
// port-lint: source kotlin-native/runtime/src/main/kotlin/kotlin/native/internal/TypeInfoNames.kt
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/native/internal/TypeInfoNames.kt:13-54
#pragma once

#include <optional>
#include <string>

struct TypeInfo;

namespace kotlin::native::internal {
namespace detail { struct CompilerClassInfo; }
// NOTE(port): The source opaque metadata pointer uses an explicit ABI transport.
// Actual Native TypeInfo and compiler-owned C++ metadata remain distinct.
// Nullable String results are copied UTF-16 values, with no transferred Native
// reference ownership or reinterpretation of C++ objects as Native objects.
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/native/internal/TypeInfoNames.kt:13-46
class TypeInfoNames final {
 public:
  // Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/native/internal/TypeInfoNames.kt:13-16
  explicit TypeInfoNames(const ::TypeInfo* type_info_ptr);
  // NOTE(port): The private compiler ABI has no Native object/string allocation.
  // Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/native/internal/TypeInfoNames.kt:13-16
  explicit TypeInfoNames(const detail::CompilerClassInfo& type_info);

  /**
   * The last component of the name if it exists and is allowed by TF_REFLECTION_SHOW_REL_NAME.
   */
  // Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/native/internal/TypeInfoNames.kt:18-25
  std::optional<std::u16string> simple_name() const;

  /**
   * The fully qualified name if it exists and is allowed by both TF_REFLECTION_SHOW_REL_NAME and TF_REFLECTION_SHOW_PKG_NAME
   */
  // Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/native/internal/TypeInfoNames.kt:27-35
  std::optional<std::u16string> qualified_name() const;

  /**
   * The fully qualified name if it exists. Ignores TF_REFLECTION_SHOW_REL_NAME and TF_REFLECTION_SHOW_PKG_NAME
   */
  // Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/native/internal/TypeInfoNames.kt:37-45
  std::optional<std::u16string> full_name() const;

 private:
  using NameGetter = std::optional<std::u16string> (*)(const void*, bool);
  // Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/native/internal/TypeInfoNames.kt:13-16
  TypeInfoNames(const void* type_info_ptr, NameGetter package_name,
                NameGetter relative_name);
  const void* type_info_ptr_;
  const NameGetter package_name_;
  const NameGetter relative_name_;
};
}  // namespace kotlin::native::internal
