/*
 * Copyright 2010-2024 JetBrains s.r.o. and Kotlin Programming Language contributors.
 * Use of this source code is governed by the Apache 2.0 license that can be found in the license/LICENSE.txt file.
 */
// port-lint: source compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/IrDeclarationWithVisibility.kt
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/IrDeclarationWithVisibility.kt:16-18
#pragma once
#include "IrDeclaration.hpp"
namespace org::jetbrains::kotlin::descriptors { class DescriptorVisibility; }

namespace org::jetbrains::kotlin::ir::declarations {
/**
 * Generated from: [org.jetbrains.kotlin.ir.generator.IrTree.declarationWithVisibility]
 */
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/IrDeclarationWithVisibility.kt:16-18
class IrDeclarationWithVisibility : public virtual IrDeclaration {
 public:
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/IrDeclarationWithVisibility.kt:17-17
  virtual ::org::jetbrains::kotlin::descriptors::DescriptorVisibility& visibility() const = 0;
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/IrDeclarationWithVisibility.kt:17-17
  virtual void set_visibility(::org::jetbrains::kotlin::descriptors::DescriptorVisibility& value) = 0;
};
}  // namespace org::jetbrains::kotlin::ir::declarations
