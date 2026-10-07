/*
 * Copyright 2010-2024 JetBrains s.r.o. and Kotlin Programming Language contributors.
 * Use of this source code is governed by the Apache 2.0 license that can be found in the license/LICENSE.txt file.
 */
// port-lint: source compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/impl/IrVariableImpl.kt
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/impl/IrVariableImpl.kt:26-54
#include "IrVariableImpl.hpp"
#include "../../symbols/IrVariableSymbol.hpp"
#include "../../../../../../kotlin/collections/Collections.hpp"

#include <stdexcept>

namespace org::jetbrains::kotlin::ir::declarations::impl {
// NOTE(port): The source constructor indicator is deliberately unused.
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/impl/IrVariableImpl.kt:26-40
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/impl/IrVariableImpl.kt:49-53
IrVariableImpl::IrVariableImpl(util::IrElementConstructorIndicator*,
                               std::int32_t start_offset, std::int32_t end_offset,
                               IrDeclarationOrigin& origin,
                               const ::org::jetbrains::kotlin::name::Name& name,
                               types::IrType& type, symbols::IrVariableSymbol& symbol,
                               bool is_var, bool is_const, bool is_lateinit)
    : start_offset_(start_offset), end_offset_(end_offset), origin_(&origin),
      name_(name), type_(&type), symbol_(&symbol), is_var_(is_var),
      is_const_(is_const), is_lateinit_(is_lateinit), attribute_owner_id_(this),
      annotations_(&::kotlin::collections::empty_list<expressions::IrAnnotation*>()) {
  symbol.bind(*this);
}

// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/impl/IrVariableImpl.kt:28-28
std::int32_t IrVariableImpl::start_offset() const { return start_offset_; }
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/impl/IrVariableImpl.kt:28-28
void IrVariableImpl::set_start_offset(std::int32_t value) { start_offset_ = value; }

// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/impl/IrVariableImpl.kt:29-29
std::int32_t IrVariableImpl::end_offset() const { return end_offset_; }
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/impl/IrVariableImpl.kt:29-29
void IrVariableImpl::set_end_offset(std::int32_t value) { end_offset_ = value; }

// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/impl/IrVariableImpl.kt:30-30
IrDeclarationOrigin& IrVariableImpl::origin() const { return *origin_; }
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/impl/IrVariableImpl.kt:30-30
void IrVariableImpl::set_origin(IrDeclarationOrigin& value) { origin_ = &value; }

// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/impl/IrVariableImpl.kt:31-31
const ::org::jetbrains::kotlin::name::Name& IrVariableImpl::name() const { return name_; }
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/impl/IrVariableImpl.kt:31-31
void IrVariableImpl::set_name(const ::org::jetbrains::kotlin::name::Name& value) { name_ = value; }

// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/impl/IrVariableImpl.kt:32-32
types::IrType& IrVariableImpl::type() const { return *type_; }
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/impl/IrVariableImpl.kt:32-32
void IrVariableImpl::set_type(types::IrType& value) { type_ = &value; }

// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/impl/IrVariableImpl.kt:34-34
bool IrVariableImpl::is_var() const { return is_var_; }
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/impl/IrVariableImpl.kt:34-34
void IrVariableImpl::set_is_var(bool value) { is_var_ = value; }

// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/impl/IrVariableImpl.kt:35-35
bool IrVariableImpl::is_const() const { return is_const_; }
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/impl/IrVariableImpl.kt:35-35
void IrVariableImpl::set_is_const(bool value) { is_const_ = value; }

// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/impl/IrVariableImpl.kt:36-36
bool IrVariableImpl::is_lateinit() const { return is_lateinit_; }
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/impl/IrVariableImpl.kt:36-36
void IrVariableImpl::set_is_lateinit(bool value) { is_lateinit_ = value; }

// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/impl/IrVariableImpl.kt:38-38
IrElement& IrVariableImpl::attribute_owner_id() const { return *attribute_owner_id_; }
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/impl/IrVariableImpl.kt:38-38
void IrVariableImpl::set_attribute_owner_id(IrElement& value) { attribute_owner_id_ = &value; }

// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/impl/IrVariableImpl.kt:40-40
::kotlin::collections::List<expressions::IrAnnotation*>& IrVariableImpl::annotations() const { return *annotations_; }
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/impl/IrVariableImpl.kt:40-40
void IrVariableImpl::set_annotations(::kotlin::collections::List<expressions::IrAnnotation*>& value) { annotations_ = &value; }

// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/impl/IrVariableImpl.kt:49-49
expressions::IrExpression* IrVariableImpl::initializer() const { return initializer_; }
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/impl/IrVariableImpl.kt:49-49
void IrVariableImpl::set_initializer(expressions::IrExpression* value) { initializer_ = value; }

// NOTE(port): This failure is the actual Kotlin factory getter behavior.
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/impl/IrVariableImpl.kt:42-43
IrFactory& IrVariableImpl::factory() const {
  throw std::logic_error("Create IrVariableImpl directly");
}

// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/impl/IrVariableImpl.kt:45-47
::org::jetbrains::kotlin::descriptors::VariableDescriptor& IrVariableImpl::descriptor() const {
  return symbol_->descriptor();
}

// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/impl/IrVariableImpl.kt:33-33
symbols::IrSymbol& IrVariableImpl::symbol_dispatch() const { return *symbol_; }
}  // namespace org::jetbrains::kotlin::ir::declarations::impl
