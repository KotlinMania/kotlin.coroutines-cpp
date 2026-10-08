/*
 * Copyright 2010-2024 JetBrains s.r.o. and Kotlin Programming Language contributors.
 * Use of this source code is governed by the Apache 2.0 license that can be found in the license/LICENSE.txt file.
 */
// port-lint: source compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/expressions/IrBody.kt
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/expressions/IrBody.kt:18-21
#pragma once
#include "../IrElementBase.hpp"
namespace org::jetbrains::kotlin::ir::expressions {
/**
 * Generated from: [org.jetbrains.kotlin.ir.generator.IrTree.body]
 */
// NOTE(port): Source sealed-family metadata remains separate compiler work.
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/expressions/IrBody.kt:18-21
class IrBody : public IrElementBase {
 public:
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/expressions/IrBody.kt:19-20
  template <typename D>
  IrBody& transform(visitors::IrTransformer<D>& transformer, D data) {
    return dynamic_cast<IrBody&>(IrElement::transform(transformer, data));
  }
 protected:
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/expressions/IrBody.kt:19-20
  void transform_dispatch(visitors::detail::IrTransformerDispatch& dispatch) override;
};
}  // namespace org::jetbrains::kotlin::ir::expressions
