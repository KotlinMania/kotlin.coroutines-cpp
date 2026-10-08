/*
 * Copyright 2010-2024 JetBrains s.r.o. and Kotlin Programming Language contributors.
 * Use of this source code is governed by the Apache 2.0 license that can be found in the license/LICENSE.txt file.
 */
// port-lint: source compiler/ir/ir.tree/src/org/jetbrains/kotlin/ir/types/IrType.kt
// Transliterated from: compiler/ir/ir.tree/src/org/jetbrains/kotlin/ir/types/IrType.kt:16-99
#pragma once
#include "../declarations/IrAnnotationContainer.hpp"
#include "../symbols/IrClassifierSymbol.hpp"
#include "../../mpp/TypeRefMarker.hpp"
#include "../../types/model/TypeSystemContext.hpp"
#include "../../types/Variance.hpp"
#include <cstdint>

namespace kotlin { class Any; }
namespace org::jetbrains::kotlin::types { class KotlinType; }
namespace org::jetbrains::kotlin::ir::symbols { class IrClassSymbol; }
namespace org::jetbrains::kotlin::ir::types {
class IrType;
/**
 * An argument for a generic parameter. Can be either [IrTypeProjection], or [IrStarProjection].
 */
// Transliterated from: compiler/ir/ir.tree/src/org/jetbrains/kotlin/ir/types/IrType.kt:89-92
class IrTypeArgument : public virtual ::org::jetbrains::kotlin::types::model::TypeArgumentMarker {
 public:
  virtual ~IrTypeArgument() = default;
// Transliterated from: compiler/ir/ir.tree/src/org/jetbrains/kotlin/ir/types/IrType.kt:90-90
  virtual bool equals(const ::kotlin::Any* other) const = 0;
// Transliterated from: compiler/ir/ir.tree/src/org/jetbrains/kotlin/ir/types/IrType.kt:91-91
  virtual std::int32_t hash_code() const = 0;
};
// Transliterated from: compiler/ir/ir.tree/src/org/jetbrains/kotlin/ir/types/IrType.kt:94-94
class IrStarProjection : public virtual IrTypeArgument {};
// Transliterated from: compiler/ir/ir.tree/src/org/jetbrains/kotlin/ir/types/IrType.kt:96-99
class IrTypeProjection : public virtual IrTypeArgument {
 public:
// Transliterated from: compiler/ir/ir.tree/src/org/jetbrains/kotlin/ir/types/IrType.kt:97-97
  virtual ::org::jetbrains::kotlin::types::Variance variance() const = 0;
// Transliterated from: compiler/ir/ir.tree/src/org/jetbrains/kotlin/ir/types/IrType.kt:98-98
  virtual IrType& type() const = 0;
};
// NOTE(port): Actual annotation and type marker interfaces retain their identities.
// Sealed-family metadata and concrete type equality remain separate source work.
// Transliterated from: compiler/ir/ir.tree/src/org/jetbrains/kotlin/ir/types/IrType.kt:16-34
class IrType : public virtual IrTypeProjection,
               public virtual ::org::jetbrains::kotlin::types::model::KotlinTypeMarker,
               public virtual ::org::jetbrains::kotlin::mpp::TypeRefMarker,
               public virtual declarations::IrAnnotationContainer {
 public:
// Transliterated from: compiler/ir/ir.tree/src/org/jetbrains/kotlin/ir/types/IrType.kt:17-18
  IrType& type() const override final;
// Transliterated from: compiler/ir/ir.tree/src/org/jetbrains/kotlin/ir/types/IrType.kt:20-21
  virtual ::org::jetbrains::kotlin::types::KotlinType* original_kotlin_type() const;
    /**
     * @return true if this type is equal to [other] symbolically. Note that this is NOT EQUIVALENT to the full type checking algorithm
     * used in the compiler frontend. For example, this method will return `false` on the types `List<*>` and `List<Any?>`,
     * whereas the real type checker from the compiler frontend would return `true`.
     *
     * Classes are compared by FQ names, which means that even if two types refer to different symbols of the class with the same FQ name,
     * such types will be considered equal. Type annotations do not have any effect on the behavior of this method.
     */
// Transliterated from: compiler/ir/ir.tree/src/org/jetbrains/kotlin/ir/types/IrType.kt:31-31
  bool equals(const ::kotlin::Any* other) const override = 0;
// Transliterated from: compiler/ir/ir.tree/src/org/jetbrains/kotlin/ir/types/IrType.kt:33-33
  std::int32_t hash_code() const override = 0;
};
// Transliterated from: compiler/ir/ir.tree/src/org/jetbrains/kotlin/ir/types/IrType.kt:36-42
class IrErrorType : public IrType,
                    public virtual ::org::jetbrains::kotlin::types::model::SimpleTypeMarker {
 public:
// Transliterated from: compiler/ir/ir.tree/src/org/jetbrains/kotlin/ir/types/IrType.kt:36-39
  explicit IrErrorType(symbols::IrClassSymbol& error_class_stub_symbol, bool is_marked_nullable = false);
// Transliterated from: compiler/ir/ir.tree/src/org/jetbrains/kotlin/ir/types/IrType.kt:38-38
  bool is_marked_nullable() const;
// Transliterated from: compiler/ir/ir.tree/src/org/jetbrains/kotlin/ir/types/IrType.kt:40-41
  symbols::IrClassSymbol& symbol() const;
 private:
  symbols::IrClassSymbol* error_class_stub_symbol_;
  const bool is_marked_nullable_;
};
// Transliterated from: compiler/ir/ir.tree/src/org/jetbrains/kotlin/ir/types/IrType.kt:44-44
class IrDynamicType : public IrType,
                      public virtual ::org::jetbrains::kotlin::types::model::DynamicTypeMarker {};
// Transliterated from: compiler/ir/ir.tree/src/org/jetbrains/kotlin/ir/types/IrType.kt:46-54
enum class SimpleTypeNullability { MARKED_NULLABLE, NOT_SPECIFIED, DEFINITELY_NOT_NULL };
// Transliterated from: compiler/ir/ir.tree/src/org/jetbrains/kotlin/ir/types/IrType.kt:52-52
SimpleTypeNullability from_has_question_mark(bool has_question_mark);
// Transliterated from: compiler/ir/ir.tree/src/org/jetbrains/kotlin/ir/types/IrType.kt:56-84
class IrSimpleType : public IrType,
                     public virtual ::org::jetbrains::kotlin::types::model::SimpleTypeMarker,
                     public virtual ::org::jetbrains::kotlin::types::model::TypeArgumentListMarker {
 public:
// Transliterated from: compiler/ir/ir.tree/src/org/jetbrains/kotlin/ir/types/IrType.kt:57-57
  virtual symbols::IrClassifierSymbol& classifier() const = 0;
    /**
     * If type is explicitly marked as nullable, [nullability] is [SimpleTypeNullability.MARKED_NULLABLE]
     *
     * If classifier is type parameter, not marked as nullable, but can store null values,
     * if corresponding argument would be nullable, [nullability] is [SimpleTypeNullability.NOT_SPECIFIED]
     *
     * If type can't store null values, [nullability] is [SimpleTypeNullability.DEFINITELY_NOT_NULL]
     *
     * Direct usages of this property should be avoided in most cases. Use relevant util functions instead.
     *
     * In most cases one of following is needed:
     *
     * Use [IrType.isNullable] to check if null value is possible for this type
     *
     * Use [IrType.isMarkedNullable] to check if type is marked with question mark in code
     *
     * Use [IrType.mergeNullability] to apply nullability of type parameter to actual type argument in type substitutions
     *
     * Use [IrType.makeNotNull] or [IrType.makeNullable] to transfer nullability from one type to another
     */
// Transliterated from: compiler/ir/ir.tree/src/org/jetbrains/kotlin/ir/types/IrType.kt:79-79
  virtual SimpleTypeNullability nullability() const = 0;
// Transliterated from: compiler/ir/ir.tree/src/org/jetbrains/kotlin/ir/types/IrType.kt:80-80
  virtual ::kotlin::collections::List<IrTypeArgument*>& arguments() const = 0;
// Transliterated from: compiler/ir/ir.tree/src/org/jetbrains/kotlin/ir/types/IrType.kt:82-83
  ::org::jetbrains::kotlin::types::Variance variance() const override;
};
}  // namespace org::jetbrains::kotlin::ir::types
