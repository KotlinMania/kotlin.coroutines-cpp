/*
 * Copyright 2010-2024 JetBrains s.r.o. and Kotlin Programming Language contributors.
 * Use of this source code is governed by the Apache 2.0 license that can be found in the license/LICENSE.txt file.
 */
// port-lint: source compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/symbols/IrSymbol.kt
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/symbols/IrSymbol.kt:91-94
// port-lint: source compiler/ir/ir.tree/src/org/jetbrains/kotlin/ir/symbols/IrSymbol.kt
#include "IrClassifierSymbol.hpp"
namespace org::jetbrains::kotlin::ir::symbols {
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/symbols/IrSymbol.kt:91-94
// Transliterated from: compiler/ir/ir.tree/src/org/jetbrains/kotlin/ir/symbols/IrSymbol.kt:69-74
::org::jetbrains::kotlin::descriptors::ClassifierDescriptor& IrClassifierSymbol::descriptor() const { return dynamic_cast<::org::jetbrains::kotlin::descriptors::ClassifierDescriptor&>(descriptor_dispatch()); }
}  // namespace org::jetbrains::kotlin::ir::symbols
