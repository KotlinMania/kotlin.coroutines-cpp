/*
 * Copyright 2010-2024 JetBrains s.r.o. and Kotlin Programming Language contributors.
 * Use of this source code is governed by the Apache 2.0 license that can be found in the license/LICENSE.txt file.
 */
// port-lint: source compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/IrSymbolOwner.kt
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/IrSymbolOwner.kt:17-19
#pragma once
#include "../IrElement.hpp"
#include "../symbols/IrSymbol.hpp"

namespace org::jetbrains::kotlin::ir::declarations {
/**
 * Generated from: [org.jetbrains.kotlin.ir.generator.IrTree.symbolOwner]
 */
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/IrSymbolOwner.kt:17-19
class IrSymbolOwner : public virtual IrElement {
 public:
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/IrSymbolOwner.kt:18-18
  symbols::IrSymbol& symbol() const;

 protected:
  // NOTE(port): C++ preserves the recursive typed symbol/owner properties
  // through one abstract virtual boundary on the actual root symbol.
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/IrSymbolOwner.kt:18-18
  virtual symbols::IrSymbol& symbol_dispatch() const = 0;
};
}  // namespace org::jetbrains::kotlin::ir::declarations
