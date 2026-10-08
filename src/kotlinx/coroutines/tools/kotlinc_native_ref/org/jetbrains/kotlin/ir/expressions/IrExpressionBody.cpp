/*
 * Copyright 2010-2024 JetBrains s.r.o. and Kotlin Programming Language contributors.
 * Use of this source code is governed by the Apache 2.0 license that can be found in the license/LICENSE.txt file.
 */
// port-lint: source compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/expressions/IrExpressionBody.kt
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/expressions/IrExpressionBody.kt:17-33
#include "IrExpressionBody.hpp"
namespace org::jetbrains::kotlin::ir::expressions {
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/expressions/IrExpressionBody.kt:23-24
void IrExpressionBody::transform_dispatch(visitors::detail::IrTransformerDispatch& dispatch) {
  dispatch.accept(*this);
  (void)dynamic_cast<IrExpressionBody&>(dispatch.take_element_result());
}
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/expressions/IrExpressionBody.kt:20-21
void IrExpressionBody::accept_dispatch(visitors::detail::IrVisitorDispatch& dispatch) { dispatch.visit_expression_body(*this); }
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/expressions/IrExpressionBody.kt:26-28
void IrExpressionBody::accept_children_dispatch(visitors::detail::IrVisitorDispatch& dispatch) { dispatch.accept(expression()); }
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/expressions/IrExpressionBody.kt:30-32
void IrExpressionBody::transform_children_dispatch(visitors::detail::IrTransformerDispatch& dispatch) {
  set_expression(dynamic_cast<IrExpression&>(dispatch.transform(expression())));
}
}  // namespace org::jetbrains::kotlin::ir::expressions
