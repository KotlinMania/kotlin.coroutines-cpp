/*
 * Copyright 2010-2024 JetBrains s.r.o. and Kotlin Programming Language contributors.
 * Use of this source code is governed by the Apache 2.0 license that can be found in the license/LICENSE.txt file.
 */
// port-lint: source compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/symbols/IrSymbol.kt
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/symbols/IrSymbol.kt:154-159
// port-lint: source compiler/ir/ir.tree/src/org/jetbrains/kotlin/ir/symbols/IrSymbol.kt
#pragma once
#include "IrValueSymbol.hpp"
#include "../declarations/IrValueParameter.hpp"
#include "../../descriptors/ParameterDescriptor.hpp"
namespace org::jetbrains::kotlin::ir::symbols {
/**
 * A symbol whose [owner] is [IrValueParameter].
 *
 * Generated from: [org.jetbrains.kotlin.ir.generator.IrSymbolTree.valueParameterSymbol]
 */
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/symbols/IrSymbol.kt:159-159
class IrValueParameterSymbol : public virtual IrValueSymbol, public virtual IrBindableSymbol<::org::jetbrains::kotlin::descriptors::ParameterDescriptor, declarations::IrValueParameter>, public virtual mpp::ValueParameterSymbolMarker {
 public:
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/symbols/IrSymbol.kt:159-159
// Transliterated from: compiler/ir/ir.tree/src/org/jetbrains/kotlin/ir/symbols/IrSymbol.kt:126-127
  declarations::IrValueParameter& owner() const;
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/symbols/IrSymbol.kt:159-159
// Transliterated from: compiler/ir/ir.tree/src/org/jetbrains/kotlin/ir/symbols/IrSymbol.kt:129-130
  ::org::jetbrains::kotlin::descriptors::ParameterDescriptor& descriptor() const;
};
}  // namespace org::jetbrains::kotlin::ir::symbols
