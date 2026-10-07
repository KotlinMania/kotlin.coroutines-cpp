/*
 * Copyright 2010-2024 JetBrains s.r.o. and Kotlin Programming Language contributors.
 * Use of this source code is governed by the Apache 2.0 license that can be found in the license/LICENSE.txt file.
 */
// port-lint: source compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/symbols/IrSymbol.kt
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/symbols/IrSymbol.kt:138-152
#pragma once

#include "IrSymbol.hpp"
#include "../../descriptors/ValueDescriptor.hpp"

namespace org::jetbrains::kotlin::ir::declarations { class IrValueDeclaration; }

namespace org::jetbrains::kotlin::ir::symbols {
/**
 * A symbol whose [owner] is either [IrValueParameter] or [IrVariable].
 *
 * Generated from: [org.jetbrains.kotlin.ir.generator.IrSymbolTree.valueSymbol]
 *
 * @see IrGetValue.symbol
 * @see IrSetValue.symbol
 */
// NOTE(port): The source sealed-family and construction/descriptor opt-in
// annotations remain metadata work. The typed owner getter retains the one
// abstract root virtual owner dispatch and does not create binding state.
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/symbols/IrSymbol.kt:146-152
class IrValueSymbol : public virtual IrSymbol {
 public:
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/symbols/IrSymbol.kt:147-148
  ::org::jetbrains::kotlin::descriptors::ValueDescriptor& descriptor() const;
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/symbols/IrSymbol.kt:150-151
  declarations::IrValueDeclaration& owner() const;
};
}  // namespace org::jetbrains::kotlin::ir::symbols
