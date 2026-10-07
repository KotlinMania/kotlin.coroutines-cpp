/*
 * Copyright 2010-2024 JetBrains s.r.o. and Kotlin Programming Language contributors.
 * Use of this source code is governed by the Apache 2.0 license that can be found in the license/LICENSE.txt file.
 */
// port-lint: source compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/expressions/IrSuspendableExpression.kt
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/expressions/IrSuspendableExpression.kt:17-34
#include "IrSuspendableExpression.hpp"

namespace org::jetbrains::kotlin::ir::expressions {
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/expressions/IrSuspendableExpression.kt:22-23
void IrSuspendableExpression::accept_dispatch(visitors::detail::IrVisitorDispatch& dispatch) {
  dispatch.visit_suspendable_expression(*this);
}

// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/expressions/IrSuspendableExpression.kt:25-28
void IrSuspendableExpression::accept_children_dispatch(visitors::detail::IrVisitorDispatch& dispatch) {
  dispatch.accept(suspension_point_id());
  dispatch.accept(result());
}

// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/expressions/IrSuspendableExpression.kt:30-33
void IrSuspendableExpression::transform_children_dispatch(visitors::detail::IrTransformerDispatch& dispatch) {
  set_suspension_point_id(dynamic_cast<IrExpression&>(dispatch.transform(suspension_point_id())));
  set_result(dynamic_cast<IrExpression&>(dispatch.transform(result())));
}
}  // namespace org::jetbrains::kotlin::ir::expressions
