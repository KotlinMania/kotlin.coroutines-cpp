/*
 * Copyright 2010-2024 JetBrains s.r.o. and Kotlin Programming Language contributors.
 * Use of this source code is governed by the Apache 2.0 license that can be found in the license/LICENSE.txt file.
 */
// port-lint: source compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/expressions/IrBody.kt
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/expressions/IrBody.kt:18-21
#include "IrBody.hpp"
namespace org::jetbrains::kotlin::ir::expressions {
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/expressions/IrBody.kt:19-20
void IrBody::transform_dispatch(visitors::detail::IrTransformerDispatch& dispatch) {
  dispatch.accept(*this);
  (void)dynamic_cast<IrBody&>(dispatch.take_element_result());
}
}  // namespace org::jetbrains::kotlin::ir::expressions
