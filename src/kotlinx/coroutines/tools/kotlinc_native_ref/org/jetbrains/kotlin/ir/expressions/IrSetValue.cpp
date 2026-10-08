/*
 * Copyright 2010-2024 JetBrains s.r.o. and Kotlin Programming Language contributors.
 * Use of this source code is governed by the Apache 2.0 license that can be found in the license/LICENSE.txt file.
 */
// port-lint: source compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/expressions/IrSetValue.kt
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/expressions/IrSetValue.kt:17-30
#include "IrSetValue.hpp"

namespace org::jetbrains::kotlin::ir::expressions {
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/expressions/IrSetValue.kt:20-21
void IrSetValue::accept_dispatch(visitors::detail::IrVisitorDispatch& dispatch) {
  dispatch.visit_set_value(*this);
}

// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/expressions/IrSetValue.kt:23-25
void IrSetValue::accept_children_dispatch(visitors::detail::IrVisitorDispatch& dispatch) {
  dispatch.accept(value());
}

// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/expressions/IrSetValue.kt:27-29
void IrSetValue::transform_children_dispatch(visitors::detail::IrTransformerDispatch& dispatch) {
  set_value(dynamic_cast<IrExpression&>(dispatch.transform(value())));
}
}  // namespace org::jetbrains::kotlin::ir::expressions
