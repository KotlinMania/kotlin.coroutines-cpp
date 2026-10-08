/*
 * Copyright 2010-2024 JetBrains s.r.o. and Kotlin Programming Language contributors.
 * Use of this source code is governed by the Apache 2.0 license that can be found in the license/LICENSE.txt file.
 */
// port-lint: source kotlin-native/runtime/src/main/kotlin/kotlin/Any.kt
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/Any.kt:31-41
#include "Any.hpp"
#include "native/Runtime.hpp"

namespace kotlin {
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/Any.kt:31-31
bool Any::equals(const Any* other) const {
  return this == other;
}

// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/Any.kt:41-41
std::int32_t Any::hash_code() const {
  return native::identity_hash_code(this);
}
}  // namespace kotlin
