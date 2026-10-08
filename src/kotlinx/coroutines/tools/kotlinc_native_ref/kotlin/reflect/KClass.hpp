/*
 * Copyright 2010-2018 JetBrains s.r.o. Use of this source code is governed by the Apache 2.0 license
 * that can be found in the LICENSE file.
 */
// port-lint: source kotlin-native/runtime/src/main/kotlin/kotlin/reflect/KClass.kt
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/reflect/KClass.kt:16-49
#pragma once

#include "../Any.hpp"
#include "KDeclarationContainer.hpp"
#include "KAnnotatedElement.hpp"
#include "KClassifier.hpp"
#include <optional>
#include <string>
#include <type_traits>

namespace kotlin::reflect {
template <typename T> class KClass;
namespace detail {
// NOTE(port): Source KClass<*> operations use one genuine abstract projection.
// Only the actual invariant KClass<T> interface can construct this base; no
// object, metadata, equality implementation or alternate class is supplied.
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/reflect/KClass.kt:16-49
class KClassObject : public virtual ::kotlin::Any {
 public:
    /**
     * The simple name of the class as it was declared in the source code,
     * or `null` if the class has no name (if, for example, it is a class of an anonymous object).
     */
  // Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/reflect/KClass.kt:21-21
  virtual std::optional<std::u16string> simple_name() const = 0;

    /**
     * The fully qualified dot-separated name of the class,
     * or `null` if the class is local or a class of an anonymous object.
     */
  // Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/reflect/KClass.kt:27-27
  virtual std::optional<std::u16string> qualified_name() const = 0;

    /**
     * Returns `true` if [value] is an instance of this class on a given platform.
     */
  // Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/reflect/KClass.kt:33-33
  virtual bool is_instance(const ::kotlin::Any* value) const = 0;

    /**
     * Returns `true` if this [KClass] instance represents the same Kotlin class as the class represented by [other].
     * On JVM this means that all of the following conditions are satisfied:
     *
     * 1. [other] has the same (fully qualified) Kotlin class name as this instance.
     * 2. [other]'s backing [Class] object is loaded with the same class loader as the [Class] object of this instance.
     * 3. If the classes represent [Array], then [Class] objects of their element types are equal.
     *
     * For example, on JVM, [KClass] instances for a primitive type (`int`) and the corresponding wrapper type (`java.lang.Integer`)
     * are considered equal, because they have the same fully qualified name "kotlin.Int".
     */
  // Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/reflect/KClass.kt:46-46
  bool equals(const ::kotlin::Any* other) const override = 0;  // KT-24971
  // Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/reflect/KClass.kt:48-48
  std::int32_t hash_code() const override = 0;  // KT-24971
 private:
  KClassObject() = default;
  template <typename> friend class ::kotlin::reflect::KClass;
};
}  // namespace detail

/**
 * Represents a class and provides introspection capabilities.
 * Instances of this class are obtainable by the `::class` syntax.
 * See the [Kotlin language documentation](https://kotlinlang.org/docs/reference/reflection.html#class-references)
 * for more information.
 *
 * @param T the type of the class.
 */
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/reflect/KClass.kt:16-49
template <typename T>
class [[clang::annotate("kotlin.class:kotlin.reflect:KClass:interface")]] KClass : public detail::KClassObject,
               public virtual KDeclarationContainer,
               public virtual KAnnotatedElement,
               public virtual KClassifier {
  static_assert(std::is_base_of_v<::kotlin::Any, T>);
 protected:
  KClass() = default;
};
}  // namespace kotlin::reflect
