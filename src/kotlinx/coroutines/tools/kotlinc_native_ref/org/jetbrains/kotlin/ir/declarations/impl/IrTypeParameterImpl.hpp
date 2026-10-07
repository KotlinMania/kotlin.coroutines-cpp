/*
 * Copyright 2010-2024 JetBrains s.r.o. and Kotlin Programming Language contributors.
 * Use of this source code is governed by the Apache 2.0 license that can be found in the license/LICENSE.txt file.
 */
// port-lint: source compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/impl/IrTypeParameterImpl.kt
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/impl/IrTypeParameterImpl.kt:26-50
#pragma once
#include "../IrTypeParameter.hpp"
namespace org::jetbrains::kotlin::ir::declarations::impl {
// NOTE(port): Actual compiler objects/lists are borrowed under the existing IR
// ownership contract. Immutable Name is stored by value; no Native GC layout is implied.
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/impl/IrTypeParameterImpl.kt:26-50
class IrTypeParameterImpl final : public IrTypeParameter {
 public:
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/impl/IrTypeParameterImpl.kt:26-36
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/impl/IrTypeParameterImpl.kt:37-39
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/impl/IrTypeParameterImpl.kt:45-49
  IrTypeParameterImpl(std::int32_t start_offset,
      std::int32_t end_offset,
      IrDeclarationOrigin& origin,
      IrFactory& factory,
      const ::org::jetbrains::kotlin::name::Name& name,
      symbols::IrTypeParameterSymbol& symbol,
      ::org::jetbrains::kotlin::types::Variance variance,
      std::int32_t index,
      bool is_reified);
  // NOTE(port): Copying a source declaration must not create a second symbol owner.
  IrTypeParameterImpl(const IrTypeParameterImpl&) = delete;
  IrTypeParameterImpl& operator=(const IrTypeParameterImpl&) = delete;
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/impl/IrTypeParameterImpl.kt:27-27
  std::int32_t start_offset() const override;
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/impl/IrTypeParameterImpl.kt:27-27
  void set_start_offset(std::int32_t value) override;
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/impl/IrTypeParameterImpl.kt:28-28
  std::int32_t end_offset() const override;
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/impl/IrTypeParameterImpl.kt:28-28
  void set_end_offset(std::int32_t value) override;
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/impl/IrTypeParameterImpl.kt:29-29
  IrDeclarationOrigin& origin() const override;
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/impl/IrTypeParameterImpl.kt:29-29
  void set_origin(IrDeclarationOrigin& value) override;
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/impl/IrTypeParameterImpl.kt:30-30
  IrFactory& factory() const override;
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/impl/IrTypeParameterImpl.kt:31-31
  const ::org::jetbrains::kotlin::name::Name& name() const override;
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/impl/IrTypeParameterImpl.kt:31-31
  void set_name(const ::org::jetbrains::kotlin::name::Name& value) override;
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/impl/IrTypeParameterImpl.kt:33-33
  ::org::jetbrains::kotlin::types::Variance variance() const override;
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/impl/IrTypeParameterImpl.kt:33-33
  void set_variance(::org::jetbrains::kotlin::types::Variance value) override;
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/impl/IrTypeParameterImpl.kt:34-34
  std::int32_t index() const override;
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/impl/IrTypeParameterImpl.kt:34-34
  void set_index(std::int32_t value) override;
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/impl/IrTypeParameterImpl.kt:35-35
  bool is_reified() const override;
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/impl/IrTypeParameterImpl.kt:35-35
  void set_is_reified(bool value) override;
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/impl/IrTypeParameterImpl.kt:37-37
  IrElement& attribute_owner_id() const override;
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/impl/IrTypeParameterImpl.kt:37-37
  void set_attribute_owner_id(IrElement& value) override;
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/impl/IrTypeParameterImpl.kt:39-39
  ::kotlin::collections::List<expressions::IrAnnotation*>& annotations() const override;
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/impl/IrTypeParameterImpl.kt:39-39
  void set_annotations(::kotlin::collections::List<expressions::IrAnnotation*>& value) override;
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/impl/IrTypeParameterImpl.kt:45-45
  ::kotlin::collections::List<types::IrType*>& super_types() const override;
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/impl/IrTypeParameterImpl.kt:45-45
  void set_super_types(::kotlin::collections::List<types::IrType*>& value) override;
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/impl/IrTypeParameterImpl.kt:41-43
  ::org::jetbrains::kotlin::descriptors::TypeParameterDescriptor& descriptor() const override;
 protected:
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/impl/IrTypeParameterImpl.kt:32-32
  symbols::IrSymbol& symbol_dispatch() const override;
 private:
  std::int32_t start_offset_;
  std::int32_t end_offset_;
  IrDeclarationOrigin* origin_;
  IrFactory* const factory_;
  ::org::jetbrains::kotlin::name::Name name_;
  symbols::IrTypeParameterSymbol* const symbol_;
  ::org::jetbrains::kotlin::types::Variance variance_;
  std::int32_t index_;
  bool is_reified_;
  IrElement* attribute_owner_id_;
  ::kotlin::collections::List<expressions::IrAnnotation*>* annotations_;
  ::kotlin::collections::List<types::IrType*>* super_types_;
};
}  // namespace org::jetbrains::kotlin::ir::declarations::impl
