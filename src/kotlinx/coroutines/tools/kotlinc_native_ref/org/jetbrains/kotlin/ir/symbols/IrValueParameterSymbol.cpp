/*
 * Copyright 2010-2024 JetBrains s.r.o. and Kotlin Programming Language contributors.
 * Use of this source code is governed by the Apache 2.0 license that can be found in the license/LICENSE.txt file.
 */
// port-lint: source compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/symbols/IrSymbol.kt
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/symbols/IrSymbol.kt:159-159
// port-lint: source compiler/ir/ir.tree/src/org/jetbrains/kotlin/ir/symbols/IrSymbol.kt
#include "IrValueParameterSymbol.hpp"
namespace org::jetbrains::kotlin::ir::symbols {
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/symbols/IrSymbol.kt:159-159
// Transliterated from: compiler/ir/ir.tree/src/org/jetbrains/kotlin/ir/symbols/IrSymbol.kt:126-127
declarations::IrValueParameter& IrValueParameterSymbol::owner() const { return dynamic_cast<declarations::IrValueParameter&>(owner_dispatch()); }
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/symbols/IrSymbol.kt:159-159
// Transliterated from: compiler/ir/ir.tree/src/org/jetbrains/kotlin/ir/symbols/IrSymbol.kt:129-130
::org::jetbrains::kotlin::descriptors::ParameterDescriptor& IrValueParameterSymbol::descriptor() const { return dynamic_cast<::org::jetbrains::kotlin::descriptors::ParameterDescriptor&>(descriptor_dispatch()); }
}  // namespace org::jetbrains::kotlin::ir::symbols
