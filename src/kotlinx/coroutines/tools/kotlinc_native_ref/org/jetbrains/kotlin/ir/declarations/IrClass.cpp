/*
 * Copyright 2010-2024 JetBrains s.r.o. and Kotlin Programming Language contributors.
 * Use of this source code is governed by the Apache 2.0 license that can be found in the license/LICENSE.txt file.
 */
// port-lint: source compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/IrClass.kt
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/IrClass.kt:24-85
#include "IrClass.hpp"
#include "../symbols/IrClassSymbol.hpp"
namespace org::jetbrains::kotlin::ir::declarations {
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/IrClass.kt:28-28
symbols::IrClassSymbol& IrClass::symbol() const { return dynamic_cast<symbols::IrClassSymbol&>(symbol_dispatch()); }
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/IrClass.kt:71-72
void IrClass::accept_dispatch(visitors::detail::IrVisitorDispatch& dispatch) { dispatch.visit_class(*this); }
}  // namespace org::jetbrains::kotlin::ir::declarations
