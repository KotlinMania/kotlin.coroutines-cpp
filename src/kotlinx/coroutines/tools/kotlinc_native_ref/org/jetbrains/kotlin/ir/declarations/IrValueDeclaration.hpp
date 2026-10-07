/*
 * Copyright 2010-2024 JetBrains s.r.o. and Kotlin Programming Language contributors.
 * Use of this source code is governed by the Apache 2.0 license that can be found in the license/LICENSE.txt file.
 */
// port-lint: source compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/IrValueDeclaration.kt
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/IrValueDeclaration.kt:19-26
#pragma once

#include "IrDeclarationWithName.hpp"
#include "IrSymbolOwner.hpp"
#include "../../descriptors/ValueDescriptor.hpp"
#include "../types/IrType.hpp"

namespace org::jetbrains::kotlin::ir::symbols { class IrValueSymbol; }

namespace org::jetbrains::kotlin::ir::declarations {
/**
 * Generated from: [org.jetbrains.kotlin.ir.generator.IrTree.valueDeclaration]
 */
// NOTE(port): Descriptor-obsolescence metadata remains open. Symbol, type and
// descriptor access borrow the actual compiler objects and retain their identity.
// The typed symbol getter uses the abstract root virtual property boundary.
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/IrValueDeclaration.kt:19-26
class IrValueDeclaration : public virtual IrDeclarationWithName,
                           public virtual IrSymbolOwner {
 public:
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/IrValueDeclaration.kt:20-21
  virtual ::org::jetbrains::kotlin::descriptors::ValueDescriptor& descriptor() const override = 0;
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/IrValueDeclaration.kt:23-23
  symbols::IrValueSymbol& symbol() const;
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/IrValueDeclaration.kt:25-25
  virtual types::IrType& type() const = 0;
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/IrValueDeclaration.kt:25-25
  virtual void set_type(types::IrType& value) = 0;
};
}  // namespace org::jetbrains::kotlin::ir::declarations
