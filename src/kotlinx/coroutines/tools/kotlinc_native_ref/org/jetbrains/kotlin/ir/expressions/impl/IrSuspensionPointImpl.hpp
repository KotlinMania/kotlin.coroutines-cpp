/*
 * Copyright 2010-2024 JetBrains s.r.o. and Kotlin Programming Language contributors.
 * Use of this source code is governed by the Apache 2.0 license that can be found in the license/LICENSE.txt file.
 */
// port-lint: source compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/expressions/impl/IrSuspensionPointImpl.kt
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/expressions/impl/IrSuspensionPointImpl.kt:20-30
#pragma once
#include "../IrSuspensionPoint.hpp"

namespace org::jetbrains::kotlin::ir::util { class IrElementConstructorIndicator; }

namespace org::jetbrains::kotlin::ir::expressions::impl {
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/expressions/impl/IrSuspensionPointImpl.kt:20-30
class IrSuspensionPointImpl final : public IrSuspensionPoint {
 public:
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/expressions/impl/IrSuspensionPointImpl.kt:20-30
  IrSuspensionPointImpl(util::IrElementConstructorIndicator* constructor_indicator,
      std::int32_t start_offset,
      std::int32_t end_offset,
      types::IrType& type,
      declarations::IrVariable& suspension_point_id_parameter,
      IrExpression& result,
      IrExpression& resume_result);
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/expressions/impl/IrSuspensionPointImpl.kt:22-22
  std::int32_t start_offset() const override;
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/expressions/impl/IrSuspensionPointImpl.kt:22-22
  void set_start_offset(std::int32_t value) override;
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/expressions/impl/IrSuspensionPointImpl.kt:23-23
  std::int32_t end_offset() const override;
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/expressions/impl/IrSuspensionPointImpl.kt:23-23
  void set_end_offset(std::int32_t value) override;
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/expressions/impl/IrSuspensionPointImpl.kt:24-24
  types::IrType& type() const override;
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/expressions/impl/IrSuspensionPointImpl.kt:24-24
  void set_type(types::IrType& value) override;
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/expressions/impl/IrSuspensionPointImpl.kt:25-25
  declarations::IrVariable& suspension_point_id_parameter() const override;
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/expressions/impl/IrSuspensionPointImpl.kt:25-25
  void set_suspension_point_id_parameter(declarations::IrVariable& value) override;
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/expressions/impl/IrSuspensionPointImpl.kt:26-26
  IrExpression& result() const override;
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/expressions/impl/IrSuspensionPointImpl.kt:26-26
  void set_result(IrExpression& value) override;
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/expressions/impl/IrSuspensionPointImpl.kt:27-27
  IrExpression& resume_result() const override;
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/expressions/impl/IrSuspensionPointImpl.kt:27-27
  void set_resume_result(IrExpression& value) override;
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/expressions/impl/IrSuspensionPointImpl.kt:29-29
  IrElement& attribute_owner_id() const override;
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/expressions/impl/IrSuspensionPointImpl.kt:29-29
  void set_attribute_owner_id(IrElement& value) override;
 private:
  std::int32_t start_offset_;
  std::int32_t end_offset_;
  types::IrType* type_;
  declarations::IrVariable* suspension_point_id_parameter_;
  IrExpression* result_;
  IrExpression* resume_result_;
  IrElement* attribute_owner_id_;
};
}  // namespace org::jetbrains::kotlin::ir::expressions::impl
