/*
 * Copyright 2010-2024 JetBrains s.r.o. and Kotlin Programming Language contributors.
 * Use of this source code is governed by the Apache 2.0 license that can be found in the license/LICENSE.txt file.
 */
// port-lint: source compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/impl/IrValueParameterImpl.kt
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/impl/IrValueParameterImpl.kt:27-55
#include "IrValueParameterImpl.hpp"
#include "../../symbols/IrValueParameterSymbol.hpp"
#include "../../../../../../kotlin/collections/Collections.hpp"
namespace org::jetbrains::kotlin::ir::declarations::impl {
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/impl/IrValueParameterImpl.kt:27-41
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/impl/IrValueParameterImpl.kt:42-44
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/impl/IrValueParameterImpl.kt:50-54
IrValueParameterImpl::IrValueParameterImpl(std::int32_t start_offset,
      std::int32_t end_offset,
      IrDeclarationOrigin& origin,
      IrFactory& factory,
      const ::org::jetbrains::kotlin::name::Name& name,
      types::IrType& type,
      IrParameterKind kind,
      bool is_assignable,
      symbols::IrValueParameterSymbol& symbol,
      types::IrType* vararg_element_type,
      bool is_crossinline,
      bool is_noinline,
      bool is_hidden)
    : start_offset_(start_offset),
      end_offset_(end_offset),
      origin_(&origin),
      factory_(&factory),
      name_(name),
      type_(&type),
      kind_(kind),
      is_assignable_(is_assignable),
      symbol_(&symbol),
      vararg_element_type_(vararg_element_type),
      is_crossinline_(is_crossinline),
      is_noinline_(is_noinline),
      is_hidden_(is_hidden),
      attribute_owner_id_(this),
      annotations_(&::kotlin::collections::empty_list<expressions::IrAnnotation*>()),
      default_value_(nullptr) {
  symbol.bind(*this);
}
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/impl/IrValueParameterImpl.kt:28-28
std::int32_t IrValueParameterImpl::start_offset() const { return start_offset_; }
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/impl/IrValueParameterImpl.kt:28-28
void IrValueParameterImpl::set_start_offset(std::int32_t value) { start_offset_ = value; }
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/impl/IrValueParameterImpl.kt:29-29
std::int32_t IrValueParameterImpl::end_offset() const { return end_offset_; }
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/impl/IrValueParameterImpl.kt:29-29
void IrValueParameterImpl::set_end_offset(std::int32_t value) { end_offset_ = value; }
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/impl/IrValueParameterImpl.kt:30-30
IrDeclarationOrigin& IrValueParameterImpl::origin() const { return *origin_; }
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/impl/IrValueParameterImpl.kt:30-30
void IrValueParameterImpl::set_origin(IrDeclarationOrigin& value) { origin_ = &value; }
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/impl/IrValueParameterImpl.kt:31-31
IrFactory& IrValueParameterImpl::factory() const { return *factory_; }
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/impl/IrValueParameterImpl.kt:32-32
const ::org::jetbrains::kotlin::name::Name& IrValueParameterImpl::name() const { return name_; }
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/impl/IrValueParameterImpl.kt:32-32
void IrValueParameterImpl::set_name(const ::org::jetbrains::kotlin::name::Name& value) { name_ = value; }
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/impl/IrValueParameterImpl.kt:33-33
types::IrType& IrValueParameterImpl::type() const { return *type_; }
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/impl/IrValueParameterImpl.kt:33-33
void IrValueParameterImpl::set_type(types::IrType& value) { type_ = &value; }
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/impl/IrValueParameterImpl.kt:34-34
IrParameterKind IrValueParameterImpl::kind() const { return kind_; }
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/impl/IrValueParameterImpl.kt:34-34
void IrValueParameterImpl::set_kind(IrParameterKind value) { kind_ = value; }
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/impl/IrValueParameterImpl.kt:35-35
bool IrValueParameterImpl::is_assignable() const { return is_assignable_; }
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/impl/IrValueParameterImpl.kt:35-35
void IrValueParameterImpl::set_is_assignable(bool value) { is_assignable_ = value; }
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/impl/IrValueParameterImpl.kt:37-37
types::IrType* IrValueParameterImpl::vararg_element_type() const { return vararg_element_type_; }
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/impl/IrValueParameterImpl.kt:37-37
void IrValueParameterImpl::set_vararg_element_type(types::IrType* value) { vararg_element_type_ = value; }
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/impl/IrValueParameterImpl.kt:38-38
bool IrValueParameterImpl::is_crossinline() const { return is_crossinline_; }
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/impl/IrValueParameterImpl.kt:38-38
void IrValueParameterImpl::set_is_crossinline(bool value) { is_crossinline_ = value; }
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/impl/IrValueParameterImpl.kt:39-39
bool IrValueParameterImpl::is_noinline() const { return is_noinline_; }
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/impl/IrValueParameterImpl.kt:39-39
void IrValueParameterImpl::set_is_noinline(bool value) { is_noinline_ = value; }
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/impl/IrValueParameterImpl.kt:40-40
bool IrValueParameterImpl::is_hidden() const { return is_hidden_; }
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/impl/IrValueParameterImpl.kt:40-40
void IrValueParameterImpl::set_is_hidden(bool value) { is_hidden_ = value; }
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/impl/IrValueParameterImpl.kt:42-42
IrElement& IrValueParameterImpl::attribute_owner_id() const { return *attribute_owner_id_; }
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/impl/IrValueParameterImpl.kt:42-42
void IrValueParameterImpl::set_attribute_owner_id(IrElement& value) { attribute_owner_id_ = &value; }
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/impl/IrValueParameterImpl.kt:44-44
::kotlin::collections::List<expressions::IrAnnotation*>& IrValueParameterImpl::annotations() const { return *annotations_; }
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/impl/IrValueParameterImpl.kt:44-44
void IrValueParameterImpl::set_annotations(::kotlin::collections::List<expressions::IrAnnotation*>& value) { annotations_ = &value; }
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/impl/IrValueParameterImpl.kt:50-50
expressions::IrExpressionBody* IrValueParameterImpl::default_value() const { return default_value_; }
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/impl/IrValueParameterImpl.kt:50-50
void IrValueParameterImpl::set_default_value(expressions::IrExpressionBody* value) { default_value_ = value; }
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/impl/IrValueParameterImpl.kt:46-48
::org::jetbrains::kotlin::descriptors::ParameterDescriptor& IrValueParameterImpl::descriptor() const { return symbol_->descriptor(); }
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/impl/IrValueParameterImpl.kt:36-36
symbols::IrSymbol& IrValueParameterImpl::symbol_dispatch() const { return *symbol_; }
}  // namespace org::jetbrains::kotlin::ir::declarations::impl
