/*
 * Copyright 2010-2024 JetBrains s.r.o. and Kotlin Programming Language contributors.
 * Use of this source code is governed by the Apache 2.0 license that can be found in the license/LICENSE.txt file.
 */
// port-lint: source compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/IrDeclarationWithName.kt
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/IrDeclarationWithName.kt:16-18
#pragma once
#include "IrDeclaration.hpp"
#include "../../name/Name.hpp"

namespace org::jetbrains::kotlin::ir::declarations {
/**
 * Generated from: [org.jetbrains.kotlin.ir.generator.IrTree.declarationWithName]
 */
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/IrDeclarationWithName.kt:16-18
class IrDeclarationWithName : public virtual IrDeclaration {
 public:
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/IrDeclarationWithName.kt:17-17
  virtual const ::org::jetbrains::kotlin::name::Name& name() const = 0;
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/IrDeclarationWithName.kt:17-17
  virtual void set_name(const ::org::jetbrains::kotlin::name::Name& value) = 0;
};
}  // namespace org::jetbrains::kotlin::ir::declarations
