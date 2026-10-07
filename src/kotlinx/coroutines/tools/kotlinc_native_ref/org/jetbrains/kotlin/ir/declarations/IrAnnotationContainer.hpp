/*
 * Copyright 2010-2024 JetBrains s.r.o. and Kotlin Programming Language contributors.
 * Use of this source code is governed by the Apache 2.0 license that can be found in the license/LICENSE.txt file.
 */
// port-lint: source compiler/ir/ir.tree/src/org/jetbrains/kotlin/ir/declarations/IrAnnotationContainer.kt
// Transliterated from: compiler/ir/ir.tree/src/org/jetbrains/kotlin/ir/declarations/IrAnnotationContainer.kt:10-12
#pragma once

namespace kotlin::collections { template <typename E> class List; }
namespace org::jetbrains::kotlin::ir::expressions { class IrAnnotation; }
namespace org::jetbrains::kotlin::ir::declarations {
// NOTE(port): The getter borrows the compiler-owned source collection object.
// Transliterated from: compiler/ir/ir.tree/src/org/jetbrains/kotlin/ir/declarations/IrAnnotationContainer.kt:10-12
class IrAnnotationContainer {
 public:
  virtual ~IrAnnotationContainer() = default;
  // Transliterated from: compiler/ir/ir.tree/src/org/jetbrains/kotlin/ir/declarations/IrAnnotationContainer.kt:11-11
  virtual ::kotlin::collections::List<expressions::IrAnnotation*>& annotations() const = 0;
};
}  // namespace org::jetbrains::kotlin::ir::declarations
