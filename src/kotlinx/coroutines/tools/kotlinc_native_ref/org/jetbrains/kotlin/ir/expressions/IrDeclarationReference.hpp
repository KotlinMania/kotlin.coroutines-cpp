/*
 * Copyright 2010-2024 JetBrains s.r.o. and Kotlin Programming Language contributors.
 * Use of this source code is governed by the Apache 2.0 license that can be found in the license/LICENSE.txt file.
 */
// port-lint: source compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/expressions/IrDeclarationReference.kt
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/expressions/IrDeclarationReference.kt:16-18
#pragma once
#include "IrExpression.hpp"
#include "../symbols/IrSymbol.hpp"

namespace org::jetbrains::kotlin::ir::expressions {
/**
 * Generated from: [org.jetbrains.kotlin.ir.generator.IrTree.declarationReference]
 */
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/expressions/IrDeclarationReference.kt:16-18
class IrDeclarationReference : public IrExpression {
 public:
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/expressions/IrDeclarationReference.kt:17-17
  virtual symbols::IrSymbol& symbol() const = 0;
};
}  // namespace org::jetbrains::kotlin::ir::expressions
