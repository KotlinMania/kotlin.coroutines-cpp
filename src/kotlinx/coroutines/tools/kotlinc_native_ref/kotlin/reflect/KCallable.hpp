/*
 * Copyright 2010-2018 JetBrains s.r.o. Use of this source code is governed by the Apache 2.0 license
 * that can be found in the LICENSE file.
 */
// port-lint: source kotlin-native/runtime/src/main/kotlin/kotlin/reflect/KCallable.kt
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/reflect/KCallable.kt:13-29
#pragma once

#include "KAnnotatedElement.hpp"
#include "KType.hpp"
#include "../collections/CollectionElement.hpp"
#include <string>

namespace kotlin::reflect {
template <typename R> class KCallable;
namespace detail {
// NOTE(port): Source R is covariant but neither property mentions R. One
// abstract virtual boundary retains the actual name and type identity across
// the typed source interfaces, without per-type data or property wrappers.
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/reflect/KCallable.kt:13-29
class KCallableObject : public virtual KAnnotatedElement {
 public:
    /**
     * The name of this callable as it was declared in the source code.
     * If the callable has no name, a special invented name is created.
     * Nameless callables include:
     * - constructors have the name "<init>",
     * - property accessors: the getter for a property named "foo" will have the name "<get-foo>",
     *   the setter, similarly, will have the name "<set-foo>".
     */
  // Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/reflect/KCallable.kt:22-23
  [[clang::annotate("kotlin.internal.IntrinsicConstEvaluation")]]
  virtual const std::u16string& name() const = 0;
    /**
     * The type of values returned by this callable.
     */
  // Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/reflect/KCallable.kt:28-28
  virtual KType& return_type() const = 0;
 protected:
  KCallableObject() = default;
};
}  // namespace detail
/**
 * Represents a callable entity, such as a function or a property.
 *
 * @param R return type of the callable.
 */
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/reflect/KCallable.kt:13-29
template <typename R>
class KCallable : public virtual detail::KCallableObject,
                  public ::kotlin::collections::detail::CovariantBases<
                      KCallable, typename ::kotlin::collections::detail::ElementSupertypes<R>::Types> {
 protected:
  KCallable() = default;
};
}  // namespace kotlin::reflect
