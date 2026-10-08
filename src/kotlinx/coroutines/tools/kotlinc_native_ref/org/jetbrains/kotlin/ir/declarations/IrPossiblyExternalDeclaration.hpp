/*
 * Copyright 2010-2024 JetBrains s.r.o. and Kotlin Programming Language contributors.
 * Use of this source code is governed by the Apache 2.0 license that can be found in the license/LICENSE.txt file.
 */
// port-lint: source compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/IrPossiblyExternalDeclaration.kt
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/IrPossiblyExternalDeclaration.kt:14-16
#pragma once
#include "IrDeclarationWithName.hpp"

namespace org::jetbrains::kotlin::ir::declarations {
/**
 * Generated from: [org.jetbrains.kotlin.ir.generator.IrTree.possiblyExternalDeclaration]
 */
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/IrPossiblyExternalDeclaration.kt:14-16
class IrPossiblyExternalDeclaration : public virtual IrDeclarationWithName {
 public:
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/IrPossiblyExternalDeclaration.kt:15-15
  virtual bool is_external() const = 0;
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/IrPossiblyExternalDeclaration.kt:15-15
  virtual void set_is_external(bool value) = 0;
};
}  // namespace org::jetbrains::kotlin::ir::declarations
