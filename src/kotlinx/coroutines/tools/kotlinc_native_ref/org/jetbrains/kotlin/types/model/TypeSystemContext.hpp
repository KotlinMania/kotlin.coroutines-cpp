/*
 * Copyright 2010-2019 JetBrains s.r.o. and Kotlin Programming Language contributors.
 * Use of this source code is governed by the Apache 2.0 license that can be found in the license/LICENSE.txt file.
 */
// port-lint: source core/compiler.common/src/org/jetbrains/kotlin/types/model/TypeSystemContext.kt
// Transliterated from: core/compiler.common/src/org/jetbrains/kotlin/types/model/TypeSystemContext.kt:19-34
#pragma once

namespace org::jetbrains::kotlin::types::model {
// NOTE(port): These nine consumed source interfaces have no methods. Virtual
// inheritance preserves one Kotlin marker identity across the type diamonds.
// Transliterated from: core/compiler.common/src/org/jetbrains/kotlin/types/model/TypeSystemContext.kt:19-19
class KotlinTypeMarker {
 public:
  virtual ~KotlinTypeMarker() = default;
 protected:
  KotlinTypeMarker() = default;
};
// Transliterated from: core/compiler.common/src/org/jetbrains/kotlin/types/model/TypeSystemContext.kt:20-20
class TypeArgumentMarker {
 public:
  virtual ~TypeArgumentMarker() = default;
 protected:
  TypeArgumentMarker() = default;
};
// Transliterated from: core/compiler.common/src/org/jetbrains/kotlin/types/model/TypeSystemContext.kt:21-21
class TypeConstructorMarker {
 public:
  virtual ~TypeConstructorMarker() = default;
 protected:
  TypeConstructorMarker() = default;
};
// Transliterated from: core/compiler.common/src/org/jetbrains/kotlin/types/model/TypeSystemContext.kt:22-22
class TypeParameterMarker {
 public:
  virtual ~TypeParameterMarker() = default;
 protected:
  TypeParameterMarker() = default;
};
// Transliterated from: core/compiler.common/src/org/jetbrains/kotlin/types/model/TypeSystemContext.kt:24-24
class RigidTypeMarker : public virtual KotlinTypeMarker {
 public:
  virtual ~RigidTypeMarker() = default;
 protected:
  RigidTypeMarker() = default;
};
// Transliterated from: core/compiler.common/src/org/jetbrains/kotlin/types/model/TypeSystemContext.kt:25-25
class FlexibleTypeMarker : public virtual KotlinTypeMarker {
 public:
  virtual ~FlexibleTypeMarker() = default;
 protected:
  FlexibleTypeMarker() = default;
};
// Transliterated from: core/compiler.common/src/org/jetbrains/kotlin/types/model/TypeSystemContext.kt:26-26
class DynamicTypeMarker : public virtual FlexibleTypeMarker {
 public:
  virtual ~DynamicTypeMarker() = default;
 protected:
  DynamicTypeMarker() = default;
};
// Transliterated from: core/compiler.common/src/org/jetbrains/kotlin/types/model/TypeSystemContext.kt:29-29
class SimpleTypeMarker : public virtual RigidTypeMarker {
 public:
  virtual ~SimpleTypeMarker() = default;
 protected:
  SimpleTypeMarker() = default;
};
// Transliterated from: core/compiler.common/src/org/jetbrains/kotlin/types/model/TypeSystemContext.kt:34-34
class TypeArgumentListMarker {
 public:
  virtual ~TypeArgumentListMarker() = default;
 protected:
  TypeArgumentListMarker() = default;
};
}  // namespace org::jetbrains::kotlin::types::model
