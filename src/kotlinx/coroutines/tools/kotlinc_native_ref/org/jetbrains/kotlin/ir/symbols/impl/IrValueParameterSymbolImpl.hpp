/*
 * Copyright 2010-2024 JetBrains s.r.o. and Kotlin Programming Language contributors.
 * Use of this source code is governed by the Apache 2.0 license that can be found in the license/LICENSE.txt file.
 */
// port-lint: source compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/symbols/impl/IrSymbolImpl.kt
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/symbols/impl/IrSymbolImpl.kt:66-69
// port-lint: source compiler/ir/ir.tree/src/org/jetbrains/kotlin/ir/symbols/IrSymbol.kt
#pragma once
#include "IrSymbolImpl.hpp"
#include "../IrValueParameterSymbol.hpp"
namespace org::jetbrains::kotlin::ir::symbols::impl {
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/symbols/impl/IrSymbolImpl.kt:66-69
class IrValueParameterSymbolImpl final : public IrSymbolWithSignature<::org::jetbrains::kotlin::descriptors::ParameterDescriptor, declarations::IrValueParameter>, public virtual IrValueParameterSymbol {
 public:
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/symbols/impl/IrSymbolImpl.kt:66-69
  explicit IrValueParameterSymbolImpl(::org::jetbrains::kotlin::descriptors::ParameterDescriptor* descriptor = nullptr, util::IdSignature* signature = nullptr);
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/symbols/impl/IrSymbolImpl.kt:66-69
// Transliterated from: compiler/ir/ir.tree/src/org/jetbrains/kotlin/ir/symbols/IrSymbol.kt:126-127
  declarations::IrValueParameter& owner() const;
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/symbols/impl/IrSymbolImpl.kt:66-69
// Transliterated from: compiler/ir/ir.tree/src/org/jetbrains/kotlin/ir/symbols/IrSymbol.kt:129-130
  ::org::jetbrains::kotlin::descriptors::ParameterDescriptor& descriptor() const;
};
}  // namespace org::jetbrains::kotlin::ir::symbols::impl
