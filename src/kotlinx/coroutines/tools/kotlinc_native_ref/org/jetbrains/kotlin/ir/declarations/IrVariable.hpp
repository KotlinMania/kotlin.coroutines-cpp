/*
 * Copyright 2010-2024 JetBrains s.r.o. and Kotlin Programming Language contributors.
 * Use of this source code is governed by the Apache 2.0 license that can be found in the license/LICENSE.txt file.
 */
// port-lint: source compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/IrVariable.kt
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/IrVariable.kt:18-45
#pragma once

#include "IrDeclarationBase.hpp"
#include "IrValueDeclaration.hpp"
#include "../../descriptors/VariableDescriptor.hpp"

namespace org::jetbrains::kotlin::ir::symbols { class IrVariableSymbol; }
namespace org::jetbrains::kotlin::ir::expressions { class IrExpression; }

namespace org::jetbrains::kotlin::ir::declarations {
/**
 * Generated from: [org.jetbrains.kotlin.ir.generator.IrTree.variable]
 */
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/IrVariable.kt:21-45
class IrVariable : public IrDeclarationBase, public virtual IrValueDeclaration {
 public:
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/IrVariable.kt:22-23
  virtual ::org::jetbrains::kotlin::descriptors::VariableDescriptor& descriptor() const override = 0;
  // NOTE(port): Retain the typed getter through the one abstract root property.
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/IrVariable.kt:25-25
  symbols::IrVariableSymbol& symbol() const;
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/IrVariable.kt:27-27
  virtual bool is_var() const = 0;
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/IrVariable.kt:27-27
  virtual void set_is_var(bool value) = 0;
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/IrVariable.kt:29-29
  virtual bool is_const() const = 0;
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/IrVariable.kt:29-29
  virtual void set_is_const(bool value) = 0;
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/IrVariable.kt:31-31
  virtual bool is_lateinit() const = 0;
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/IrVariable.kt:31-31
  virtual void set_is_lateinit(bool value) = 0;
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/IrVariable.kt:33-33
  virtual expressions::IrExpression* initializer() const = 0;
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/IrVariable.kt:33-33
  virtual void set_initializer(expressions::IrExpression* value) = 0;

 protected:
  // NOTE(port): The generic source operations use the existing typed virtual boundary.
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/IrVariable.kt:35-36
  void accept_dispatch(visitors::detail::IrVisitorDispatch& dispatch) override;
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/IrVariable.kt:38-40
  void accept_children_dispatch(visitors::detail::IrVisitorDispatch& dispatch) override;
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/IrVariable.kt:42-44
  void transform_children_dispatch(visitors::detail::IrTransformerDispatch& dispatch) override;
};
}  // namespace org::jetbrains::kotlin::ir::declarations
