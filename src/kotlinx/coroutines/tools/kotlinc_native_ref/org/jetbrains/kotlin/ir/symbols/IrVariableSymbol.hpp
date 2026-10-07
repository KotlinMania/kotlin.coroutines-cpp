/*
 * Copyright 2010-2024 JetBrains s.r.o. and Kotlin Programming Language contributors.
 * Use of this source code is governed by the Apache 2.0 license that can be found in the license/LICENSE.txt file.
 */
// port-lint: source compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/symbols/IrSymbol.kt
// port-lint: source compiler/ir/ir.tree/src/org/jetbrains/kotlin/ir/symbols/IrSymbol.kt
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/symbols/IrSymbol.kt:161-168
#pragma once

#include "IrValueSymbol.hpp"
#include "../declarations/IrVariable.hpp"
#include "../../descriptors/VariableDescriptor.hpp"

namespace org::jetbrains::kotlin::ir::symbols {
/**
 * A symbol whose [owner] is [IrVariable].
 *
 * Generated from: [org.jetbrains.kotlin.ir.generator.IrSymbolTree.variableSymbol]
 *
 * @see IrLocalDelegatedPropertyReference.delegate
 */
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/symbols/IrSymbol.kt:168-168
class IrVariableSymbol
    : public virtual IrValueSymbol,
      public virtual IrBindableSymbol<::org::jetbrains::kotlin::descriptors::VariableDescriptor,
                                      declarations::IrVariable> {
 public:
  // NOTE(port): Resolve the two inherited typed getters through the sole root
  // dispatch. The genuine generic owner/descriptor bounds remain enforced.
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/symbols/IrSymbol.kt:168-168
  // Transliterated from: compiler/ir/ir.tree/src/org/jetbrains/kotlin/ir/symbols/IrSymbol.kt:126-127
  declarations::IrVariable& owner() const;
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/symbols/IrSymbol.kt:168-168
  // Transliterated from: compiler/ir/ir.tree/src/org/jetbrains/kotlin/ir/symbols/IrSymbol.kt:129-130
  ::org::jetbrains::kotlin::descriptors::VariableDescriptor& descriptor() const;
};
}  // namespace org::jetbrains::kotlin::ir::symbols
