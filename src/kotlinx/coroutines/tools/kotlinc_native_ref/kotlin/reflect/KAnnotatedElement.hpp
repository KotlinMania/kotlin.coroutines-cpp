/*
 * Copyright 2010-2018 JetBrains s.r.o. Use of this source code is governed by the Apache 2.0 license
 * that can be found in the LICENSE file.
 */
// port-lint: source kotlin-native/runtime/src/main/kotlin/kotlin/reflect/KAnnotatedElement.kt
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/reflect/KAnnotatedElement.kt:13-13
#pragma once
namespace kotlin::reflect {
/**
 * Represents an annotated element and allows to obtain its annotations.
 * See the [Kotlin language documentation](https://kotlinlang.org/docs/reference/annotations.html)
 * for more information.
 */
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/reflect/KAnnotatedElement.kt:13-13
class [[clang::annotate("kotlin.class:kotlin.reflect:KAnnotatedElement:interface")]] KAnnotatedElement {
 public:
  virtual ~KAnnotatedElement() = default;
 protected:
  // NOTE(port): The actual Native interface has no methods. Keep this an
  // interface base; do not invent the JVM annotations property here.
  KAnnotatedElement() = default;
  KAnnotatedElement(const KAnnotatedElement&) = delete;
  KAnnotatedElement& operator=(const KAnnotatedElement&) = delete;
};
}  // namespace kotlin::reflect
