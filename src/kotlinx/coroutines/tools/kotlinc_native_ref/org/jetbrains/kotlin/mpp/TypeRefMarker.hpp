/*
 * Copyright 2010-2023 JetBrains s.r.o. and Kotlin Programming Language contributors.
 * Use of this source code is governed by the Apache 2.0 license that can be found in the license/LICENSE.txt file.
 */
// port-lint: source core/compiler.common/src/org/jetbrains/kotlin/mpp/TypeRefMarker.kt
// Transliterated from: core/compiler.common/src/org/jetbrains/kotlin/mpp/TypeRefMarker.kt:10-17
#pragma once

namespace org::jetbrains::kotlin::mpp {
/**
 * Common interface for type references to be used in abstract checker.
 * The idea is similar to [org.jetbrains.kotlin.types.model.KotlinTypeMarker],
 * but type reference, unlike a type, has source element.
 *
 * Used in [org.jetbrains.kotlin.resolve.calls.mpp.AbstractExpectActualAnnotationMatchChecker].
 */
// Transliterated from: core/compiler.common/src/org/jetbrains/kotlin/mpp/TypeRefMarker.kt:17-17
class TypeRefMarker {
 public:
  virtual ~TypeRefMarker() = default;
 protected:
  TypeRefMarker() = default;
};
}  // namespace org::jetbrains::kotlin::mpp
