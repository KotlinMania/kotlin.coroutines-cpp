/*
 * Copyright 2010-2024 JetBrains s.r.o. and Kotlin Programming Language contributors.
 * Use of this source code is governed by the Apache 2.0 license that can be found in the license/LICENSE.txt file.
 */
// port-lint: source compiler/ir/ir.tree/src/org/jetbrains/kotlin/ir/declarations/IrParameterKind.kt
// Transliterated from: compiler/ir/ir.tree/src/org/jetbrains/kotlin/ir/declarations/IrParameterKind.kt:8-13
#pragma once

namespace org::jetbrains::kotlin::ir::declarations {

// Transliterated from: compiler/ir/ir.tree/src/org/jetbrains/kotlin/ir/declarations/IrParameterKind.kt:8-13
enum class IrParameterKind {
  DISPATCH_RECEIVER,
  CONTEXT,
  EXTENSION_RECEIVER,
  REGULAR,
};

}  // namespace org::jetbrains::kotlin::ir::declarations
