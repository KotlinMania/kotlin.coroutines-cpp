/*
 * Copyright 2010-2024 JetBrains s.r.o. and Kotlin Programming Language contributors.
 * Use of this source code is governed by the Apache 2.0 license that can be found in the license/LICENSE.txt file.
 */
// port-lint: source compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/expressions/IrSuspensionPoint.kt
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/expressions/IrSuspensionPoint.kt:18-39
#include "IrSuspensionPoint.hpp"
#include "../declarations/IrVariable.hpp"

namespace org::jetbrains::kotlin::ir::expressions {
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/expressions/IrSuspensionPoint.kt:25-26
void IrSuspensionPoint::accept_dispatch(visitors::detail::IrVisitorDispatch& dispatch) {
  dispatch.visit_suspension_point(*this);
}

// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/expressions/IrSuspensionPoint.kt:28-32
void IrSuspensionPoint::accept_children_dispatch(visitors::detail::IrVisitorDispatch& dispatch) {
  dispatch.accept(suspension_point_id_parameter());
  dispatch.accept(result());
  dispatch.accept(resume_result());
}

// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/expressions/IrSuspensionPoint.kt:34-38
void IrSuspensionPoint::transform_children_dispatch(visitors::detail::IrTransformerDispatch& dispatch) {
  set_suspension_point_id_parameter(dynamic_cast<declarations::IrVariable&>(dispatch.transform(suspension_point_id_parameter())));
  set_result(dynamic_cast<IrExpression&>(dispatch.transform(result())));
  set_resume_result(dynamic_cast<IrExpression&>(dispatch.transform(resume_result())));
}
}  // namespace org::jetbrains::kotlin::ir::expressions
