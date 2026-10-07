/*
 * Copyright 2010-2024 JetBrains s.r.o. and Kotlin Programming Language contributors.
 * Use of this source code is governed by the Apache 2.0 license that can be found in the license/LICENSE.txt file.
 */
// port-lint: source compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/expressions/impl/IrSuspendableExpressionImpl.kt
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/expressions/impl/IrSuspendableExpressionImpl.kt:19-28
#pragma once
#include "../IrSuspendableExpression.hpp"

namespace org::jetbrains::kotlin::ir::util { class IrElementConstructorIndicator; }

namespace org::jetbrains::kotlin::ir::expressions::impl {
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/expressions/impl/IrSuspendableExpressionImpl.kt:19-28
class IrSuspendableExpressionImpl final : public IrSuspendableExpression {
 public:
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/expressions/impl/IrSuspendableExpressionImpl.kt:19-28
  IrSuspendableExpressionImpl(util::IrElementConstructorIndicator* constructor_indicator,
      std::int32_t start_offset,
      std::int32_t end_offset,
      types::IrType& type,
      IrExpression& suspension_point_id,
      IrExpression& result);
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/expressions/impl/IrSuspendableExpressionImpl.kt:21-21
  std::int32_t start_offset() const override;
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/expressions/impl/IrSuspendableExpressionImpl.kt:21-21
  void set_start_offset(std::int32_t value) override;
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/expressions/impl/IrSuspendableExpressionImpl.kt:22-22
  std::int32_t end_offset() const override;
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/expressions/impl/IrSuspendableExpressionImpl.kt:22-22
  void set_end_offset(std::int32_t value) override;
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/expressions/impl/IrSuspendableExpressionImpl.kt:23-23
  types::IrType& type() const override;
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/expressions/impl/IrSuspendableExpressionImpl.kt:23-23
  void set_type(types::IrType& value) override;
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/expressions/impl/IrSuspendableExpressionImpl.kt:24-24
  IrExpression& suspension_point_id() const override;
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/expressions/impl/IrSuspendableExpressionImpl.kt:24-24
  void set_suspension_point_id(IrExpression& value) override;
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/expressions/impl/IrSuspendableExpressionImpl.kt:25-25
  IrExpression& result() const override;
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/expressions/impl/IrSuspendableExpressionImpl.kt:25-25
  void set_result(IrExpression& value) override;
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/expressions/impl/IrSuspendableExpressionImpl.kt:27-27
  IrElement& attribute_owner_id() const override;
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/expressions/impl/IrSuspendableExpressionImpl.kt:27-27
  void set_attribute_owner_id(IrElement& value) override;
 private:
  std::int32_t start_offset_;
  std::int32_t end_offset_;
  types::IrType* type_;
  IrExpression* suspension_point_id_;
  IrExpression* result_;
  IrElement* attribute_owner_id_;
};
}  // namespace org::jetbrains::kotlin::ir::expressions::impl
