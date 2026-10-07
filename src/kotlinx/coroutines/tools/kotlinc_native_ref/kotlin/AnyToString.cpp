/*
 * Copyright 2010-2024 JetBrains s.r.o. and Kotlin Programming Language contributors.
 * Use of this source code is governed by the Apache 2.0 license that can be found in the license/LICENSE.txt file.
 */
// port-lint: source kotlin-native/runtime/src/main/kotlin/kotlin/Any.kt
// port-lint: source kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/lower/PostInlineLowering.kt
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/Any.kt:46-52
#include "Any.hpp"
#include "native/internal/KClassImpl.hpp"
#include "text/StringNumberConversions.hpp"

namespace kotlin {
// NOTE(port): Source this::class uses the actual Native visitGetClass construction.
// Its real object/class metadata binding must be supplied by that dependency;
// compiler-owned C++ objects cannot be cast to Native ObjHeader to implement it.
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/Any.kt:46-52
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/lower/PostInlineLowering.kt:81-93
std::u16string Any::to_string() const {
  const auto klass = native::internal::KClassImpl<Any>(
      native::internal::get_object_type_info(*this));
  const auto class_name = native::internal::full_name(klass).value_or(u"<object>");
  // Upstream question: consider using [identityHashCode].
  const auto unsigned_hash_code = static_cast<std::int64_t>(hash_code()) & 0xffffffffLL;
  const auto hash_code_string = text::to_string(unsigned_hash_code, 16);
  return class_name + u"@" + hash_code_string;
}
}  // namespace kotlin
