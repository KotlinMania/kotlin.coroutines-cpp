/*
 * Copyright 2010-2019 JetBrains s.r.o. and Kotlin Programming Language contributors.
 * Use of this source code is governed by the Apache 2.0 license that can be found in the license/LICENSE.txt file.
 */
// port-lint: source libraries/stdlib/src/kotlin/reflect/KClassifier.kt
// Transliterated from: libraries/stdlib/src/kotlin/reflect/KClassifier.kt:17-17
#pragma once

namespace kotlin::reflect {
/**
 * A classifier is either a class or a type parameter.
 *
 * @see [KClass]
 * @see [KTypeParameter]
 */
// Transliterated from: libraries/stdlib/src/kotlin/reflect/KClassifier.kt:17-17
class [[clang::annotate("kotlin.class:kotlin.reflect:KClassifier:interface")]] KClassifier {
 public:
  virtual ~KClassifier() = default;
 protected:
  // NOTE(port): The shared source supplies a genuine methodless interface;
  // its JvmBuiltin declaration annotation is not an application JVM dependency.
  KClassifier() = default;
  KClassifier(const KClassifier&) = delete;
  KClassifier& operator=(const KClassifier&) = delete;
};
}  // namespace kotlin::reflect
