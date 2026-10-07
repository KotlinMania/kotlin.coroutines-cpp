/*
 * Copyright 2010-2024 JetBrains s.r.o. and Kotlin Programming Language contributors.
 * Use of this source code is governed by the Apache 2.0 license that can be found in the license/LICENSE.txt file.
 */
// port-lint: source compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/IrDeclarationContainer.kt
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/IrDeclarationContainer.kt:16-25
#pragma once
#include "IrDeclarationParent.hpp"
namespace kotlin::collections { template <typename> class MutableList; }
namespace org::jetbrains::kotlin::ir::declarations { class IrDeclaration; }

namespace org::jetbrains::kotlin::ir::declarations {
/**
 * Generated from: [org.jetbrains.kotlin.ir.generator.IrTree.declarationContainer]
 */
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/IrDeclarationContainer.kt:16-25
class IrDeclarationContainer : public virtual IrDeclarationParent {
 public:
  /**
   * Accessing list of declaration may trigger lazy declaration list computation for lazy class,
   *   which requires computation of fake-overrides for this class. So it's unsafe to access it
   *   before IR for all sources is built (because fake-overrides of lazy classes may depend on
   *   declaration of source classes, e.g. for java source classes)
   */
  // NOTE(port): Construction-phase opt-in metadata remains untranslated.
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/IrDeclarationContainer.kt:24-24
  virtual ::kotlin::collections::MutableList<IrDeclaration*>& declarations() const = 0;
};
}  // namespace org::jetbrains::kotlin::ir::declarations
