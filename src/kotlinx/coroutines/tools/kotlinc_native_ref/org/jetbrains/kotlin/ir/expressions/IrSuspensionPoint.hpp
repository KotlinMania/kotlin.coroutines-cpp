/*
 * Copyright 2010-2024 JetBrains s.r.o. and Kotlin Programming Language contributors.
 * Use of this source code is governed by the Apache 2.0 license that can be found in the license/LICENSE.txt file.
 */
// port-lint: source compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/expressions/IrSuspensionPoint.kt
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/expressions/IrSuspensionPoint.kt:18-39
#pragma once
#include "IrExpression.hpp"

namespace org::jetbrains::kotlin::ir::declarations { class IrVariable; }

namespace org::jetbrains::kotlin::ir::expressions {
/**
 * Generated from: [org.jetbrains.kotlin.ir.generator.IrTree.suspensionPoint]
 */
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/expressions/IrSuspensionPoint.kt:18-39
class IrSuspensionPoint : public IrExpression {
 public:
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/expressions/IrSuspensionPoint.kt:19-19
  virtual declarations::IrVariable& suspension_point_id_parameter() const = 0;
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/expressions/IrSuspensionPoint.kt:19-19
  virtual void set_suspension_point_id_parameter(declarations::IrVariable& value) = 0;
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/expressions/IrSuspensionPoint.kt:21-21
  virtual IrExpression& result() const = 0;
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/expressions/IrSuspensionPoint.kt:21-21
  virtual void set_result(IrExpression& value) = 0;
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/expressions/IrSuspensionPoint.kt:23-23
  virtual IrExpression& resume_result() const = 0;
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/expressions/IrSuspensionPoint.kt:23-23
  virtual void set_resume_result(IrExpression& value) = 0;
 protected:
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/expressions/IrSuspensionPoint.kt:25-26
  void accept_dispatch(visitors::detail::IrVisitorDispatch& dispatch) override;
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/expressions/IrSuspensionPoint.kt:28-32
  void accept_children_dispatch(visitors::detail::IrVisitorDispatch& dispatch) override;
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/expressions/IrSuspensionPoint.kt:34-38
  void transform_children_dispatch(visitors::detail::IrTransformerDispatch& dispatch) override;
};
}  // namespace org::jetbrains::kotlin::ir::expressions
