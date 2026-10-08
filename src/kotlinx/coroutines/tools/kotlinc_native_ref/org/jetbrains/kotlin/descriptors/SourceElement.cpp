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
// port-lint: source core/compiler.common/src/org/jetbrains/kotlin/descriptors/SourceElement.java
// Transliterated from: core/compiler.common/src/org/jetbrains/kotlin/descriptors/SourceElement.java:21-37
#include "SourceElement.hpp"

#include <bit>
#include <functional>
#include <sstream>
#include <typeinfo>

namespace org::jetbrains::kotlin::descriptors {

namespace {

// Transliterated from: core/compiler.common/src/org/jetbrains/kotlin/descriptors/SourceElement.java:22-33
class NoSourceElement final : public SourceElement {
 public:
  // Transliterated from: core/compiler.common/src/org/jetbrains/kotlin/descriptors/SourceElement.java:23-26
  std::string to_string() const override { return "NO_SOURCE"; }
  // Transliterated from: core/compiler.common/src/org/jetbrains/kotlin/descriptors/SourceElement.java:28-32
  const SourceFile& get_containing_file() const override { return SourceFile::NO_SOURCE_FILE; }
};

// NOTE(port): Static storage retains the source singleton for compiler lifetime.
NoSourceElement no_source_element;

}  // namespace

// Transliterated from: core/compiler.common/src/org/jetbrains/kotlin/descriptors/SourceElement.java:22-33
SourceElement& SourceElement::NO_SOURCE = no_source_element;

// NOTE(port): Inherited Object operations use the same compiler-object
// adaptation as DeclarationDescriptor; derived source overrides remain virtual.
// Transliterated from: libraries/stdlib/jvm/builtins/Any.kt:28-28
bool SourceElement::equals(const std::any& other) const {
  const auto* object = std::any_cast<std::reference_wrapper<const SourceElement>>(&other);
  return object != nullptr && this == &object->get();
}

// Transliterated from: libraries/stdlib/jvm/builtins/Any.kt:37-37
std::int32_t SourceElement::hash_code() const {
  const auto bits = static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(this));
  return std::bit_cast<std::int32_t>(bits);
}

// Transliterated from: libraries/stdlib/jvm/builtins/Any.kt:42-42
std::string SourceElement::to_string() const {
  std::ostringstream result;
  result << typeid(*this).name() << '@' << std::hex << static_cast<std::uint32_t>(hash_code());
  return result.str();
}

}  // namespace org::jetbrains::kotlin::descriptors
