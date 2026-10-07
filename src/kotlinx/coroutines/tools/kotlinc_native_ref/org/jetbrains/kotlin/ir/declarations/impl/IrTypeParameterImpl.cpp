/*
 * Copyright 2010-2024 JetBrains s.r.o. and Kotlin Programming Language contributors.
 * Use of this source code is governed by the Apache 2.0 license that can be found in the license/LICENSE.txt file.
 */
// port-lint: source compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/impl/IrTypeParameterImpl.kt
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/impl/IrTypeParameterImpl.kt:26-50
#include "IrTypeParameterImpl.hpp"
#include "../../symbols/IrTypeParameterSymbol.hpp"
#include "../../../../../../kotlin/collections/Collections.hpp"
namespace org::jetbrains::kotlin::ir::declarations::impl {
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/impl/IrTypeParameterImpl.kt:26-36
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/impl/IrTypeParameterImpl.kt:37-39
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/impl/IrTypeParameterImpl.kt:45-49
IrTypeParameterImpl::IrTypeParameterImpl(std::int32_t start_offset,
      std::int32_t end_offset,
      IrDeclarationOrigin& origin,
      IrFactory& factory,
      const ::org::jetbrains::kotlin::name::Name& name,
      symbols::IrTypeParameterSymbol& symbol,
      ::org::jetbrains::kotlin::types::Variance variance,
      std::int32_t index,
      bool is_reified)
    : start_offset_(start_offset),
      end_offset_(end_offset),
      origin_(&origin),
      factory_(&factory),
      name_(name),
      symbol_(&symbol),
      variance_(variance),
      index_(index),
      is_reified_(is_reified),
      attribute_owner_id_(this),
      annotations_(&::kotlin::collections::empty_list<expressions::IrAnnotation*>()),
      super_types_(&::kotlin::collections::empty_list<types::IrType*>()) {
  symbol.bind(*this);
}
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/impl/IrTypeParameterImpl.kt:27-27
std::int32_t IrTypeParameterImpl::start_offset() const { return start_offset_; }
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/impl/IrTypeParameterImpl.kt:27-27
void IrTypeParameterImpl::set_start_offset(std::int32_t value) { start_offset_ = value; }
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/impl/IrTypeParameterImpl.kt:28-28
std::int32_t IrTypeParameterImpl::end_offset() const { return end_offset_; }
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/impl/IrTypeParameterImpl.kt:28-28
void IrTypeParameterImpl::set_end_offset(std::int32_t value) { end_offset_ = value; }
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/impl/IrTypeParameterImpl.kt:29-29
IrDeclarationOrigin& IrTypeParameterImpl::origin() const { return *origin_; }
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/impl/IrTypeParameterImpl.kt:29-29
void IrTypeParameterImpl::set_origin(IrDeclarationOrigin& value) { origin_ = &value; }
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/impl/IrTypeParameterImpl.kt:30-30
IrFactory& IrTypeParameterImpl::factory() const { return *factory_; }
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/impl/IrTypeParameterImpl.kt:31-31
const ::org::jetbrains::kotlin::name::Name& IrTypeParameterImpl::name() const { return name_; }
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/impl/IrTypeParameterImpl.kt:31-31
void IrTypeParameterImpl::set_name(const ::org::jetbrains::kotlin::name::Name& value) { name_ = value; }
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/impl/IrTypeParameterImpl.kt:33-33
::org::jetbrains::kotlin::types::Variance IrTypeParameterImpl::variance() const { return variance_; }
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/impl/IrTypeParameterImpl.kt:33-33
void IrTypeParameterImpl::set_variance(::org::jetbrains::kotlin::types::Variance value) { variance_ = value; }
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/impl/IrTypeParameterImpl.kt:34-34
std::int32_t IrTypeParameterImpl::index() const { return index_; }
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/impl/IrTypeParameterImpl.kt:34-34
void IrTypeParameterImpl::set_index(std::int32_t value) { index_ = value; }
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/impl/IrTypeParameterImpl.kt:35-35
bool IrTypeParameterImpl::is_reified() const { return is_reified_; }
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/impl/IrTypeParameterImpl.kt:35-35
void IrTypeParameterImpl::set_is_reified(bool value) { is_reified_ = value; }
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/impl/IrTypeParameterImpl.kt:37-37
IrElement& IrTypeParameterImpl::attribute_owner_id() const { return *attribute_owner_id_; }
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/impl/IrTypeParameterImpl.kt:37-37
void IrTypeParameterImpl::set_attribute_owner_id(IrElement& value) { attribute_owner_id_ = &value; }
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/impl/IrTypeParameterImpl.kt:39-39
::kotlin::collections::List<expressions::IrAnnotation*>& IrTypeParameterImpl::annotations() const { return *annotations_; }
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/impl/IrTypeParameterImpl.kt:39-39
void IrTypeParameterImpl::set_annotations(::kotlin::collections::List<expressions::IrAnnotation*>& value) { annotations_ = &value; }
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/impl/IrTypeParameterImpl.kt:45-45
::kotlin::collections::List<types::IrType*>& IrTypeParameterImpl::super_types() const { return *super_types_; }
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/impl/IrTypeParameterImpl.kt:45-45
void IrTypeParameterImpl::set_super_types(::kotlin::collections::List<types::IrType*>& value) { super_types_ = &value; }
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/impl/IrTypeParameterImpl.kt:41-43
::org::jetbrains::kotlin::descriptors::TypeParameterDescriptor& IrTypeParameterImpl::descriptor() const { return symbol_->descriptor(); }
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/impl/IrTypeParameterImpl.kt:32-32
symbols::IrSymbol& IrTypeParameterImpl::symbol_dispatch() const { return *symbol_; }
}  // namespace org::jetbrains::kotlin::ir::declarations::impl
