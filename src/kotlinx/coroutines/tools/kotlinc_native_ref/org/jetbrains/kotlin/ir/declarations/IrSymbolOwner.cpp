/*
 * Copyright 2010-2024 JetBrains s.r.o. and Kotlin Programming Language contributors.
 * Use of this source code is governed by the Apache 2.0 license that can be found in the license/LICENSE.txt file.
 */
// port-lint: source compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/IrSymbolOwner.kt
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/IrSymbolOwner.kt:17-19
#include "IrSymbolOwner.hpp"

namespace org::jetbrains::kotlin::ir::declarations {

// NOTE(port): Public getter retains the source abstract virtual property dispatch.
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/IrSymbolOwner.kt:18-18
symbols::IrSymbol& IrSymbolOwner::symbol() const {
  return symbol_dispatch();
}

}  // namespace org::jetbrains::kotlin::ir::declarations
