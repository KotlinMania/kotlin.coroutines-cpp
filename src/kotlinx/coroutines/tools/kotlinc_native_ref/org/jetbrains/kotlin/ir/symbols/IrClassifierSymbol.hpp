/*
 * Copyright 2010-2024 JetBrains s.r.o. and Kotlin Programming Language contributors.
 * Use of this source code is governed by the Apache 2.0 license that can be found in the license/LICENSE.txt file.
 */
// port-lint: source compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/symbols/IrSymbol.kt
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/symbols/IrSymbol.kt:83-94
// port-lint: source compiler/ir/ir.tree/src/org/jetbrains/kotlin/ir/symbols/IrSymbol.kt
#pragma once
#include "IrSymbol.hpp"
#include "../../descriptors/ClassifierDescriptor.hpp"
#include "../../types/model/TypeSystemContext.hpp"
namespace org::jetbrains::kotlin::ir::symbols {
/**
 * A symbol whose [owner] is [IrClass], [IrScript] or [IrTypeParameter].
 *
 * Generated from: [org.jetbrains.kotlin.ir.generator.IrSymbolTree.classifierSymbol]
 *
 * @see IrClassReference.symbol
 * @see IrSimpleType.classifier
 */
// NOTE(port): Source sealed/opt-in metadata remains open; no substitute owner
// or descriptor state is introduced by the typed root-dispatch accessors.
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/symbols/IrSymbol.kt:91-94
class IrClassifierSymbol : public virtual IrSymbol, public virtual ::org::jetbrains::kotlin::types::model::TypeConstructorMarker {
 public:
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/symbols/IrSymbol.kt:91-94
  ::org::jetbrains::kotlin::descriptors::ClassifierDescriptor& descriptor() const;
};
}  // namespace org::jetbrains::kotlin::ir::symbols
