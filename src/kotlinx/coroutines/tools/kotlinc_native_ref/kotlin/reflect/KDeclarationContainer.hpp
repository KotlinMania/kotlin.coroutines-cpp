/*
 * Copyright 2010-2018 JetBrains s.r.o. Use of this source code is governed by the Apache 2.0 license
 * that can be found in the LICENSE file.
 */
// port-lint: source kotlin-native/runtime/src/main/kotlin/kotlin/reflect/KDeclarationContainer.kt
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/reflect/KDeclarationContainer.kt:12-12
#pragma once

namespace kotlin::reflect {
/**
 * Represents an entity which may contain declarations of any other entities,
 * such as a class or a package.
 */
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/reflect/KDeclarationContainer.kt:12-12
class [[clang::annotate("kotlin.class:kotlin.reflect:KDeclarationContainer:interface")]] KDeclarationContainer {
 public:
  virtual ~KDeclarationContainer() = default;
 protected:
  // NOTE(port): The genuine Native interface is methodless. Keep its interface
  // constructor protected; do not synthesize a declaration-members collection.
  KDeclarationContainer() = default;
  KDeclarationContainer(const KDeclarationContainer&) = delete;
  KDeclarationContainer& operator=(const KDeclarationContainer&) = delete;
};
}  // namespace kotlin::reflect
