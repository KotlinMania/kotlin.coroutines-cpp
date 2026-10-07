/*
 * Copyright 2010-2023 JetBrains s.r.o. Use of this source code is governed by the Apache 2.0 license
 * that can be found in the LICENSE file.
 */
// port-lint: source kotlin-native/runtime/src/main/kotlin/kotlin/native/internal/KClassImpl.kt
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/native/internal/KClassImpl.kt:42-43
#include "KClassImpl.hpp"

namespace kotlin::native::internal {
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/native/internal/KClassImpl.kt:42-43
std::optional<std::u16string> full_name(const ::kotlin::reflect::detail::KClassObject& klass) {
  const auto* type_info = type_info_ptr(klass);
  return type_info != nullptr ? TypeInfoNames(*type_info).full_name() : std::nullopt;
}
}  // namespace kotlin::native::internal
