/*
 * Copyright 2010-2024 JetBrains s.r.o. and Kotlin Programming Language contributors.
 * Use of this source code is governed by the Apache 2.0 license that can be found in the license/LICENSE.txt file.
 */
// port-lint: source compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/IrValueParameter.kt
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/IrValueParameter.kt:23-87
#pragma once
#include "IrDeclarationBase.hpp"
#include "IrValueDeclaration.hpp"
#include "../../descriptors/ParameterDescriptor.hpp"
#include "../types/IrType.hpp"
#include "IrParameterKind.hpp"
#include "../expressions/IrExpressionBody.hpp"
namespace org::jetbrains::kotlin::ir::symbols { class IrValueParameterSymbol; }
namespace org::jetbrains::kotlin::ir::declarations {
/**
 * Generated from: [org.jetbrains.kotlin.ir.generator.IrTree.valueParameter]
 */
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/IrValueParameter.kt:23-87
class IrValueParameter : public IrDeclarationBase, public virtual IrValueDeclaration {
 public:
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/IrValueParameter.kt:24-25
  ::org::jetbrains::kotlin::descriptors::ParameterDescriptor& descriptor() const override = 0;
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/IrValueParameter.kt:27-27
  virtual IrParameterKind kind() const = 0;
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/IrValueParameter.kt:27-27
  virtual void set_kind(IrParameterKind value) = 0;
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/IrValueParameter.kt:29-29
  virtual bool is_assignable() const = 0;
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/IrValueParameter.kt:29-29
  virtual void set_is_assignable(bool value) = 0;
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/IrValueParameter.kt:31-31
  symbols::IrValueParameterSymbol& symbol() const;
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/IrValueParameter.kt:33-33
  virtual types::IrType* vararg_element_type() const = 0;
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/IrValueParameter.kt:33-33
  virtual void set_vararg_element_type(types::IrType* value) = 0;
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/IrValueParameter.kt:35-35
  virtual bool is_crossinline() const = 0;
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/IrValueParameter.kt:35-35
  virtual void set_is_crossinline(bool value) = 0;
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/IrValueParameter.kt:37-37
  virtual bool is_noinline() const = 0;
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/IrValueParameter.kt:37-37
  virtual void set_is_noinline(bool value) = 0;
  // NOTE(port): Line comments preserve nested comment examples and the upstream open question.
  //     /**
  //      * If `true`, the value parameter does not participate in [IdSignature] computation.
  //      *
  //      * This is a workaround that is needed for better support of compiler plugins.
  //      * Suppose you have the following code and some IR plugin that adds a value parameter to functions
  //      * marked with the `@PluginMarker` annotation.
  //      * ```kotlin
  //      * @PluginMarker
  //      * fun foo(defined: Int) { /* ... */ }
  //      * ```
  //      *
  //      * Suppose that after applying the plugin the function is changed to:
  //      * ```kotlin
  //      * @PluginMarker
  //      * fun foo(defined: Int, $extra: String) { /* ... */ }
  //      * ```
  //      *
  //      * If a compiler plugin adds parameters to an [IrFunction],
  //      * the representations of the function in the frontend and in the backend may diverge, potentially causing signature mismatch and
  //      * linkage errors (see [KT-40980](https://youtrack.jetbrains.com/issue/KT-40980)).
  //      * We wouldn't want IR plugins to affect the frontend representation, since in an IDE you'd want to be able to see those
  //      * declarations in their original form (without the `$extra` parameter).
  //      *
  //      * To fix this problem, [isHidden] was introduced.
  //      *
  //      * Upstream question: consider dropping [isHidden] if it isn't used by any known plugin.
  //      */
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/IrValueParameter.kt:66-66
  virtual bool is_hidden() const = 0;
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/IrValueParameter.kt:66-66
  virtual void set_is_hidden(bool value) = 0;
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/IrValueParameter.kt:68-68
  virtual expressions::IrExpressionBody* default_value() const = 0;
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/IrValueParameter.kt:68-68
  virtual void set_default_value(expressions::IrExpressionBody* value) = 0;
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/IrValueParameter.kt:70-72
  std::int32_t index_in_parameters() const;
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/IrValueParameter.kt:70-72
  void set_index_in_parameters(std::int32_t value);
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/IrValueParameter.kt:77-78
  template <typename D>
  IrValueParameter& transform(visitors::IrTransformer<D>& transformer, D data) {
    return dynamic_cast<IrValueParameter&>(IrElement::transform(transformer, data));
  }
 protected:
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/IrValueParameter.kt:74-75
  void accept_dispatch(visitors::detail::IrVisitorDispatch& dispatch) override;
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/IrValueParameter.kt:77-78
  void transform_dispatch(visitors::detail::IrTransformerDispatch& dispatch) override;
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/IrValueParameter.kt:80-82
  void accept_children_dispatch(visitors::detail::IrVisitorDispatch& dispatch) override;
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/IrValueParameter.kt:84-86
  void transform_children_dispatch(visitors::detail::IrTransformerDispatch& dispatch) override;
 private:
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/IrValueParameter.kt:70-72
  std::int32_t index_in_parameters_ = -1;
};
}  // namespace org::jetbrains::kotlin::ir::declarations
