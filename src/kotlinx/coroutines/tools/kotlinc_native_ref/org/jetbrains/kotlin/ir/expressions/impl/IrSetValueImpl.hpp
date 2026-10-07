/*
 * Copyright 2010-2024 JetBrains s.r.o. and Kotlin Programming Language contributors.
 * Use of this source code is governed by the Apache 2.0 license that can be found in the license/LICENSE.txt file.
 */
// port-lint: source compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/expressions/impl/IrSetValueImpl.kt
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/expressions/impl/IrSetValueImpl.kt:21-31
#pragma once
#include "../IrSetValue.hpp"

namespace org::jetbrains::kotlin::ir::util { class IrElementConstructorIndicator; }

namespace org::jetbrains::kotlin::ir::expressions::impl {
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/expressions/impl/IrSetValueImpl.kt:21-31
class IrSetValueImpl final : public IrSetValue {
 public:
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/expressions/impl/IrSetValueImpl.kt:21-31
  IrSetValueImpl(util::IrElementConstructorIndicator* constructor_indicator,
      std::int32_t start_offset,
      std::int32_t end_offset,
      types::IrType& type,
      symbols::IrValueSymbol& symbol,
      IrStatementOrigin* origin,
      IrExpression& value);
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/expressions/impl/IrSetValueImpl.kt:23-23
  std::int32_t start_offset() const override;
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/expressions/impl/IrSetValueImpl.kt:23-23
  void set_start_offset(std::int32_t value) override;
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/expressions/impl/IrSetValueImpl.kt:24-24
  std::int32_t end_offset() const override;
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/expressions/impl/IrSetValueImpl.kt:24-24
  void set_end_offset(std::int32_t value) override;
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/expressions/impl/IrSetValueImpl.kt:25-25
  types::IrType& type() const override;
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/expressions/impl/IrSetValueImpl.kt:25-25
  void set_type(types::IrType& value) override;
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/expressions/impl/IrSetValueImpl.kt:26-26
  symbols::IrValueSymbol& symbol() const override;
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/expressions/impl/IrSetValueImpl.kt:26-26
  void set_symbol(symbols::IrValueSymbol& value) override;
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/expressions/impl/IrSetValueImpl.kt:27-27
  IrStatementOrigin* origin() const override;
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/expressions/impl/IrSetValueImpl.kt:27-27
  void set_origin(IrStatementOrigin* value) override;
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/expressions/impl/IrSetValueImpl.kt:28-28
  IrExpression& value() const override;
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/expressions/impl/IrSetValueImpl.kt:28-28
  void set_value(IrExpression& value) override;
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/expressions/impl/IrSetValueImpl.kt:30-30
  IrElement& attribute_owner_id() const override;
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/expressions/impl/IrSetValueImpl.kt:30-30
  void set_attribute_owner_id(IrElement& value) override;
 private:
  std::int32_t start_offset_;
  std::int32_t end_offset_;
  types::IrType* type_;
  symbols::IrValueSymbol* symbol_;
  IrStatementOrigin* origin_;
  IrExpression* value_;
  IrElement* attribute_owner_id_;
};
}  // namespace org::jetbrains::kotlin::ir::expressions::impl
