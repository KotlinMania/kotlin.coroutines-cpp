/*
 * Copyright 2010-2018 JetBrains s.r.o. Use of this source code is governed by the Apache 2.0 license
 * that can be found in the LICENSE file.
 */
// port-lint: source kotlin-native/runtime/src/main/kotlin/kotlin/reflect/KProperty.kt
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/reflect/KProperty.kt:17-17
#pragma once

#include "KCallable.hpp"

namespace kotlin::reflect {
/**
 * Represents a property, such as a named `val` or `var` declaration.
 * Instances of this class are obtainable by the `::` operator.
 *
 * See the [Kotlin language documentation](https://kotlinlang.org/docs/reference/reflection.html)
 * for more information.
 *
 * @param V the type of the property value.
 */
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/reflect/KProperty.kt:17-17
template <typename V>
class KProperty : public virtual KCallable<V>,
                  public ::kotlin::collections::detail::CovariantBases<
                      KProperty, typename ::kotlin::collections::detail::ElementSupertypes<V>::Types> {
 protected:
  // NOTE(port): The consumed Native base adds no own methods. Parameterized
  // and mutable property families are separate actual source contracts.
  KProperty() = default;
};
}  // namespace kotlin::reflect
