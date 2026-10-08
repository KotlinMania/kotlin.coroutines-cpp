/*
 * Copyright 2010-2023 JetBrains s.r.o. Use of this source code is governed by the Apache 2.0 license
 * that can be found in the LICENSE file.
 */
// port-lint: source kotlin-native/runtime/src/main/kotlin/kotlin/native/internal/KClassImpl.kt
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/native/internal/KClassImpl.kt:16-43
#pragma once

#include "../../reflect/KClass.hpp"
#include "TypeInfoHolder.hpp"
#include "CompilerClassInfo.hpp"
#include "TypeInfoNames.hpp"
#include "KClassPointerHash.hpp"

namespace kotlin::native::internal {
// NOTE(port): Source NativePtr is bound to real compiler-owned C++ class data.
// Actual Kotlin/Native TypeInfo and object headers use a distinct interop ABI.
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/native/internal/KClassImpl.kt:72-76
const detail::CompilerClassInfo* get_object_type_info(const ::kotlin::Any& object);
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/native/internal/KClassImpl.kt:79-81
bool is_instance(const ::kotlin::Any& object, const detail::CompilerClassInfo* type_info);

// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/native/internal/KClassImpl.kt:39-40
const detail::CompilerClassInfo* type_info_ptr(const ::kotlin::reflect::detail::KClassObject& klass);
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/native/internal/KClassImpl.kt:42-43
std::optional<std::u16string> full_name(const ::kotlin::reflect::detail::KClassObject& klass);

// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/native/internal/KClassImpl.kt:16-37
template <typename T>
class [[clang::annotate("kotlin.class:kotlin.native.internal:KClassImpl:class")]] KClassImpl final : public ::kotlin::reflect::KClass<T>, public TypeInfoHolder {
 public:
  // Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/native/internal/KClassImpl.kt:16-16
  explicit KClassImpl(const detail::CompilerClassInfo* type_info) : type_info_(type_info) {}

  // NOTE(port): This genuine source constructor is a compiler constant intrinsic.
  // Its source diagnostic body is unreachable after lowering; the port requires
  // actual lowering and supplies no zero/default metadata implementation.
  // Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/native/internal/KClassImpl.kt:18-21
  [[clang::annotate("kotlin.native.internal.ConstantConstructorIntrinsic:KCLASS_IMPL")]]
  KClassImpl();

  // Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/native/internal/KClassImpl.kt:16-16
  const detail::CompilerClassInfo* type_info() const override { return type_info_; }

  // Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/native/internal/KClassImpl.kt:23-24
  std::optional<std::u16string> simple_name() const override {
    return TypeInfoNames(*type_info_).simple_name();
  }

  // Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/native/internal/KClassImpl.kt:26-27
  std::optional<std::u16string> qualified_name() const override {
    return TypeInfoNames(*type_info_).qualified_name();
  }

  // Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/native/internal/KClassImpl.kt:29-29
  bool is_instance(const ::kotlin::Any* value) const override {
    return value != nullptr && internal::is_instance(*value, type_info_);
  }

  // Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/native/internal/KClassImpl.kt:31-32
  bool equals(const ::kotlin::Any* other) const override {
    const auto* klass = dynamic_cast<const ::kotlin::reflect::detail::KClassObject*>(other);
    return klass != nullptr && type_info_ == type_info_ptr(*klass);
  }

  // Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/native/internal/KClassImpl.kt:34-34
  std::int32_t hash_code() const override {
    return detail::hash_type_info_pointer(type_info_);
  }

  // Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/native/internal/KClassImpl.kt:36-36
  std::u16string to_string() const override {
    return u"class " + full_name(*this).value_or(u"<anonymous>");
  }

 private:
  const detail::CompilerClassInfo* const type_info_;
};
}  // namespace kotlin::native::internal
