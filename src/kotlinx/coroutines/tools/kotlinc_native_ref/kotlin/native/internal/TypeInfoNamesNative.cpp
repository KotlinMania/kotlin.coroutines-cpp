/*
 * Copyright 2010-2024 JetBrains s.r.o. and Kotlin Programming Language contributors.
 * Use of this source code is governed by the Apache 2.0 license that can be found in the license/LICENSE.txt file.
 */
// port-lint: source kotlin-native/runtime/src/main/kotlin/kotlin/native/internal/TypeInfoNames.kt
// port-lint: source kotlin-native/runtime/src/main/kotlin/kotlin/String.kt
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/native/internal/TypeInfoNames.kt:13-54
#include "TypeInfoNames.hpp"
#include "../../../../../KotlinGCBridge.hpp"


// NOTE(port): Strong calls into the linked Native runtime. Object-returning
// entries receive a real result root; metadata retains the source void* ABI.
extern "C" {
// The return value is stored in a global.
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/native/internal/TypeInfoNames.kt:48-50
ObjHeader* Kotlin_TypeInfo_getPackageName(void* type_info, bool check_flags,
                                         ObjHeader** result);
// The return value is stored in a global.
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/native/internal/TypeInfoNames.kt:52-54
ObjHeader* Kotlin_TypeInfo_getRelativeName(void* type_info, bool check_flags,
                                          ObjHeader** result);
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/String.kt:64-66
std::int32_t Kotlin_String_getStringLength(const ObjHeader* text);
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/String.kt:48-51
std::uint16_t Kotlin_String_get(const ObjHeader* text, std::int32_t index);
}

namespace kotlin::native::internal {
namespace {
// NOTE(port): Copy the source String value through its actual length/get ABI;
// the caller's ObjHolder retains its Native root throughout the copy. This
// representation adapter does not implement or replace Native string storage.
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/String.kt:40-51
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/String.kt:64-66
std::optional<std::u16string> copy_string(const ObjHeader* text) {
  if (text == nullptr) return std::nullopt;
  const auto length = Kotlin_String_getStringLength(text);
  std::u16string value;
  value.reserve(static_cast<std::size_t>(length));
  for (std::int32_t index = 0; index < length; ++index) {
    value.push_back(static_cast<char16_t>(Kotlin_String_get(text, index)));
  }
  return value;
}

// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/native/internal/TypeInfoNames.kt:48-50
std::optional<std::u16string> get_package_name(const void* type_info,
                                             bool check_flags) {
  ObjHolder result;
  return copy_string(Kotlin_TypeInfo_getPackageName(
      const_cast<void*>(type_info), check_flags, result.slot()));
}

// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/native/internal/TypeInfoNames.kt:52-54
std::optional<std::u16string> get_relative_name(const void* type_info,
                                              bool check_flags) {
  ObjHolder result;
  return copy_string(Kotlin_TypeInfo_getRelativeName(
      const_cast<void*>(type_info), check_flags, result.slot()));
}
}  // namespace

// NOTE(port): Only this transport unit depends on actual Native roots/runtime.
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/native/internal/TypeInfoNames.kt:13-16
TypeInfoNames::TypeInfoNames(const ::TypeInfo* type_info_ptr)
    : TypeInfoNames(type_info_ptr, get_package_name, get_relative_name) {}
}  // namespace kotlin::native::internal
