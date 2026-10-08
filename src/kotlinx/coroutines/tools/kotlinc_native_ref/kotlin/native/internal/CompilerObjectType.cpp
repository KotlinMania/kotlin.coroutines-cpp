/*
 * Copyright 2010-2022 JetBrains s.r.o. Use of this source code is governed by the Apache 2.0 license
 * that can be found in the LICENSE file.
 */
// port-lint: source kotlin-native/runtime/src/main/cpp/Natives.cpp
// port-lint: source kotlin-native/runtime/src/main/cpp/Types.cpp
// port-lint: source kotlin-native/runtime/src/main/kotlin/kotlin/native/internal/KClassImpl.kt
// Transliterated from: kotlin-native/runtime/src/main/cpp/Types.cpp:16-37
#include "CompilerClassInfo.hpp"
#include "KClassImpl.hpp"

namespace kotlin::native::internal {
// NOTE(port): The private C++ object ABI obtains the actual dynamic declaration
// through the Clang-generated accessor. No C++ object becomes Native ObjHeader.
// Transliterated from: kotlin-native/runtime/src/main/cpp/Natives.cpp:110-112
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/native/internal/KClassImpl.kt:72-76
const detail::CompilerClassInfo* get_object_type_info(const ::kotlin::Any& object) {
  return object.__kxs_compiler_type_info();
}

namespace {
// Transliterated from: kotlin-native/runtime/src/main/cpp/Types.cpp:23-37
bool is_subtype(const detail::CompilerClassInfo* obj_type_info,
                const detail::CompilerClassInfo* type_info) {
  // If it is an interface - check in list of implemented interfaces.
  if ((type_info->flags & detail::TF_INTERFACE) != 0) {
    for (std::int32_t index = 0;
         index < obj_type_info->implemented_interfaces_count; ++index) {
      if (obj_type_info->implemented_interfaces[index] == type_info) {
        return true;
      }
    }
    return false;
  }
  while (obj_type_info != nullptr && obj_type_info != type_info) {
    obj_type_info = obj_type_info->super_type;
  }
  return obj_type_info != nullptr;
}
}  // namespace

// Transliterated from: kotlin-native/runtime/src/main/cpp/Types.cpp:16-21
// Transliterated from: kotlin-native/runtime/src/main/cpp/Types.cpp:49-51
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/native/internal/KClassImpl.kt:79-81
bool is_instance(const ::kotlin::Any& object,
                 const detail::CompilerClassInfo* type_info) {
  // We assume null check is handled by caller.
  const auto* obj_type_info = get_object_type_info(object);
  return is_subtype(obj_type_info, type_info);
}
}  // namespace kotlin::native::internal
