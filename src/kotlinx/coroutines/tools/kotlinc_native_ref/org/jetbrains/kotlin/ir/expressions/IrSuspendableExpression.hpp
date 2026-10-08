/*
 * Copyright 2010-2024 JetBrains s.r.o. and Kotlin Programming Language contributors.
 * Use of this source code is governed by the Apache 2.0 license that can be found in the license/LICENSE.txt file.
 */
// port-lint: source compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/expressions/IrSuspendableExpression.kt
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/expressions/IrSuspendableExpression.kt:17-34
#pragma once
#include "IrExpression.hpp"

namespace org::jetbrains::kotlin::ir::expressions {
/**
 * Generated from: [org.jetbrains.kotlin.ir.generator.IrTree.suspendableExpression]
 */
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/expressions/IrSuspendableExpression.kt:17-34
class IrSuspendableExpression : public IrExpression {
 public:
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/expressions/IrSuspendableExpression.kt:18-18
  virtual IrExpression& suspension_point_id() const = 0;
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/expressions/IrSuspendableExpression.kt:18-18
  virtual void set_suspension_point_id(IrExpression& value) = 0;
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/expressions/IrSuspendableExpression.kt:20-20
  virtual IrExpression& result() const = 0;
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/expressions/IrSuspendableExpression.kt:20-20
  virtual void set_result(IrExpression& value) = 0;
 protected:
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/expressions/IrSuspendableExpression.kt:22-23
  void accept_dispatch(visitors::detail::IrVisitorDispatch& dispatch) override;
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/expressions/IrSuspendableExpression.kt:25-28
  void accept_children_dispatch(visitors::detail::IrVisitorDispatch& dispatch) override;
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/expressions/IrSuspendableExpression.kt:30-33
  void transform_children_dispatch(visitors::detail::IrTransformerDispatch& dispatch) override;
};
}  // namespace org::jetbrains::kotlin::ir::expressions
