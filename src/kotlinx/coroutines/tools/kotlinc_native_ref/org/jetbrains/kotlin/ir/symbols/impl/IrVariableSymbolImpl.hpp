/*
 * Copyright 2010-2024 JetBrains s.r.o. and Kotlin Programming Language contributors.
 * Use of this source code is governed by the Apache 2.0 license that can be found in the license/LICENSE.txt file.
 */
// port-lint: source compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/symbols/impl/IrSymbolImpl.kt
// port-lint: source compiler/ir/ir.tree/src/org/jetbrains/kotlin/ir/symbols/IrSymbol.kt
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/symbols/impl/IrSymbolImpl.kt:71-73
#pragma once

#include "IrSymbolImpl.hpp"
#include "../IrVariableSymbol.hpp"

namespace org::jetbrains::kotlin::ir::symbols::impl {
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/symbols/impl/IrSymbolImpl.kt:71-73
class IrVariableSymbolImpl final
    : public IrSymbolBase<::org::jetbrains::kotlin::descriptors::VariableDescriptor,
                          declarations::IrVariable>,
      public virtual IrVariableSymbol {
 public:
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/symbols/impl/IrSymbolImpl.kt:71-73
  explicit IrVariableSymbolImpl(::org::jetbrains::kotlin::descriptors::VariableDescriptor* descriptor = nullptr);
  // NOTE(port): Resolve inherited typed access without another binding field.
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/symbols/impl/IrSymbolImpl.kt:71-73
  // Transliterated from: compiler/ir/ir.tree/src/org/jetbrains/kotlin/ir/symbols/IrSymbol.kt:126-127
  declarations::IrVariable& owner() const;
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/symbols/impl/IrSymbolImpl.kt:71-73
  // Transliterated from: compiler/ir/ir.tree/src/org/jetbrains/kotlin/ir/symbols/IrSymbol.kt:129-130
  ::org::jetbrains::kotlin::descriptors::VariableDescriptor& descriptor() const;
};
}  // namespace org::jetbrains::kotlin::ir::symbols::impl
