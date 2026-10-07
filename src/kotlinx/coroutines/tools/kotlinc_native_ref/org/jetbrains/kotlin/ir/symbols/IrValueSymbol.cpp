/*
 * Copyright 2010-2024 JetBrains s.r.o. and Kotlin Programming Language contributors.
 * Use of this source code is governed by the Apache 2.0 license that can be found in the license/LICENSE.txt file.
 */
// port-lint: source compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/symbols/IrSymbol.kt
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/symbols/IrSymbol.kt:146-152
#include "IrValueSymbol.hpp"
#include "../declarations/IrValueDeclaration.hpp"

namespace org::jetbrains::kotlin::ir::symbols {

// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/symbols/IrSymbol.kt:147-148
::org::jetbrains::kotlin::descriptors::ValueDescriptor& IrValueSymbol::descriptor() const {
  return dynamic_cast<::org::jetbrains::kotlin::descriptors::ValueDescriptor&>(descriptor_dispatch());
}

// NOTE(port): Narrow the same virtual owner after the genuine declaration
// hierarchy is complete; compiler ownership and identity remain unchanged.
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/symbols/IrSymbol.kt:150-151
declarations::IrValueDeclaration& IrValueSymbol::owner() const {
  return dynamic_cast<declarations::IrValueDeclaration&>(owner_dispatch());
}

}  // namespace org::jetbrains::kotlin::ir::symbols
