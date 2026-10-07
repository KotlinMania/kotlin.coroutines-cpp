/*
 * Copyright 2010-2024 JetBrains s.r.o. and Kotlin Programming Language contributors.
 * Use of this source code is governed by the Apache 2.0 license that can be found in the license/LICENSE.txt file.
 */
// port-lint: source compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/IrValueDeclaration.kt
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/IrValueDeclaration.kt:19-26
#include "IrValueDeclaration.hpp"
#include "../symbols/IrValueSymbol.hpp"

namespace org::jetbrains::kotlin::ir::declarations {

// NOTE(port): Narrow the one actual symbol result after both source interfaces
// are complete. No object copy or alternate symbol is introduced.
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/IrValueDeclaration.kt:23-23
symbols::IrValueSymbol& IrValueDeclaration::symbol() const {
  return dynamic_cast<symbols::IrValueSymbol&>(symbol_dispatch());
}

}  // namespace org::jetbrains::kotlin::ir::declarations
