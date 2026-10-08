/*
 * Copyright 2010-2017 JetBrains s.r.o.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
// port-lint: source compiler/ir/ir.tree/src/org/jetbrains/kotlin/ir/symbols/IrSymbol.kt
// Transliterated from: compiler/ir/ir.tree/src/org/jetbrains/kotlin/ir/symbols/IrSymbol.kt:36-138
#include "IrSymbol.hpp"

namespace org::jetbrains::kotlin::ir::symbols {

// NOTE(port): C++ typed getter boundary for the source abstract owner property.
// Transliterated from: compiler/ir/ir.tree/src/org/jetbrains/kotlin/ir/symbols/IrSymbol.kt:56-67
declarations::IrSymbolOwner& IrSymbol::owner() const {
  return owner_dispatch();
}

// Transliterated from: compiler/ir/ir.tree/src/org/jetbrains/kotlin/ir/symbols/IrSymbol.kt:69-74
::org::jetbrains::kotlin::descriptors::DeclarationDescriptor& IrSymbol::descriptor() const {
  return descriptor_dispatch();
}

// Transliterated from: compiler/ir/ir.tree/src/org/jetbrains/kotlin/ir/symbols/IrSymbol.kt:110-116
bool is_public_api(const IrSymbol& symbol) { return symbol.signature() != nullptr; }

}  // namespace org::jetbrains::kotlin::ir::symbols
