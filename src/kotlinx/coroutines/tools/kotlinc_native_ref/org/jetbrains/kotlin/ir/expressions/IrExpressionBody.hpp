/*
 * Copyright 2010-2024 JetBrains s.r.o. and Kotlin Programming Language contributors.
 * Use of this source code is governed by the Apache 2.0 license that can be found in the license/LICENSE.txt file.
 */
// port-lint: source compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/expressions/IrExpressionBody.kt
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/expressions/IrExpressionBody.kt:17-33
#pragma once
#include "IrBody.hpp"
#include "IrExpression.hpp"
namespace org::jetbrains::kotlin::ir::expressions {
/**
 * Generated from: [org.jetbrains.kotlin.ir.generator.IrTree.expressionBody]
 */
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/expressions/IrExpressionBody.kt:17-33
class IrExpressionBody : public IrBody {
 public:
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/expressions/IrExpressionBody.kt:18-18
  virtual IrExpression& expression() const = 0;
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/expressions/IrExpressionBody.kt:18-18
  virtual void set_expression(IrExpression& value) = 0;
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/expressions/IrExpressionBody.kt:23-24
  template <typename D>
  IrExpressionBody& transform(visitors::IrTransformer<D>& transformer, D data) {
    return dynamic_cast<IrExpressionBody&>(IrElement::transform(transformer, data));
  }
 protected:
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/expressions/IrExpressionBody.kt:23-24
  void transform_dispatch(visitors::detail::IrTransformerDispatch& dispatch) override;
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/expressions/IrExpressionBody.kt:20-21
  void accept_dispatch(visitors::detail::IrVisitorDispatch& dispatch) override;
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/expressions/IrExpressionBody.kt:26-28
  void accept_children_dispatch(visitors::detail::IrVisitorDispatch& dispatch) override;
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/expressions/IrExpressionBody.kt:30-32
  void transform_children_dispatch(visitors::detail::IrTransformerDispatch& dispatch) override;
};
}  // namespace org::jetbrains::kotlin::ir::expressions
