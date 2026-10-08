/*
 * Copyright 2010-2024 JetBrains s.r.o. and Kotlin Programming Language contributors.
 * Use of this source code is governed by the Apache 2.0 license that can be found in the license/LICENSE.txt file.
 */
// port-lint: source compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/expressions/impl/IrSuspendableExpressionImpl.kt
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/expressions/impl/IrSuspendableExpressionImpl.kt:19-28
#include "IrSuspendableExpressionImpl.hpp"

namespace org::jetbrains::kotlin::ir::expressions::impl {
// NOTE(port): The unused source constructor indicator is retained in the API.
// Borrowed fields preserve actual compiler-owned node/type/symbol identities.
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/expressions/impl/IrSuspendableExpressionImpl.kt:19-28
IrSuspendableExpressionImpl::IrSuspendableExpressionImpl(util::IrElementConstructorIndicator*,
      std::int32_t start_offset,
      std::int32_t end_offset,
      types::IrType& type,
      IrExpression& suspension_point_id,
      IrExpression& result)
    : start_offset_(start_offset), end_offset_(end_offset), type_(&type), suspension_point_id_(&suspension_point_id), result_(&result),
      attribute_owner_id_(this) {}

// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/expressions/impl/IrSuspendableExpressionImpl.kt:21-21
std::int32_t IrSuspendableExpressionImpl::start_offset() const { return start_offset_; }
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/expressions/impl/IrSuspendableExpressionImpl.kt:21-21
void IrSuspendableExpressionImpl::set_start_offset(std::int32_t value) { start_offset_ = value; }

// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/expressions/impl/IrSuspendableExpressionImpl.kt:22-22
std::int32_t IrSuspendableExpressionImpl::end_offset() const { return end_offset_; }
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/expressions/impl/IrSuspendableExpressionImpl.kt:22-22
void IrSuspendableExpressionImpl::set_end_offset(std::int32_t value) { end_offset_ = value; }

// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/expressions/impl/IrSuspendableExpressionImpl.kt:23-23
types::IrType& IrSuspendableExpressionImpl::type() const { return *type_; }
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/expressions/impl/IrSuspendableExpressionImpl.kt:23-23
void IrSuspendableExpressionImpl::set_type(types::IrType& value) { type_ = &value; }

// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/expressions/impl/IrSuspendableExpressionImpl.kt:24-24
IrExpression& IrSuspendableExpressionImpl::suspension_point_id() const { return *suspension_point_id_; }
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/expressions/impl/IrSuspendableExpressionImpl.kt:24-24
void IrSuspendableExpressionImpl::set_suspension_point_id(IrExpression& value) { suspension_point_id_ = &value; }

// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/expressions/impl/IrSuspendableExpressionImpl.kt:25-25
IrExpression& IrSuspendableExpressionImpl::result() const { return *result_; }
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/expressions/impl/IrSuspendableExpressionImpl.kt:25-25
void IrSuspendableExpressionImpl::set_result(IrExpression& value) { result_ = &value; }

// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/expressions/impl/IrSuspendableExpressionImpl.kt:27-27
IrElement& IrSuspendableExpressionImpl::attribute_owner_id() const { return *attribute_owner_id_; }
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/expressions/impl/IrSuspendableExpressionImpl.kt:27-27
void IrSuspendableExpressionImpl::set_attribute_owner_id(IrElement& value) { attribute_owner_id_ = &value; }
}  // namespace org::jetbrains::kotlin::ir::expressions::impl
