/*
 * Copyright 2010-2023 JetBrains s.r.o. and Kotlin Programming Language contributors.
 * Use of this source code is governed by the Apache 2.0 license that can be found in the license/LICENSE.txt file.
 */
// port-lint: source core/compiler.common/src/org/jetbrains/kotlin/mpp/DeclarationSymbolMarkers.kt
// Transliterated from: core/compiler.common/src/org/jetbrains/kotlin/mpp/DeclarationSymbolMarkers.kt:8-28
#pragma once

namespace org::jetbrains::kotlin::mpp {

/*
 * These markers are needed for the common expect/actual compatibility-checking
 * algorithm in resolve.calls.mpp.AbstractExpectActualCompatibilityChecker.
 */
// NOTE(port): These upstream interfaces have no methods. Protected constructors
// keep them as interface bases. Virtual inheritance preserves the single marker
// identity when Kotlin's interface hierarchy has multiple inheritance paths.
// Transliterated from: core/compiler.common/src/org/jetbrains/kotlin/mpp/DeclarationSymbolMarkers.kt:13-13
class DeclarationSymbolMarker {
 public:
  virtual ~DeclarationSymbolMarker() = default;
 protected:
  DeclarationSymbolMarker() = default;
};
// Transliterated from: core/compiler.common/src/org/jetbrains/kotlin/mpp/DeclarationSymbolMarkers.kt:14-14
class CallableSymbolMarker : public virtual DeclarationSymbolMarker {
 protected:
  CallableSymbolMarker() = default;
};
// Transliterated from: core/compiler.common/src/org/jetbrains/kotlin/mpp/DeclarationSymbolMarkers.kt:15-15
class FunctionSymbolMarker : public virtual CallableSymbolMarker {
 protected:
  FunctionSymbolMarker() = default;
};
// Transliterated from: core/compiler.common/src/org/jetbrains/kotlin/mpp/DeclarationSymbolMarkers.kt:16-16
class ConstructorSymbolMarker : public virtual FunctionSymbolMarker {
 protected:
  ConstructorSymbolMarker() = default;
};
// Transliterated from: core/compiler.common/src/org/jetbrains/kotlin/mpp/DeclarationSymbolMarkers.kt:17-17
class SimpleFunctionSymbolMarker : public virtual FunctionSymbolMarker {
 protected:
  SimpleFunctionSymbolMarker() = default;
};
// Transliterated from: core/compiler.common/src/org/jetbrains/kotlin/mpp/DeclarationSymbolMarkers.kt:18-18
class PropertySymbolMarker : public virtual CallableSymbolMarker {
 protected:
  PropertySymbolMarker() = default;
};
// Transliterated from: core/compiler.common/src/org/jetbrains/kotlin/mpp/DeclarationSymbolMarkers.kt:19-19
class ValueParameterSymbolMarker : public virtual CallableSymbolMarker {
 protected:
  ValueParameterSymbolMarker() = default;
};
// Transliterated from: core/compiler.common/src/org/jetbrains/kotlin/mpp/DeclarationSymbolMarkers.kt:20-20
class FieldSymbolMarker : public virtual CallableSymbolMarker {
 protected:
  FieldSymbolMarker() = default;
};
// Transliterated from: core/compiler.common/src/org/jetbrains/kotlin/mpp/DeclarationSymbolMarkers.kt:21-21
class EnumEntrySymbolMarker : public virtual CallableSymbolMarker {
 protected:
  EnumEntrySymbolMarker() = default;
};
// Transliterated from: core/compiler.common/src/org/jetbrains/kotlin/mpp/DeclarationSymbolMarkers.kt:23-23
class ClassifierSymbolMarker : public virtual DeclarationSymbolMarker {
 protected:
  ClassifierSymbolMarker() = default;
};
// Transliterated from: core/compiler.common/src/org/jetbrains/kotlin/mpp/DeclarationSymbolMarkers.kt:24-24
class TypeParameterSymbolMarker : public virtual ClassifierSymbolMarker {
 protected:
  TypeParameterSymbolMarker() = default;
};
// Transliterated from: core/compiler.common/src/org/jetbrains/kotlin/mpp/DeclarationSymbolMarkers.kt:25-25
class ClassLikeSymbolMarker : public virtual ClassifierSymbolMarker {
 protected:
  ClassLikeSymbolMarker() = default;
};
// Transliterated from: core/compiler.common/src/org/jetbrains/kotlin/mpp/DeclarationSymbolMarkers.kt:26-26
class RegularClassSymbolMarker : public virtual ClassLikeSymbolMarker {
 protected:
  RegularClassSymbolMarker() = default;
};
// Transliterated from: core/compiler.common/src/org/jetbrains/kotlin/mpp/DeclarationSymbolMarkers.kt:27-27
class TypeAliasSymbolMarker : public virtual ClassLikeSymbolMarker {
 protected:
  TypeAliasSymbolMarker() = default;
};
// Transliterated from: core/compiler.common/src/org/jetbrains/kotlin/mpp/DeclarationSymbolMarkers.kt:28-28
class K1SyntheticClassifierSymbolMarker : public virtual ClassifierSymbolMarker {
 protected:
  K1SyntheticClassifierSymbolMarker() = default;
};

}  // namespace org::jetbrains::kotlin::mpp
