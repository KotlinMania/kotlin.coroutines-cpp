/*
 * Copyright 2010-2024 JetBrains s.r.o. and Kotlin Programming Language contributors.
 * Use of this source code is governed by the Apache 2.0 license that can be found in the license/LICENSE.txt file.
 */
// port-lint: source compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/expressions/IrGetValue.kt
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/expressions/IrGetValue.kt:16-19
#pragma once
#include "IrValueAccessExpression.hpp"

namespace org::jetbrains::kotlin::ir::expressions {
/**
 * Generated from: [org.jetbrains.kotlin.ir.generator.IrTree.getValue]
 */
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/expressions/IrGetValue.kt:16-19
class IrGetValue : public IrValueAccessExpression {
 protected:
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/expressions/IrGetValue.kt:17-18
  void accept_dispatch(visitors::detail::IrVisitorDispatch& dispatch) override;
};
}  // namespace org::jetbrains::kotlin::ir::expressions
