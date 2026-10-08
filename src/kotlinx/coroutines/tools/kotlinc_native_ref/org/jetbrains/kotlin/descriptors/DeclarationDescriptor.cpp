/*
 * Copyright 2010-2015 JetBrains s.r.o.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
// port-lint: source core/descriptors/src/org/jetbrains/kotlin/descriptors/DeclarationDescriptor.java
// Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/DeclarationDescriptor.java:24-40
#include "DeclarationDescriptor.hpp"

#include <bit>
#include <functional>
#include <sstream>
#include <typeinfo>

namespace org::jetbrains::kotlin::descriptors {

// NOTE(port): Implement inherited JVM Object identity for the C++ compiler
// object. Derived descriptor views override equality using their actual owners.
// Transliterated from: libraries/stdlib/jvm/builtins/Any.kt:28-28
bool DeclarationDescriptor::equals(const std::any& other) const {
  const auto* descriptor = std::any_cast<std::reference_wrapper<const DeclarationDescriptor>>(&other);
  return descriptor != nullptr && this == &descriptor->get();
}

// NOTE(port): C++ compiler identities use stable object addresses. This satisfies
// the inherited hash contract; Kotlin/Native identityHashCode remains a separate
// runtime dependency and is not claimed by this compiler-object implementation.
// Transliterated from: libraries/stdlib/jvm/builtins/Any.kt:37-37
std::int32_t DeclarationDescriptor::hash_code() const {
  const auto bits = static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(this));
  return std::bit_cast<std::int32_t>(bits);
}

// NOTE(port): JVM Object's class@identity diagnostics use C++ RTTI spelling and
// the compiler object's identity hash. Descriptor rendering overrides this.
// Transliterated from: libraries/stdlib/jvm/builtins/Any.kt:42-42
std::string DeclarationDescriptor::to_string() const {
  std::ostringstream result;
  result << typeid(*this).name() << '@' << std::hex << static_cast<std::uint32_t>(hash_code());
  return result.str();
}

// NOTE(port): C++ structural equality calls the inherited source equals contract.
// Transliterated from: libraries/stdlib/jvm/builtins/Any.kt:28-28
bool DeclarationDescriptor::operator==(const DeclarationDescriptor& other) const {
  return equals(std::cref(other));
}

}  // namespace org::jetbrains::kotlin::descriptors
