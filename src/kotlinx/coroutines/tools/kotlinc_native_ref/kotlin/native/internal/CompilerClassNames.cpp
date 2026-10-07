/*
 * Copyright 2010-2022 JetBrains s.r.o. Use of this source code is governed by the Apache 2.0 license
 * that can be found in the LICENSE file.
 */
// port-lint: source kotlin-native/runtime/src/main/cpp/Types.cpp
// port-lint: source kotlin-native/runtime/src/main/kotlin/kotlin/native/internal/TypeInfoNames.kt
// Transliterated from: kotlin-native/runtime/src/main/cpp/Types.cpp:53-69
#include "CompilerClassInfo.hpp"
#include "TypeInfoNames.hpp"

namespace kotlin::native::internal {
namespace {
// NOTE(port): Read only actual compiler-emitted metadata, using the source flags
// and nullable-field rules. The typed ABI binding is separate from Native roots.
// Transliterated from: kotlin-native/runtime/src/main/cpp/Types.cpp:53-60
std::optional<std::u16string> get_package_name(const void* type_info,
                                             bool check_flags) {
  const auto* info = static_cast<const detail::CompilerClassInfo*>(type_info);
  if (!check_flags || (info->flags & detail::TF_REFLECTION_SHOW_PKG_NAME)) {
    return info->package_name != nullptr
        ? std::optional<std::u16string>(std::in_place, info->package_name,
                                        info->package_name_size)
        : std::nullopt;
  } else {
    return std::nullopt;
  }
}

// Transliterated from: kotlin-native/runtime/src/main/cpp/Types.cpp:62-69
std::optional<std::u16string> get_relative_name(const void* type_info,
                                              bool check_flags) {
  const auto* info = static_cast<const detail::CompilerClassInfo*>(type_info);
  if (!check_flags || (info->flags & detail::TF_REFLECTION_SHOW_REL_NAME)) {
    return info->relative_name != nullptr
        ? std::optional<std::u16string>(std::in_place, info->relative_name,
                                        info->relative_name_size)
        : std::nullopt;
  } else {
    return std::nullopt;
  }
}
}  // namespace

// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/native/internal/TypeInfoNames.kt:13-16
TypeInfoNames::TypeInfoNames(const detail::CompilerClassInfo& type_info)
    : TypeInfoNames(&type_info, get_package_name, get_relative_name) {}
}  // namespace kotlin::native::internal
