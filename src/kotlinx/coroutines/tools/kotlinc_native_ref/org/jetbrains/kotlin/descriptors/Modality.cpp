/*
 * Copyright 2010-2020 JetBrains s.r.o. and Kotlin Programming Language contributors.
 * Use of this source code is governed by the Apache 2.0 license that can be found in the license/LICENSE.txt file.
 */
// port-lint: source core/compiler.common/src/org/jetbrains/kotlin/descriptors/Modality.kt
// Transliterated from: core/compiler.common/src/org/jetbrains/kotlin/descriptors/Modality.kt:8-27
#include "Modality.hpp"

namespace org::jetbrains::kotlin::descriptors {
// Transliterated from: core/compiler.common/src/org/jetbrains/kotlin/descriptors/Modality.kt:19-26
Modality convert_from_flags(bool sealed, bool abstract, bool open) {
  if (sealed) return Modality::SEALED;
  if (abstract) return Modality::ABSTRACT;
  if (open) return Modality::OPEN;
  return Modality::FINAL;
}
}  // namespace org::jetbrains::kotlin::descriptors
