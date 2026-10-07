/*
 * Copyright 2010-2024 JetBrains s.r.o. and Kotlin Programming Language contributors.
 * Use of this source code is governed by the Apache 2.0 license that can be found in the license/LICENSE.txt file.
 */
// port-lint: source compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/symbols/IrSymbol.kt
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/symbols/IrSymbol.kt:96-112
// port-lint: source compiler/ir/ir.tree/src/org/jetbrains/kotlin/ir/symbols/IrSymbol.kt
#pragma once
#include "IrClassifierSymbol.hpp"
#include "../declarations/IrClass.hpp"
#include "../../descriptors/ClassDescriptor.hpp"
namespace org::jetbrains::kotlin::ir::symbols {
/**
 * A symbol whose [owner] is [IrClass].
 *
 * Generated from: [org.jetbrains.kotlin.ir.generator.IrSymbolTree.classSymbol]
 *
 * @see IrClass.sealedSubclasses
 * @see IrFunctionWithLateBinding.companionExtensionClass
 * @see IrScript.targetClass
 * @see IrReplSnippet.stateObject
 * @see IrReplSnippet.targetClass
 * @see IrSimpleFunction.companionExtensionClass
 * @see IrAnnotation.classSymbol
 * @see IrGetObjectValue.symbol
 * @see IrCall.superQualifierSymbol
 * @see IrInstanceInitializerCall.classSymbol
 */
// NOTE(port): Source sealed/opt-in metadata remains open; no substitute owner
// or descriptor state is introduced by the typed root-dispatch accessors.
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/symbols/IrSymbol.kt:112-112
class IrClassSymbol : public virtual IrClassifierSymbol, public virtual IrBindableSymbol<::org::jetbrains::kotlin::descriptors::ClassDescriptor, declarations::IrClass>, public virtual mpp::RegularClassSymbolMarker {
 public:
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/symbols/IrSymbol.kt:112-112
// Transliterated from: compiler/ir/ir.tree/src/org/jetbrains/kotlin/ir/symbols/IrSymbol.kt:126-127
  declarations::IrClass& owner() const;
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/symbols/IrSymbol.kt:112-112
  ::org::jetbrains::kotlin::descriptors::ClassDescriptor& descriptor() const;
};
}  // namespace org::jetbrains::kotlin::ir::symbols
