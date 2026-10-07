/*
 * Copyright 2010-2024 JetBrains s.r.o. and Kotlin Programming Language contributors.
 * Use of this source code is governed by the Apache 2.0 license that can be found in the license/LICENSE.txt file.
 */
// port-lint: source compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/IrClass.kt
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/IrClass.kt:74-84
#include "IrClass.hpp"
#include "IrTypeParameter.hpp"
#include "IrValueParameter.hpp"
#include "../../../../../kotlin/collections/MutableList.hpp"
#include "../util/Transform.hpp"
namespace org::jetbrains::kotlin::ir::declarations {
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/IrClass.kt:74-78
void IrClass::accept_children_dispatch(visitors::detail::IrVisitorDispatch& dispatch) {
  auto parameters = type_parameters()->iterator();
  while (parameters->has_next()) dispatch.accept(*parameters->next());
  auto members = declarations().iterator();
  while (members->has_next()) dispatch.accept(*members->next());
  if (auto* receiver = this_receiver()) dispatch.accept(*receiver);
}
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/IrClass.kt:80-84
void IrClass::transform_children_dispatch(visitors::detail::IrTransformerDispatch& dispatch) {
  set_type_parameters(util::transform_if_needed(type_parameters(), dispatch));
  util::transform_in_place(declarations(), dispatch);
  auto* receiver = this_receiver();
  set_this_receiver(receiver == nullptr ? nullptr : &dynamic_cast<IrValueParameter&>(dispatch.transform(*receiver)));
}
}  // namespace org::jetbrains::kotlin::ir::declarations
