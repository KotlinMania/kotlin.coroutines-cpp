/*
 * Copyright 2010-2024 JetBrains s.r.o. and Kotlin Programming Language contributors.
 * Use of this source code is governed by the Apache 2.0 license that can be found in the license/LICENSE.txt file.
 */
// port-lint: source compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/expressions/IrExpression.kt
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/expressions/IrExpression.kt:16-24
#pragma once
#include "../IrElementBase.hpp"
#include "../IrStatement.hpp"
#include "IrVarargElement.hpp"
#include "../types/IrType.hpp"

namespace org::jetbrains::kotlin::ir::expressions {
/**
 * Generated from: [org.jetbrains.kotlin.ir.generator.IrTree.expression]
 */
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/expressions/IrExpression.kt:19-24
class IrExpression : public IrElementBase, public virtual IrStatement, public virtual IrVarargElement {
 public:
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/expressions/IrExpression.kt:20-20
  virtual types::IrType& type() const = 0;
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/expressions/IrExpression.kt:20-20
  virtual void set_type(types::IrType& value) = 0;

  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/expressions/IrExpression.kt:22-23
  template <typename D>
  IrExpression& transform(visitors::IrTransformer<D>& transformer, D data) {
    return dynamic_cast<IrExpression&>(IrElement::transform(transformer, data));
  }

 protected:
  // NOTE(port): Enforce the source checked expression result through the one
  // virtual boundary as well as the typed public template, including base calls.
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/expressions/IrExpression.kt:22-23
  void transform_dispatch(visitors::detail::IrTransformerDispatch& dispatch) override;
};
}  // namespace org::jetbrains::kotlin::ir::expressions
