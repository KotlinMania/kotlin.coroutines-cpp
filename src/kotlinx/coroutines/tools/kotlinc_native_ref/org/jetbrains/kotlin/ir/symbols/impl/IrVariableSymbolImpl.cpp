/*
 * Copyright 2010-2024 JetBrains s.r.o. and Kotlin Programming Language contributors.
 * Use of this source code is governed by the Apache 2.0 license that can be found in the license/LICENSE.txt file.
 */
// port-lint: source compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/symbols/impl/IrSymbolImpl.kt
// port-lint: source compiler/ir/ir.tree/src/org/jetbrains/kotlin/ir/symbols/IrSymbol.kt
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/symbols/impl/IrSymbolImpl.kt:71-73
#include "IrVariableSymbolImpl.hpp"

namespace org::jetbrains::kotlin::ir::symbols::impl {
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/symbols/impl/IrSymbolImpl.kt:71-73
IrVariableSymbolImpl::IrVariableSymbolImpl(::org::jetbrains::kotlin::descriptors::VariableDescriptor* descriptor)
    : IrSymbolBase<::org::jetbrains::kotlin::descriptors::VariableDescriptor,
                   declarations::IrVariable>(descriptor) {}

// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/symbols/impl/IrSymbolImpl.kt:71-73
// Transliterated from: compiler/ir/ir.tree/src/org/jetbrains/kotlin/ir/symbols/IrSymbol.kt:126-127
declarations::IrVariable& IrVariableSymbolImpl::owner() const {
  return IrVariableSymbol::owner();
}

// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/symbols/impl/IrSymbolImpl.kt:71-73
// Transliterated from: compiler/ir/ir.tree/src/org/jetbrains/kotlin/ir/symbols/IrSymbol.kt:129-130
::org::jetbrains::kotlin::descriptors::VariableDescriptor& IrVariableSymbolImpl::descriptor() const {
  return IrVariableSymbol::descriptor();
}
}  // namespace org::jetbrains::kotlin::ir::symbols::impl
