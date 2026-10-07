/*
 * Copyright 2010-2024 JetBrains s.r.o. and Kotlin Programming Language contributors.
 * Use of this source code is governed by the Apache 2.0 license that can be found in the license/LICENSE.txt file.
 */
// port-lint: source compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/expressions/impl/IrSetValueImpl.kt
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/expressions/impl/IrSetValueImpl.kt:21-31
#include "IrSetValueImpl.hpp"

namespace org::jetbrains::kotlin::ir::expressions::impl {
// NOTE(port): The unused source constructor indicator is retained in the API.
// Borrowed fields preserve actual compiler-owned node/type/symbol identities.
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/expressions/impl/IrSetValueImpl.kt:21-31
IrSetValueImpl::IrSetValueImpl(util::IrElementConstructorIndicator*,
      std::int32_t start_offset,
      std::int32_t end_offset,
      types::IrType& type,
      symbols::IrValueSymbol& symbol,
      IrStatementOrigin* origin,
      IrExpression& value)
    : start_offset_(start_offset), end_offset_(end_offset), type_(&type), symbol_(&symbol), origin_(origin), value_(&value),
      attribute_owner_id_(this) {}

// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/expressions/impl/IrSetValueImpl.kt:23-23
std::int32_t IrSetValueImpl::start_offset() const { return start_offset_; }
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/expressions/impl/IrSetValueImpl.kt:23-23
void IrSetValueImpl::set_start_offset(std::int32_t value) { start_offset_ = value; }

// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/expressions/impl/IrSetValueImpl.kt:24-24
std::int32_t IrSetValueImpl::end_offset() const { return end_offset_; }
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/expressions/impl/IrSetValueImpl.kt:24-24
void IrSetValueImpl::set_end_offset(std::int32_t value) { end_offset_ = value; }

// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/expressions/impl/IrSetValueImpl.kt:25-25
types::IrType& IrSetValueImpl::type() const { return *type_; }
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/expressions/impl/IrSetValueImpl.kt:25-25
void IrSetValueImpl::set_type(types::IrType& value) { type_ = &value; }

// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/expressions/impl/IrSetValueImpl.kt:26-26
symbols::IrValueSymbol& IrSetValueImpl::symbol() const { return *symbol_; }
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/expressions/impl/IrSetValueImpl.kt:26-26
void IrSetValueImpl::set_symbol(symbols::IrValueSymbol& value) { symbol_ = &value; }

// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/expressions/impl/IrSetValueImpl.kt:27-27
IrStatementOrigin* IrSetValueImpl::origin() const { return origin_; }
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/expressions/impl/IrSetValueImpl.kt:27-27
void IrSetValueImpl::set_origin(IrStatementOrigin* value) { origin_ = value; }

// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/expressions/impl/IrSetValueImpl.kt:28-28
IrExpression& IrSetValueImpl::value() const { return *value_; }
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/expressions/impl/IrSetValueImpl.kt:28-28
void IrSetValueImpl::set_value(IrExpression& value) { value_ = &value; }

// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/expressions/impl/IrSetValueImpl.kt:30-30
IrElement& IrSetValueImpl::attribute_owner_id() const { return *attribute_owner_id_; }
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/expressions/impl/IrSetValueImpl.kt:30-30
void IrSetValueImpl::set_attribute_owner_id(IrElement& value) { attribute_owner_id_ = &value; }
}  // namespace org::jetbrains::kotlin::ir::expressions::impl
