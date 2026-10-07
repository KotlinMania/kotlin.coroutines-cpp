/*
 * Copyright 2010-2024 JetBrains s.r.o. and Kotlin Programming Language contributors.
 * Use of this source code is governed by the Apache 2.0 license that can be found in the license/LICENSE.txt file.
 */
// port-lint: source compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/IrTypeParameter.kt
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/IrTypeParameter.kt:22-41
#pragma once
#include "IrDeclarationBase.hpp"
#include "IrDeclarationWithName.hpp"
#include "../../descriptors/TypeParameterDescriptor.hpp"
#include "../types/IrType.hpp"
#include "../../types/Variance.hpp"
namespace org::jetbrains::kotlin::ir::symbols { class IrTypeParameterSymbol; }
namespace org::jetbrains::kotlin::ir::declarations {
/**
 * Generated from: [org.jetbrains.kotlin.ir.generator.IrTree.typeParameter]
 */
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/IrTypeParameter.kt:22-41
class IrTypeParameter : public IrDeclarationBase, public virtual IrDeclarationWithName {
 public:
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/IrTypeParameter.kt:23-24
  ::org::jetbrains::kotlin::descriptors::TypeParameterDescriptor& descriptor() const override = 0;
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/IrTypeParameter.kt:26-26
  symbols::IrTypeParameterSymbol& symbol() const;
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/IrTypeParameter.kt:28-28
  virtual ::org::jetbrains::kotlin::types::Variance variance() const = 0;
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/IrTypeParameter.kt:28-28
  virtual void set_variance(::org::jetbrains::kotlin::types::Variance value) = 0;
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/IrTypeParameter.kt:30-30
  virtual std::int32_t index() const = 0;
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/IrTypeParameter.kt:30-30
  virtual void set_index(std::int32_t value) = 0;
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/IrTypeParameter.kt:32-32
  virtual bool is_reified() const = 0;
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/IrTypeParameter.kt:32-32
  virtual void set_is_reified(bool value) = 0;
  // NOTE(port): Borrow the actual compiler-owned list, as existing IR type contracts do.
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/IrTypeParameter.kt:34-34
  virtual ::kotlin::collections::List<types::IrType*>& super_types() const = 0;
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/IrTypeParameter.kt:34-34
  virtual void set_super_types(::kotlin::collections::List<types::IrType*>& value) = 0;
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/IrTypeParameter.kt:39-40
  template <typename D>
  IrTypeParameter& transform(visitors::IrTransformer<D>& transformer, D data) {
    return dynamic_cast<IrTypeParameter&>(IrElement::transform(transformer, data));
  }
 protected:
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/IrTypeParameter.kt:36-37
  void accept_dispatch(visitors::detail::IrVisitorDispatch& dispatch) override;
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/IrTypeParameter.kt:39-40
  void transform_dispatch(visitors::detail::IrTransformerDispatch& dispatch) override;
};
}  // namespace org::jetbrains::kotlin::ir::declarations
