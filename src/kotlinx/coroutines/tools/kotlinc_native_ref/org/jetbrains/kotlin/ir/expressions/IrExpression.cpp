/*
 * Copyright 2010-2024 JetBrains s.r.o. and Kotlin Programming Language contributors.
 * Use of this source code is governed by the Apache 2.0 license that can be found in the license/LICENSE.txt file.
 */
// port-lint: source compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/expressions/IrExpression.kt
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/expressions/IrExpression.kt:22-23
#include "IrExpression.hpp"

namespace org::jetbrains::kotlin::ir::expressions {
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/expressions/IrExpression.kt:22-23
void IrExpression::transform_dispatch(visitors::detail::IrTransformerDispatch& dispatch) {
  dispatch.accept(*this);
  (void)dynamic_cast<IrExpression&>(dispatch.take_element_result());
}
}  // namespace org::jetbrains::kotlin::ir::expressions
