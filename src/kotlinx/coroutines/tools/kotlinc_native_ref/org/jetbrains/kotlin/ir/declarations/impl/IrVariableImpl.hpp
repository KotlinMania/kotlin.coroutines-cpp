/*
 * Copyright 2010-2024 JetBrains s.r.o. and Kotlin Programming Language contributors.
 * Use of this source code is governed by the Apache 2.0 license that can be found in the license/LICENSE.txt file.
 */
// port-lint: source compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/impl/IrVariableImpl.kt
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/impl/IrVariableImpl.kt:26-54
#pragma once

#include "../IrVariable.hpp"

namespace org::jetbrains::kotlin::ir::util { class IrElementConstructorIndicator; }

namespace org::jetbrains::kotlin::ir::declarations::impl {
// NOTE(port): The compiler retains the actual referenced type, origin,
// annotations and symbol. Name already has an immutable C++ value representation;
// storing that value preserves it when a Name factory result is temporary.
// Kotlin internal construction is module-level visibility in the compiler port.
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/impl/IrVariableImpl.kt:26-54
class IrVariableImpl final : public IrVariable {
 public:
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/impl/IrVariableImpl.kt:26-40
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/impl/IrVariableImpl.kt:49-53
  IrVariableImpl(util::IrElementConstructorIndicator* constructor_indicator,
                 std::int32_t start_offset, std::int32_t end_offset,
                 IrDeclarationOrigin& origin,
                 const ::org::jetbrains::kotlin::name::Name& name,
                 types::IrType& type, symbols::IrVariableSymbol& symbol,
                 bool is_var, bool is_const, bool is_lateinit);
  // NOTE(port): Kotlin IR nodes are reference objects. A C++ copy or move must
  // not manufacture a second owner while its symbol remains bound to the first.
  IrVariableImpl(const IrVariableImpl&) = delete;
  IrVariableImpl& operator=(const IrVariableImpl&) = delete;
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/impl/IrVariableImpl.kt:28-28
  std::int32_t start_offset() const override;
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/impl/IrVariableImpl.kt:28-28
  void set_start_offset(std::int32_t value) override;
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/impl/IrVariableImpl.kt:29-29
  std::int32_t end_offset() const override;
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/impl/IrVariableImpl.kt:29-29
  void set_end_offset(std::int32_t value) override;
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/impl/IrVariableImpl.kt:30-30
  IrDeclarationOrigin& origin() const override;
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/impl/IrVariableImpl.kt:30-30
  void set_origin(IrDeclarationOrigin& value) override;
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/impl/IrVariableImpl.kt:31-31
  const ::org::jetbrains::kotlin::name::Name& name() const override;
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/impl/IrVariableImpl.kt:31-31
  void set_name(const ::org::jetbrains::kotlin::name::Name& value) override;
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/impl/IrVariableImpl.kt:32-32
  types::IrType& type() const override;
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/impl/IrVariableImpl.kt:32-32
  void set_type(types::IrType& value) override;
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/impl/IrVariableImpl.kt:34-34
  bool is_var() const override;
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/impl/IrVariableImpl.kt:34-34
  void set_is_var(bool value) override;
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/impl/IrVariableImpl.kt:35-35
  bool is_const() const override;
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/impl/IrVariableImpl.kt:35-35
  void set_is_const(bool value) override;
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/impl/IrVariableImpl.kt:36-36
  bool is_lateinit() const override;
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/impl/IrVariableImpl.kt:36-36
  void set_is_lateinit(bool value) override;
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/impl/IrVariableImpl.kt:38-38
  IrElement& attribute_owner_id() const override;
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/impl/IrVariableImpl.kt:38-38
  void set_attribute_owner_id(IrElement& value) override;
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/impl/IrVariableImpl.kt:40-40
  ::kotlin::collections::List<expressions::IrAnnotation*>& annotations() const override;
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/impl/IrVariableImpl.kt:40-40
  void set_annotations(::kotlin::collections::List<expressions::IrAnnotation*>& value) override;
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/impl/IrVariableImpl.kt:49-49
  expressions::IrExpression* initializer() const override;
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/impl/IrVariableImpl.kt:49-49
  void set_initializer(expressions::IrExpression* value) override;
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/impl/IrVariableImpl.kt:42-43
  IrFactory& factory() const override;
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/impl/IrVariableImpl.kt:45-47
  ::org::jetbrains::kotlin::descriptors::VariableDescriptor& descriptor() const override;

 protected:
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/impl/IrVariableImpl.kt:33-33
  symbols::IrSymbol& symbol_dispatch() const override;

 private:
  std::int32_t start_offset_;
  std::int32_t end_offset_;
  IrDeclarationOrigin* origin_;
  ::org::jetbrains::kotlin::name::Name name_;
  types::IrType* type_;
  symbols::IrVariableSymbol* const symbol_;
  bool is_var_;
  bool is_const_;
  bool is_lateinit_;
  IrElement* attribute_owner_id_;
  ::kotlin::collections::List<expressions::IrAnnotation*>* annotations_;
  expressions::IrExpression* initializer_ = nullptr;
};
}  // namespace org::jetbrains::kotlin::ir::declarations::impl
