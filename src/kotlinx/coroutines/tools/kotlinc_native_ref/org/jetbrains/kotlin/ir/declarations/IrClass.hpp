/*
 * Copyright 2010-2024 JetBrains s.r.o. and Kotlin Programming Language contributors.
 * Use of this source code is governed by the Apache 2.0 license that can be found in the license/LICENSE.txt file.
 */
// port-lint: source compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/IrClass.kt
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/IrClass.kt:24-85
#pragma once
#include "IrDeclarationBase.hpp"
#include "IrPossiblyExternalDeclaration.hpp"
#include "IrDeclarationWithVisibility.hpp"
#include "IrTypeParametersContainer.hpp"
#include "IrDeclarationContainer.hpp"
#include "IrMetadataSourceOwner.hpp"
#include "../../descriptors/ClassDescriptor.hpp"
#include "../../descriptors/SourceElement.hpp"
#include "../types/IrType.hpp"
#include <memory>
namespace org::jetbrains::kotlin::ir::symbols { class IrClassSymbol; }
namespace org::jetbrains::kotlin::ir::declarations {
class IrValueParameter;
/**
 * Generated from: [org.jetbrains.kotlin.ir.generator.IrTree.class]
 */
// NOTE(port): Class identity belongs to the compiler; no Native ObjHeader is
// implied. Lists retain their actual shared C++ collection; contained nodes and
// symbols borrow the compiler-owned objects, matching the existing IR contracts.
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/IrClass.kt:24-85
class IrClass : public IrDeclarationBase, public virtual IrPossiblyExternalDeclaration, public virtual IrDeclarationWithVisibility, public virtual IrTypeParametersContainer, public virtual IrDeclarationContainer, public virtual IrMetadataSourceOwner {
 public:
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/IrClass.kt:25-26
  ::org::jetbrains::kotlin::descriptors::ClassDescriptor& descriptor() const override = 0;
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/IrClass.kt:28-28
  symbols::IrClassSymbol& symbol() const;
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/IrClass.kt:30-30
  virtual ::org::jetbrains::kotlin::descriptors::ClassKind kind() const = 0;
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/IrClass.kt:30-30
  virtual void set_kind(::org::jetbrains::kotlin::descriptors::ClassKind value) = 0;
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/IrClass.kt:32-32
  virtual ::org::jetbrains::kotlin::descriptors::Modality modality() const = 0;
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/IrClass.kt:32-32
  virtual void set_modality(::org::jetbrains::kotlin::descriptors::Modality value) = 0;
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/IrClass.kt:34-34
  virtual bool is_companion() const = 0;
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/IrClass.kt:34-34
  virtual void set_is_companion(bool value) = 0;
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/IrClass.kt:36-36
  virtual bool is_inner() const = 0;
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/IrClass.kt:36-36
  virtual void set_is_inner(bool value) = 0;
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/IrClass.kt:38-38
  virtual bool is_data() const = 0;
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/IrClass.kt:38-38
  virtual void set_is_data(bool value) = 0;
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/IrClass.kt:40-40
  virtual bool is_value() const = 0;
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/IrClass.kt:40-40
  virtual void set_is_value(bool value) = 0;
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/IrClass.kt:42-42
  virtual bool is_expect() const = 0;
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/IrClass.kt:42-42
  virtual void set_is_expect(bool value) = 0;
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/IrClass.kt:44-44
  virtual bool is_fun() const = 0;
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/IrClass.kt:44-44
  virtual void set_is_fun(bool value) = 0;
/**
     * Returns true iff this is a class loaded from dependencies which has the `HAS_ENUM_ENTRIES` metadata flag set.
     * This flag is useful for Kotlin/JVM to determine whether an enum class from dependency actually has the `entries` property
     * in its bytecode, as opposed to whether it has it in its member scope, which is true even for enum classes compiled by
     * old versions of Kotlin which did not support the EnumEntries language feature.
     */
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/IrClass.kt:52-52
  virtual bool has_enum_entries() const = 0;
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/IrClass.kt:52-52
  virtual void set_has_enum_entries(bool value) = 0;
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/IrClass.kt:54-54
  virtual ::org::jetbrains::kotlin::descriptors::SourceElement& source() const = 0;
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/IrClass.kt:56-56
  virtual std::shared_ptr<::kotlin::collections::List<types::IrType*>> super_types() const = 0;
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/IrClass.kt:56-56
  virtual void set_super_types(std::shared_ptr<::kotlin::collections::List<types::IrType*>> value) = 0;
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/IrClass.kt:58-58
  virtual IrValueParameter* this_receiver() const = 0;
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/IrClass.kt:58-58
  virtual void set_this_receiver(IrValueParameter* value) = 0;
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/IrClass.kt:60-60
  virtual ::org::jetbrains::kotlin::descriptors::ValueClassRepresentation<types::IrSimpleType>* value_class_representation() const = 0;
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/IrClass.kt:60-60
  virtual void set_value_class_representation(::org::jetbrains::kotlin::descriptors::ValueClassRepresentation<types::IrSimpleType>* value) = 0;
/**
     * If this is a sealed class or interface, this list contains symbols of all its immediate subclasses.
     * Otherwise, this is an empty list.
     *
     * NOTE: If this [IrClass] was deserialized from a klib, this list will always be empty!
     * See [KT-54028](https://youtrack.jetbrains.com/issue/KT-54028).
     */
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/IrClass.kt:69-69
  virtual std::shared_ptr<::kotlin::collections::List<symbols::IrClassSymbol*>> sealed_subclasses() const = 0;
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/IrClass.kt:69-69
  virtual void set_sealed_subclasses(std::shared_ptr<::kotlin::collections::List<symbols::IrClassSymbol*>> value) = 0;
 protected:
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/IrClass.kt:71-72
  void accept_dispatch(visitors::detail::IrVisitorDispatch& dispatch) override;
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/IrClass.kt:74-78
  void accept_children_dispatch(visitors::detail::IrVisitorDispatch& dispatch) override;
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/IrClass.kt:80-84
  void transform_children_dispatch(visitors::detail::IrTransformerDispatch& dispatch) override;
};
}  // namespace org::jetbrains::kotlin::ir::declarations
