/*
 * Copyright 2010-2024 JetBrains s.r.o. and Kotlin Programming Language contributors.
 * Use of this source code is governed by the Apache 2.0 license that can be found in the license/LICENSE.txt file.
 */
// port-lint: source kotlin-native/runtime/src/main/kotlin/kotlin/native/internal/TypeInfoNames.kt
// port-lint: source kotlin-native/runtime/src/main/kotlin/kotlin/String.kt
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/native/internal/TypeInfoNames.kt:13-54
#include "TypeInfoNames.hpp"
#include <stdexcept>

namespace kotlin::native::internal {
// NOTE(port): The source opaque pointer is read by its explicitly selected ABI
// transport. Both transports execute these same source getter bodies; the
// actual Native transport lives in a separately linked unit.
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/native/internal/TypeInfoNames.kt:13-16
TypeInfoNames::TypeInfoNames(const void* type_info_ptr, NameGetter package_name,
                             NameGetter relative_name)
    : type_info_ptr_(type_info_ptr), package_name_(package_name),
      relative_name_(relative_name) {
  // NOTE(port): Source require uses IllegalArgumentException at this boundary.
  if (type_info_ptr_ == nullptr) throw std::invalid_argument("Failed requirement.");
}

// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/native/internal/TypeInfoNames.kt:21-25
std::optional<std::u16string> TypeInfoNames::simple_name() const {
  // Consider replacing '$' by another delimeter that can't be used in class name specified with backticks (``).
  auto relative_name = relative_name_(type_info_ptr_, true);
  if (!relative_name) return std::nullopt;
  auto delimiter = relative_name->find_last_of(u'.');
  if (delimiter != std::u16string::npos) *relative_name = relative_name->substr(delimiter + 1);
  delimiter = relative_name->find_last_of(u'$');
  if (delimiter != std::u16string::npos) *relative_name = relative_name->substr(delimiter + 1);
  return relative_name;
}

// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/native/internal/TypeInfoNames.kt:30-35
std::optional<std::u16string> TypeInfoNames::qualified_name() const {
  auto package_name = package_name_(type_info_ptr_, true);
  if (!package_name) return std::nullopt;
  auto relative_name = relative_name_(type_info_ptr_, true);
  if (!relative_name) return std::nullopt;
  return package_name->empty() ? *relative_name : *package_name + u"." + *relative_name;
}

// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/native/internal/TypeInfoNames.kt:40-45
std::optional<std::u16string> TypeInfoNames::full_name() const {
  auto relative_name = relative_name_(type_info_ptr_, false);
  if (!relative_name) return std::nullopt;
  auto package_name = package_name_(type_info_ptr_, false);
  return !package_name || package_name->empty()
             ? *relative_name : *package_name + u"." + *relative_name;
}
}  // namespace kotlin::native::internal
