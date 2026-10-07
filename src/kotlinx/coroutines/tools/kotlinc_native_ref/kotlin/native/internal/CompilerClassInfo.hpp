/*
 * Copyright 2010-2022 JetBrains s.r.o. Use of this source code is governed by the Apache 2.0 license
 * that can be found in the LICENSE file.
 */
// port-lint: source kotlin-native/runtime/src/main/cpp/TypeInfo.h
// Transliterated from: kotlin-native/runtime/src/main/cpp/TypeInfo.h:43-58
// Transliterated from: kotlin-native/runtime/src/main/cpp/TypeInfo.h:97-136
#pragma once

#include <cstddef>
#include <cstdint>

namespace kotlin::native::internal::detail {
// NOTE(port): Private C++ compiler ABI projection of the consumed TypeInfo
// fields. Clang emits it from actual translated declarations. This is neither
// Native TypeInfo nor an object header, vtable, GC descriptor or frame ABI.
// Source String fields use immutable compiler-emitted UTF-16 literals here.
// Transliterated from: kotlin-native/runtime/src/main/cpp/TypeInfo.h:97-136
struct [[clang::annotate("kotlin.compiler.class_info")]] CompilerClassInfo final {
  const CompilerClassInfo* super_type;
  const CompilerClassInfo* const* implemented_interfaces;
  std::int32_t implemented_interfaces_count;
  const char16_t* package_name;
  std::size_t package_name_size;
  const char16_t* relative_name;
  std::size_t relative_name_size;
  std::int32_t flags;
};

// Keep in sync with constants in RTTIGenerator.
// Transliterated from: kotlin-native/runtime/src/main/cpp/TypeInfo.h:48-48
inline constexpr std::int32_t TF_INTERFACE = 1 << 2;
// Transliterated from: kotlin-native/runtime/src/main/cpp/TypeInfo.h:54-54
inline constexpr std::int32_t TF_REFLECTION_SHOW_PKG_NAME = 1 << 8;
// Transliterated from: kotlin-native/runtime/src/main/cpp/TypeInfo.h:55-55
inline constexpr std::int32_t TF_REFLECTION_SHOW_REL_NAME = 1 << 9;
}  // namespace kotlin::native::internal::detail
