/*
 * Copyright 2010-2023 JetBrains s.r.o. Use of this source code is governed by the Apache 2.0 license
 * that can be found in the LICENSE file.
 */
// port-lint: source kotlin-native/runtime/src/main/kotlin/kotlin/native/internal/KClassImpl.kt
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/native/internal/KClassImpl.kt:39-40
#include "KClassImpl.hpp"

namespace kotlin::native::internal {
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/native/internal/KClassImpl.kt:39-40
const detail::CompilerClassInfo* type_info_ptr(const ::kotlin::reflect::detail::KClassObject& klass) {
  const auto* holder = dynamic_cast<const TypeInfoHolder*>(&klass);
  return holder != nullptr ? holder->type_info() : nullptr;
}
}  // namespace kotlin::native::internal
