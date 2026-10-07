/*
 * Copyright 2010-2024 JetBrains s.r.o. and Kotlin Programming Language contributors.
 * Use of this source code is governed by the Apache 2.0 license that can be found in the license/LICENSE.txt file.
 */
// port-lint: source compiler/ir/ir.tree/src/org/jetbrains/kotlin/ir/declarations/IrDeclarationBase.kt
// Transliterated from: compiler/ir/ir.tree/src/org/jetbrains/kotlin/ir/declarations/IrDeclarationBase.kt:17-28
#include "IrDeclarationBase.hpp"
#include "../util/RenderIrElement.hpp"

#include <stdexcept>

namespace org::jetbrains::kotlin::ir::declarations {
// Transliterated from: compiler/ir/ir.tree/src/org/jetbrains/kotlin/ir/declarations/IrDeclarationBase.kt:18-19
IrDeclarationParent* IrDeclarationBase::parent_or_null() const { return parent_; }

// NOTE(port): Kotlin error's IllegalStateException uses the C++ logic-error boundary.
// Transliterated from: compiler/ir/ir.tree/src/org/jetbrains/kotlin/ir/declarations/IrDeclarationBase.kt:20-24
IrDeclarationParent& IrDeclarationBase::parent() const {
  if (parent_ != nullptr) return *parent_;
  throw std::logic_error(
      "Parent of element (" + util::render(*this) + ") is not initialized.\n" +
      "Please assign it explicitly or use utility such as IrElement.patchDeclarationParents().");
}

// Transliterated from: compiler/ir/ir.tree/src/org/jetbrains/kotlin/ir/declarations/IrDeclarationBase.kt:25-27
void IrDeclarationBase::set_parent(IrDeclarationParent& value) { parent_ = &value; }
}  // namespace org::jetbrains::kotlin::ir::declarations
