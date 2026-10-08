/*
 * Copyright 2010-2024 JetBrains s.r.o. and Kotlin Programming Language contributors.
 * Use of this source code is governed by the Apache 2.0 license that can be found in the license/LICENSE.txt file.
 */
// port-lint: source compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/symbols/IrSymbol.kt
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/symbols/IrSymbol.kt:131-136
// port-lint: source compiler/ir/ir.tree/src/org/jetbrains/kotlin/ir/symbols/IrSymbol.kt
#pragma once
#include "IrClassifierSymbol.hpp"
#include "../declarations/IrTypeParameter.hpp"
#include "../../descriptors/TypeParameterDescriptor.hpp"
namespace org::jetbrains::kotlin::ir::symbols {
/**
 * A symbol whose [owner] is [IrTypeParameter].
 *
 * Generated from: [org.jetbrains.kotlin.ir.generator.IrSymbolTree.typeParameterSymbol]
 */
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/symbols/IrSymbol.kt:136-136
class IrTypeParameterSymbol : public virtual IrClassifierSymbol, public virtual IrBindableSymbol<::org::jetbrains::kotlin::descriptors::TypeParameterDescriptor, declarations::IrTypeParameter>, public virtual ::org::jetbrains::kotlin::types::model::TypeParameterMarker, public virtual mpp::TypeParameterSymbolMarker {
 public:
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/symbols/IrSymbol.kt:136-136
// Transliterated from: compiler/ir/ir.tree/src/org/jetbrains/kotlin/ir/symbols/IrSymbol.kt:126-127
  declarations::IrTypeParameter& owner() const;
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/symbols/IrSymbol.kt:136-136
// Transliterated from: compiler/ir/ir.tree/src/org/jetbrains/kotlin/ir/symbols/IrSymbol.kt:129-130
  ::org::jetbrains::kotlin::descriptors::TypeParameterDescriptor& descriptor() const;
};
}  // namespace org::jetbrains::kotlin::ir::symbols
