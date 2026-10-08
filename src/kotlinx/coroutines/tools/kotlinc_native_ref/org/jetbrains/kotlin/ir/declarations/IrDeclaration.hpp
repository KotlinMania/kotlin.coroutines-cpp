/*
 * Copyright 2010-2024 JetBrains s.r.o. and Kotlin Programming Language contributors.
 * Use of this source code is governed by the Apache 2.0 license that can be found in the license/LICENSE.txt file.
 */
// port-lint: source compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/IrDeclaration.kt
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/IrDeclaration.kt:18-27
#pragma once
#include "../IrStatement.hpp"
#include "IrSymbolOwner.hpp"
#include "IrMutableAnnotationContainer.hpp"
#include "IrDeclarationParent.hpp"
#include "../../descriptors/DeclarationDescriptor.hpp"
namespace org::jetbrains::kotlin::ir::declarations {
class IrDeclarationOrigin;
class IrFactory;
}
// NOTE(port): Descriptor-obsolescence opt-in remains compiler metadata work.
namespace org::jetbrains::kotlin::ir::declarations {
/**
 * Generated from: [org.jetbrains.kotlin.ir.generator.IrTree.declaration]
 */
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/IrDeclaration.kt:18-27
class IrDeclaration : public virtual IrStatement, public virtual IrSymbolOwner, public virtual IrMutableAnnotationContainer {
 public:
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/IrDeclaration.kt:20-20
  virtual ::org::jetbrains::kotlin::descriptors::DeclarationDescriptor& descriptor() const = 0;
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/IrDeclaration.kt:22-22
  virtual IrDeclarationOrigin& origin() const = 0;
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/IrDeclaration.kt:22-22
  virtual void set_origin(IrDeclarationOrigin& value) = 0;
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/IrDeclaration.kt:24-24
  virtual IrFactory& factory() const = 0;
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/IrDeclaration.kt:26-26
  virtual IrDeclarationParent& parent() const = 0;
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/IrDeclaration.kt:26-26
  virtual void set_parent(IrDeclarationParent& value) = 0;
};
}  // namespace org::jetbrains::kotlin::ir::declarations
