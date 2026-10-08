/*
 * Copyright 2010-2024 JetBrains s.r.o. and Kotlin Programming Language contributors.
 * Use of this source code is governed by the Apache 2.0 license that can be found in the license/LICENSE.txt file.
 */
// port-lint: source compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/impl/IrValueParameterImpl.kt
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/impl/IrValueParameterImpl.kt:27-55
#pragma once
#include "../IrValueParameter.hpp"
namespace org::jetbrains::kotlin::ir::declarations::impl {
// NOTE(port): Actual compiler objects/lists are borrowed under the existing IR
// ownership contract. Immutable Name is stored by value; no Native GC layout is implied.
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/impl/IrValueParameterImpl.kt:27-55
class IrValueParameterImpl final : public IrValueParameter {
 public:
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/impl/IrValueParameterImpl.kt:27-41
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/impl/IrValueParameterImpl.kt:42-44
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/impl/IrValueParameterImpl.kt:50-54
  IrValueParameterImpl(std::int32_t start_offset,
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
      bool is_hidden);
  // NOTE(port): Copying a source declaration must not create a second symbol owner.
  IrValueParameterImpl(const IrValueParameterImpl&) = delete;
  IrValueParameterImpl& operator=(const IrValueParameterImpl&) = delete;
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/impl/IrValueParameterImpl.kt:28-28
  std::int32_t start_offset() const override;
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/impl/IrValueParameterImpl.kt:28-28
  void set_start_offset(std::int32_t value) override;
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/impl/IrValueParameterImpl.kt:29-29
  std::int32_t end_offset() const override;
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/impl/IrValueParameterImpl.kt:29-29
  void set_end_offset(std::int32_t value) override;
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/impl/IrValueParameterImpl.kt:30-30
  IrDeclarationOrigin& origin() const override;
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/impl/IrValueParameterImpl.kt:30-30
  void set_origin(IrDeclarationOrigin& value) override;
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/impl/IrValueParameterImpl.kt:31-31
  IrFactory& factory() const override;
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/impl/IrValueParameterImpl.kt:32-32
  const ::org::jetbrains::kotlin::name::Name& name() const override;
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/impl/IrValueParameterImpl.kt:32-32
  void set_name(const ::org::jetbrains::kotlin::name::Name& value) override;
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/impl/IrValueParameterImpl.kt:33-33
  types::IrType& type() const override;
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/impl/IrValueParameterImpl.kt:33-33
  void set_type(types::IrType& value) override;
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/impl/IrValueParameterImpl.kt:34-34
  IrParameterKind kind() const override;
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/impl/IrValueParameterImpl.kt:34-34
  void set_kind(IrParameterKind value) override;
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/impl/IrValueParameterImpl.kt:35-35
  bool is_assignable() const override;
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/impl/IrValueParameterImpl.kt:35-35
  void set_is_assignable(bool value) override;
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/impl/IrValueParameterImpl.kt:37-37
  types::IrType* vararg_element_type() const override;
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/impl/IrValueParameterImpl.kt:37-37
  void set_vararg_element_type(types::IrType* value) override;
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/impl/IrValueParameterImpl.kt:38-38
  bool is_crossinline() const override;
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/impl/IrValueParameterImpl.kt:38-38
  void set_is_crossinline(bool value) override;
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/impl/IrValueParameterImpl.kt:39-39
  bool is_noinline() const override;
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/impl/IrValueParameterImpl.kt:39-39
  void set_is_noinline(bool value) override;
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/impl/IrValueParameterImpl.kt:40-40
  bool is_hidden() const override;
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/impl/IrValueParameterImpl.kt:40-40
  void set_is_hidden(bool value) override;
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/impl/IrValueParameterImpl.kt:42-42
  IrElement& attribute_owner_id() const override;
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/impl/IrValueParameterImpl.kt:42-42
  void set_attribute_owner_id(IrElement& value) override;
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/impl/IrValueParameterImpl.kt:44-44
  ::kotlin::collections::List<expressions::IrAnnotation*>& annotations() const override;
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/impl/IrValueParameterImpl.kt:44-44
  void set_annotations(::kotlin::collections::List<expressions::IrAnnotation*>& value) override;
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/impl/IrValueParameterImpl.kt:50-50
  expressions::IrExpressionBody* default_value() const override;
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/impl/IrValueParameterImpl.kt:50-50
  void set_default_value(expressions::IrExpressionBody* value) override;
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/impl/IrValueParameterImpl.kt:46-48
  ::org::jetbrains::kotlin::descriptors::ParameterDescriptor& descriptor() const override;
 protected:
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/impl/IrValueParameterImpl.kt:36-36
  symbols::IrSymbol& symbol_dispatch() const override;
 private:
  std::int32_t start_offset_;
  std::int32_t end_offset_;
  IrDeclarationOrigin* origin_;
  IrFactory* const factory_;
  ::org::jetbrains::kotlin::name::Name name_;
  types::IrType* type_;
  IrParameterKind kind_;
  bool is_assignable_;
  symbols::IrValueParameterSymbol* const symbol_;
  types::IrType* vararg_element_type_;
  bool is_crossinline_;
  bool is_noinline_;
  bool is_hidden_;
  IrElement* attribute_owner_id_;
  ::kotlin::collections::List<expressions::IrAnnotation*>* annotations_;
  expressions::IrExpressionBody* default_value_;
};
}  // namespace org::jetbrains::kotlin::ir::declarations::impl
