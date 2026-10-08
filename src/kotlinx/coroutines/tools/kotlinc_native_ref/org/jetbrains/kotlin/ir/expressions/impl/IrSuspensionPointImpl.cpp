/*
 * Copyright 2010-2024 JetBrains s.r.o. and Kotlin Programming Language contributors.
 * Use of this source code is governed by the Apache 2.0 license that can be found in the license/LICENSE.txt file.
 */
// port-lint: source compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/expressions/impl/IrSuspensionPointImpl.kt
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/expressions/impl/IrSuspensionPointImpl.kt:20-30
#include "IrSuspensionPointImpl.hpp"

namespace org::jetbrains::kotlin::ir::expressions::impl {
// NOTE(port): The unused source constructor indicator is retained in the API.
// Borrowed fields preserve actual compiler-owned node/type/symbol identities.
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/expressions/impl/IrSuspensionPointImpl.kt:20-30
IrSuspensionPointImpl::IrSuspensionPointImpl(util::IrElementConstructorIndicator*,
      std::int32_t start_offset,
      std::int32_t end_offset,
      types::IrType& type,
      declarations::IrVariable& suspension_point_id_parameter,
      IrExpression& result,
      IrExpression& resume_result)
    : start_offset_(start_offset), end_offset_(end_offset), type_(&type), suspension_point_id_parameter_(&suspension_point_id_parameter), result_(&result), resume_result_(&resume_result),
      attribute_owner_id_(this) {}

// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/expressions/impl/IrSuspensionPointImpl.kt:22-22
std::int32_t IrSuspensionPointImpl::start_offset() const { return start_offset_; }
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/expressions/impl/IrSuspensionPointImpl.kt:22-22
void IrSuspensionPointImpl::set_start_offset(std::int32_t value) { start_offset_ = value; }

// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/expressions/impl/IrSuspensionPointImpl.kt:23-23
std::int32_t IrSuspensionPointImpl::end_offset() const { return end_offset_; }
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/expressions/impl/IrSuspensionPointImpl.kt:23-23
void IrSuspensionPointImpl::set_end_offset(std::int32_t value) { end_offset_ = value; }

// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/expressions/impl/IrSuspensionPointImpl.kt:24-24
types::IrType& IrSuspensionPointImpl::type() const { return *type_; }
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/expressions/impl/IrSuspensionPointImpl.kt:24-24
void IrSuspensionPointImpl::set_type(types::IrType& value) { type_ = &value; }

// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/expressions/impl/IrSuspensionPointImpl.kt:25-25
declarations::IrVariable& IrSuspensionPointImpl::suspension_point_id_parameter() const { return *suspension_point_id_parameter_; }
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/expressions/impl/IrSuspensionPointImpl.kt:25-25
void IrSuspensionPointImpl::set_suspension_point_id_parameter(declarations::IrVariable& value) { suspension_point_id_parameter_ = &value; }

// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/expressions/impl/IrSuspensionPointImpl.kt:26-26
IrExpression& IrSuspensionPointImpl::result() const { return *result_; }
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/expressions/impl/IrSuspensionPointImpl.kt:26-26
void IrSuspensionPointImpl::set_result(IrExpression& value) { result_ = &value; }

// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/expressions/impl/IrSuspensionPointImpl.kt:27-27
IrExpression& IrSuspensionPointImpl::resume_result() const { return *resume_result_; }
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/expressions/impl/IrSuspensionPointImpl.kt:27-27
void IrSuspensionPointImpl::set_resume_result(IrExpression& value) { resume_result_ = &value; }

// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/expressions/impl/IrSuspensionPointImpl.kt:29-29
IrElement& IrSuspensionPointImpl::attribute_owner_id() const { return *attribute_owner_id_; }
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/expressions/impl/IrSuspensionPointImpl.kt:29-29
void IrSuspensionPointImpl::set_attribute_owner_id(IrElement& value) { attribute_owner_id_ = &value; }
}  // namespace org::jetbrains::kotlin::ir::expressions::impl
