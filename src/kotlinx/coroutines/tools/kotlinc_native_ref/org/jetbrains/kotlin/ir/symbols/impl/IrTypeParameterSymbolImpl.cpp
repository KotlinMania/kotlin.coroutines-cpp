/*
 * Copyright 2010-2024 JetBrains s.r.o. and Kotlin Programming Language contributors.
 * Use of this source code is governed by the Apache 2.0 license that can be found in the license/LICENSE.txt file.
 */
// port-lint: source compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/symbols/impl/IrSymbolImpl.kt
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/symbols/impl/IrSymbolImpl.kt:61-64
// port-lint: source compiler/ir/ir.tree/src/org/jetbrains/kotlin/ir/symbols/IrSymbol.kt
#include "IrTypeParameterSymbolImpl.hpp"
namespace org::jetbrains::kotlin::ir::symbols::impl {
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/symbols/impl/IrSymbolImpl.kt:61-64
IrTypeParameterSymbolImpl::IrTypeParameterSymbolImpl(::org::jetbrains::kotlin::descriptors::TypeParameterDescriptor* descriptor, util::IdSignature* signature)
    : IrSymbolWithSignature<::org::jetbrains::kotlin::descriptors::TypeParameterDescriptor, declarations::IrTypeParameter>(descriptor, signature) {}
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/symbols/impl/IrSymbolImpl.kt:61-64
// Transliterated from: compiler/ir/ir.tree/src/org/jetbrains/kotlin/ir/symbols/IrSymbol.kt:126-127
declarations::IrTypeParameter& IrTypeParameterSymbolImpl::owner() const { return IrTypeParameterSymbol::owner(); }
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/symbols/impl/IrSymbolImpl.kt:61-64
// Transliterated from: compiler/ir/ir.tree/src/org/jetbrains/kotlin/ir/symbols/IrSymbol.kt:129-130
::org::jetbrains::kotlin::descriptors::TypeParameterDescriptor& IrTypeParameterSymbolImpl::descriptor() const { return IrTypeParameterSymbol::descriptor(); }
}  // namespace org::jetbrains::kotlin::ir::symbols::impl
