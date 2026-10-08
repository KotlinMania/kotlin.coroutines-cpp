/*
 * Copyright 2010-2024 JetBrains s.r.o. and Kotlin Programming Language contributors.
 * Use of this source code is governed by the Apache 2.0 license that can be found in the license/LICENSE.txt file.
 */
// port-lint: source compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/IrValueParameter.kt
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/IrValueParameter.kt:23-87
#include "IrValueParameter.hpp"
#include "../symbols/IrValueParameterSymbol.hpp"
namespace org::jetbrains::kotlin::ir::declarations {
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/IrValueParameter.kt:31-31
symbols::IrValueParameterSymbol& IrValueParameter::symbol() const { return dynamic_cast<symbols::IrValueParameterSymbol&>(symbol_dispatch()); }
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/IrValueParameter.kt:70-72
std::int32_t IrValueParameter::index_in_parameters() const { return index_in_parameters_; }
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/IrValueParameter.kt:70-72
void IrValueParameter::set_index_in_parameters(std::int32_t value) { index_in_parameters_ = value; }
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/IrValueParameter.kt:74-75
void IrValueParameter::accept_dispatch(visitors::detail::IrVisitorDispatch& dispatch) { dispatch.visit_value_parameter(*this); }
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/IrValueParameter.kt:77-78
void IrValueParameter::transform_dispatch(visitors::detail::IrTransformerDispatch& dispatch) {
  dispatch.accept(*this);
  (void)dynamic_cast<IrValueParameter&>(dispatch.take_element_result());
}
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/IrValueParameter.kt:80-82
void IrValueParameter::accept_children_dispatch(visitors::detail::IrVisitorDispatch& dispatch) {
  if (auto* value = default_value()) dispatch.accept(*value);
}
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/IrValueParameter.kt:84-86
void IrValueParameter::transform_children_dispatch(visitors::detail::IrTransformerDispatch& dispatch) {
  auto* value = default_value();
  set_default_value(value == nullptr ? nullptr : &dynamic_cast<expressions::IrExpressionBody&>(dispatch.transform(*value)));
}
}  // namespace org::jetbrains::kotlin::ir::declarations
