/*
 * Copyright 2010-2024 JetBrains s.r.o. and Kotlin Programming Language contributors.
 * Use of this source code is governed by the Apache 2.0 license that can be found in the license/LICENSE.txt file.
 */
// port-lint: source compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/IrVariable.kt
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/IrVariable.kt:21-45
#include "IrVariable.hpp"
#include "../symbols/IrVariableSymbol.hpp"
#include "../expressions/IrExpression.hpp"

namespace org::jetbrains::kotlin::ir::declarations {
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/IrVariable.kt:25-25
symbols::IrVariableSymbol& IrVariable::symbol() const {
  return dynamic_cast<symbols::IrVariableSymbol&>(symbol_dispatch());
}

// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/IrVariable.kt:35-36
void IrVariable::accept_dispatch(visitors::detail::IrVisitorDispatch& dispatch) {
  dispatch.visit_variable(*this);
}

// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/IrVariable.kt:38-40
void IrVariable::accept_children_dispatch(visitors::detail::IrVisitorDispatch& dispatch) {
  if (auto* value = initializer()) dispatch.accept(*value);
}

// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/IrVariable.kt:42-44
void IrVariable::transform_children_dispatch(visitors::detail::IrTransformerDispatch& dispatch) {
  auto* value = initializer();
  set_initializer(value == nullptr ? nullptr
                                  : &dynamic_cast<expressions::IrExpression&>(dispatch.transform(*value)));
}
}  // namespace org::jetbrains::kotlin::ir::declarations
