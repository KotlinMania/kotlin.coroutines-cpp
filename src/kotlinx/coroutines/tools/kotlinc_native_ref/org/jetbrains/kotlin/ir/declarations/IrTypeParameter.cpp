/*
 * Copyright 2010-2024 JetBrains s.r.o. and Kotlin Programming Language contributors.
 * Use of this source code is governed by the Apache 2.0 license that can be found in the license/LICENSE.txt file.
 */
// port-lint: source compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/IrTypeParameter.kt
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/IrTypeParameter.kt:22-41
#include "IrTypeParameter.hpp"
#include "../symbols/IrTypeParameterSymbol.hpp"
namespace org::jetbrains::kotlin::ir::declarations {
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/IrTypeParameter.kt:26-26
symbols::IrTypeParameterSymbol& IrTypeParameter::symbol() const { return dynamic_cast<symbols::IrTypeParameterSymbol&>(symbol_dispatch()); }
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/IrTypeParameter.kt:36-37
void IrTypeParameter::accept_dispatch(visitors::detail::IrVisitorDispatch& dispatch) { dispatch.visit_type_parameter(*this); }
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/IrTypeParameter.kt:39-40
void IrTypeParameter::transform_dispatch(visitors::detail::IrTransformerDispatch& dispatch) {
  dispatch.accept(*this);
  (void)dynamic_cast<IrTypeParameter&>(dispatch.take_element_result());
}
}  // namespace org::jetbrains::kotlin::ir::declarations
