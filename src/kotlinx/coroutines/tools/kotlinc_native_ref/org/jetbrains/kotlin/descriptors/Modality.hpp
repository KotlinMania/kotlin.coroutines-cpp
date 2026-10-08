/*
 * Copyright 2010-2020 JetBrains s.r.o. and Kotlin Programming Language contributors.
 * Use of this source code is governed by the Apache 2.0 license that can be found in the license/LICENSE.txt file.
 */
// port-lint: source core/compiler.common/src/org/jetbrains/kotlin/descriptors/Modality.kt
// Transliterated from: core/compiler.common/src/org/jetbrains/kotlin/descriptors/Modality.kt:8-27
#pragma once

namespace org::jetbrains::kotlin::descriptors {
// For sealed classes, isOverridable is false but isOverridableByMembers is true
// Transliterated from: core/compiler.common/src/org/jetbrains/kotlin/descriptors/Modality.kt:9-16
enum class Modality {
  // THE ORDER OF ENTRIES MATTERS HERE
  FINAL,
  // NB: class can be sealed but not function or property
  SEALED,
  OPEN,
  ABSTRACT
};
// Transliterated from: core/compiler.common/src/org/jetbrains/kotlin/descriptors/Modality.kt:19-26
Modality convert_from_flags(bool sealed, bool abstract, bool open);
}  // namespace org::jetbrains::kotlin::descriptors
