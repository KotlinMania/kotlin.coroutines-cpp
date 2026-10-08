/*
 * Copyright 2010-2024 JetBrains s.r.o. and Kotlin Programming Language contributors.
 * Use of this source code is governed by the Apache 2.0 license that can be found in the license/LICENSE.txt file.
 */
// port-lint: source kotlin-native/runtime/src/main/kotlin/kotlin/Any.kt
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/Any.kt:18-53
#pragma once

#include "native/internal/CompilerClassInfo.hpp"
#include <cstdint>
#include <string>

namespace kotlin {
class Any;
namespace native::internal {
namespace detail { struct CompilerClassInfo; }
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/native/internal/KClassImpl.kt:72-76
const detail::CompilerClassInfo* get_object_type_info(const Any& object);
}

/**
 * The root of the Kotlin class hierarchy. Every Kotlin class has [Any] as a superclass.
 */
// NOTE(port): This is the source object contract for compiler-owned C++ objects.
// ExportTypeInfo("theAnyTypeInfo"), class literals and actual Native object layout
// require their real compiler/runtime bindings; this class supplies no metadata
// field, Native header reinterpretation or replacement reflection result.
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/Any.kt:18-53
class [[clang::annotate("kotlin.class:kotlin:Any:class")]] Any {
 public:
  Any() = default;
  virtual ~Any() = default;
  // NOTE(port): Source references keep one object identity. Compiler-owned C++
  // reference objects are not copied or relocated by value.
  Any(const Any&) = delete;
  Any& operator=(const Any&) = delete;
  Any(Any&&) = delete;
  Any& operator=(Any&&) = delete;

    /**
     * Indicates whether some other object is "equal to" this one.
     *
     * Implementations must fulfil the following requirements:
     * * Reflexive: for any non-null value `x`, `x.equals(x)` should return true.
     * * Symmetric: for any non-null values `x` and `y`, `x.equals(y)` should return true if and only if `y.equals(x)` returns true.
     * * Transitive: for any non-null values `x`, `y`, and `z`, if `x.equals(y)` returns true and `y.equals(z)` returns true, then `x.equals(z)` should return true.
     * * Consistent: for any non-null values `x` and `y`, multiple invocations of `x.equals(y)` consistently return true or consistently return false, provided no information used in `equals` comparisons on the objects is modified.
     * * Never equal to null: for any non-null value `x`, `x.equals(null)` should return false.
     *
     * Read more about [equality](https://kotlinlang.org/docs/reference/equality.html) in Kotlin.
     */
  // Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/Any.kt:31-31
  virtual bool equals(const Any* other) const;

    /**
     * Returns a hash code value for the object.
     *
     * The general contract of `hashCode` is:
     * * Whenever it is invoked on the same object more than once, the `hashCode` method must consistently return the same integer, provided no information used in `equals` comparisons on the object is modified.
     * * If two objects are equal according to the `equals()` method, then calling the `hashCode` method on each of the two objects must produce the same integer result.
     */
  // Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/Any.kt:41-41
  virtual std::int32_t hash_code() const;

    /**
     * Returns a string representation of the object.
     */
  // Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/Any.kt:46-52
  virtual std::u16string to_string() const;
 private:
  // NOTE(port): Compiler-only metadata intrinsic supplied by the Clang binding.
  // It returns the actual C++ declaration's data, not a Native object header.
  // Transliterated from: kotlin-native/runtime/src/main/cpp/Natives.cpp:110-112
  virtual const native::internal::detail::CompilerClassInfo*
  __kxs_compiler_type_info() const;
  friend const native::internal::detail::CompilerClassInfo*
  native::internal::get_object_type_info(const Any& object);
};
}  // namespace kotlin
