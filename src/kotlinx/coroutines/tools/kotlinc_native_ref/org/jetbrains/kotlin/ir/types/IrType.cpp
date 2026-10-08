/*
 * Copyright 2010-2024 JetBrains s.r.o. and Kotlin Programming Language contributors.
 * Use of this source code is governed by the Apache 2.0 license that can be found in the license/LICENSE.txt file.
 */
// port-lint: source compiler/ir/ir.tree/src/org/jetbrains/kotlin/ir/types/IrType.kt
// Transliterated from: compiler/ir/ir.tree/src/org/jetbrains/kotlin/ir/types/IrType.kt:16-99
#include "IrType.hpp"

namespace org::jetbrains::kotlin::ir::types {
// NOTE(port): A const C++ accessor borrows the same mutable source type object.
// Transliterated from: compiler/ir/ir.tree/src/org/jetbrains/kotlin/ir/types/IrType.kt:17-18
IrType& IrType::type() const { return const_cast<IrType&>(*this); }
// Transliterated from: compiler/ir/ir.tree/src/org/jetbrains/kotlin/ir/types/IrType.kt:20-21
::org::jetbrains::kotlin::types::KotlinType* IrType::original_kotlin_type() const { return nullptr; }
// Transliterated from: compiler/ir/ir.tree/src/org/jetbrains/kotlin/ir/types/IrType.kt:36-39
IrErrorType::IrErrorType(symbols::IrClassSymbol& error_class_stub_symbol, bool is_marked_nullable)
    : error_class_stub_symbol_(&error_class_stub_symbol), is_marked_nullable_(is_marked_nullable) {}
// Transliterated from: compiler/ir/ir.tree/src/org/jetbrains/kotlin/ir/types/IrType.kt:38-38
bool IrErrorType::is_marked_nullable() const { return is_marked_nullable_; }
// Transliterated from: compiler/ir/ir.tree/src/org/jetbrains/kotlin/ir/types/IrType.kt:40-41
symbols::IrClassSymbol& IrErrorType::symbol() const { return *error_class_stub_symbol_; }
// Transliterated from: compiler/ir/ir.tree/src/org/jetbrains/kotlin/ir/types/IrType.kt:52-52
SimpleTypeNullability from_has_question_mark(bool has_question_mark) {
  return has_question_mark ? SimpleTypeNullability::MARKED_NULLABLE : SimpleTypeNullability::NOT_SPECIFIED;
}
// Transliterated from: compiler/ir/ir.tree/src/org/jetbrains/kotlin/ir/types/IrType.kt:82-83
::org::jetbrains::kotlin::types::Variance IrSimpleType::variance() const {
  return ::org::jetbrains::kotlin::types::Variance::INVARIANT;
}
}  // namespace org::jetbrains::kotlin::ir::types
