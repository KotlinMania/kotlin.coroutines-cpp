/*
 * Copyright 2010-2025 JetBrains s.r.o. and Kotlin Programming Language contributors.
 * Use of this source code is governed by the Apache 2.0 license that can be found in the license/LICENSE.txt file.
 */
// port-lint: source compiler/ir/ir.tree/src/org/jetbrains/kotlin/ir/visitors/Deprecated.kt
// Transliterated from: compiler/ir/ir.tree/src/org/jetbrains/kotlin/ir/visitors/Deprecated.kt:1-33
#pragma once

namespace org::jetbrains::kotlin::ir::visitors {
/**
 * See [KT-75353](https://youtrack.jetbrains.com/issue/KT-75353) for an explanation
 * why this is a marker interface and not a type alias to [IrTransformer].
 */
// NOTE(port): The consumed source interface has no methods. C++ polymorphic
// destruction supplies the interface lifetime contract. Kotlin's error-level
// deprecation is suppressed by the source IrTransformer itself.
// Transliterated from: compiler/ir/ir.tree/src/org/jetbrains/kotlin/ir/visitors/Deprecated.kt:28-33
template <typename D>
class IrElementTransformer {
 public:
  virtual ~IrElementTransformer() = default;
};
}  // namespace org::jetbrains::kotlin::ir::visitors
