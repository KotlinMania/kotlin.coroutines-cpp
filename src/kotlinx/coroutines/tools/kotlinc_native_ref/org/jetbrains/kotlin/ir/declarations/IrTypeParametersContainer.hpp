/*
 * Copyright 2010-2024 JetBrains s.r.o. and Kotlin Programming Language contributors.
 * Use of this source code is governed by the Apache 2.0 license that can be found in the license/LICENSE.txt file.
 */
// port-lint: source compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/IrTypeParametersContainer.kt
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/IrTypeParametersContainer.kt:14-16
#pragma once
#include "IrDeclaration.hpp"
#include "IrDeclarationParent.hpp"
#include <memory>
// NOTE(port): Strong source List references retain the same C++ shared list.
// Elements remain borrowed compiler-owned declarations, as in existing IR nodes.
namespace kotlin::collections { template <typename> class List; }
namespace org::jetbrains::kotlin::ir::declarations { class IrTypeParameter; }

namespace org::jetbrains::kotlin::ir::declarations {
/**
 * Generated from: [org.jetbrains.kotlin.ir.generator.IrTree.typeParametersContainer]
 */
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/IrTypeParametersContainer.kt:14-16
class IrTypeParametersContainer : public virtual IrDeclaration, public virtual IrDeclarationParent {
 public:
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/IrTypeParametersContainer.kt:15-15
  virtual std::shared_ptr<::kotlin::collections::List<IrTypeParameter*>> type_parameters() const = 0;
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/IrTypeParametersContainer.kt:15-15
  virtual void set_type_parameters(std::shared_ptr<::kotlin::collections::List<IrTypeParameter*>> value) = 0;
};
}  // namespace org::jetbrains::kotlin::ir::declarations
