/*
 * Copyright 2010-2024 JetBrains s.r.o. and Kotlin Programming Language contributors.
 * Use of this source code is governed by the Apache 2.0 license that can be found in the license/LICENSE.txt file.
 */
// port-lint: source compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/IrMutableAnnotationContainer.kt
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/IrMutableAnnotationContainer.kt:17-19
#pragma once
#include "../IrElement.hpp"
#include "IrAnnotationContainer.hpp"

namespace org::jetbrains::kotlin::ir::declarations {
/**
 * Generated from: [org.jetbrains.kotlin.ir.generator.IrTree.mutableAnnotationContainer]
 */
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/IrMutableAnnotationContainer.kt:17-19
class IrMutableAnnotationContainer : public virtual IrElement, public virtual IrAnnotationContainer {
 public:
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/IrMutableAnnotationContainer.kt:18-18
  virtual ::kotlin::collections::List<expressions::IrAnnotation*>& annotations() const override = 0;
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/IrMutableAnnotationContainer.kt:18-18
  virtual void set_annotations(::kotlin::collections::List<expressions::IrAnnotation*>& value) = 0;
};
}  // namespace org::jetbrains::kotlin::ir::declarations
