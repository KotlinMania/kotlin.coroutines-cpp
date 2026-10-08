/*
 * Copyright 2010-2024 JetBrains s.r.o. and Kotlin Programming Language contributors.
 * Use of this source code is governed by the Apache 2.0 license that can be found in the license/LICENSE.txt file.
 */
// port-lint: source compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/expressions/IrValueAccessExpression.kt
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/expressions/IrValueAccessExpression.kt:16-20
#pragma once
#include "IrDeclarationReference.hpp"
#include "../symbols/IrValueSymbol.hpp"

namespace org::jetbrains::kotlin::ir::expressions {
class IrStatementOrigin;
/**
 * Generated from: [org.jetbrains.kotlin.ir.generator.IrTree.valueAccessExpression]
 */
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/expressions/IrValueAccessExpression.kt:16-20
class IrValueAccessExpression : public IrDeclarationReference {
 public:
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/expressions/IrValueAccessExpression.kt:17-17
  virtual symbols::IrValueSymbol& symbol() const override = 0;
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/expressions/IrValueAccessExpression.kt:17-17
  virtual void set_symbol(symbols::IrValueSymbol& value) = 0;
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/expressions/IrValueAccessExpression.kt:19-19
  virtual IrStatementOrigin* origin() const = 0;
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/expressions/IrValueAccessExpression.kt:19-19
  virtual void set_origin(IrStatementOrigin* value) = 0;
};
}  // namespace org::jetbrains::kotlin::ir::expressions
