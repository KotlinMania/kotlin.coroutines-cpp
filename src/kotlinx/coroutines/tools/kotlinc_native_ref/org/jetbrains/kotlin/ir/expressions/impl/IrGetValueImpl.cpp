/*
 * Copyright 2010-2024 JetBrains s.r.o. and Kotlin Programming Language contributors.
 * Use of this source code is governed by the Apache 2.0 license that can be found in the license/LICENSE.txt file.
 */
// port-lint: source compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/expressions/impl/IrGetValueImpl.kt
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/expressions/impl/IrGetValueImpl.kt:20-29
#include "IrGetValueImpl.hpp"

namespace org::jetbrains::kotlin::ir::expressions::impl {
// NOTE(port): The unused source constructor indicator is retained in the API.
// Borrowed fields preserve actual compiler-owned node/type/symbol identities.
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/expressions/impl/IrGetValueImpl.kt:20-29
IrGetValueImpl::IrGetValueImpl(util::IrElementConstructorIndicator*,
      std::int32_t start_offset,
      std::int32_t end_offset,
      types::IrType& type,
      symbols::IrValueSymbol& symbol,
      IrStatementOrigin* origin)
    : start_offset_(start_offset), end_offset_(end_offset), type_(&type), symbol_(&symbol), origin_(origin),
      attribute_owner_id_(this) {}

// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/expressions/impl/IrGetValueImpl.kt:22-22
std::int32_t IrGetValueImpl::start_offset() const { return start_offset_; }
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/expressions/impl/IrGetValueImpl.kt:22-22
void IrGetValueImpl::set_start_offset(std::int32_t value) { start_offset_ = value; }

// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/expressions/impl/IrGetValueImpl.kt:23-23
std::int32_t IrGetValueImpl::end_offset() const { return end_offset_; }
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/expressions/impl/IrGetValueImpl.kt:23-23
void IrGetValueImpl::set_end_offset(std::int32_t value) { end_offset_ = value; }

// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/expressions/impl/IrGetValueImpl.kt:24-24
types::IrType& IrGetValueImpl::type() const { return *type_; }
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/expressions/impl/IrGetValueImpl.kt:24-24
void IrGetValueImpl::set_type(types::IrType& value) { type_ = &value; }

// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/expressions/impl/IrGetValueImpl.kt:25-25
symbols::IrValueSymbol& IrGetValueImpl::symbol() const { return *symbol_; }
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/expressions/impl/IrGetValueImpl.kt:25-25
void IrGetValueImpl::set_symbol(symbols::IrValueSymbol& value) { symbol_ = &value; }

// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/expressions/impl/IrGetValueImpl.kt:26-26
IrStatementOrigin* IrGetValueImpl::origin() const { return origin_; }
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/expressions/impl/IrGetValueImpl.kt:26-26
void IrGetValueImpl::set_origin(IrStatementOrigin* value) { origin_ = value; }

// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/expressions/impl/IrGetValueImpl.kt:28-28
IrElement& IrGetValueImpl::attribute_owner_id() const { return *attribute_owner_id_; }
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/expressions/impl/IrGetValueImpl.kt:28-28
void IrGetValueImpl::set_attribute_owner_id(IrElement& value) { attribute_owner_id_ = &value; }
}  // namespace org::jetbrains::kotlin::ir::expressions::impl
