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
// port-lint: source core/compiler.common/src/org/jetbrains/kotlin/descriptors/SourceFile.java
// Transliterated from: core/compiler.common/src/org/jetbrains/kotlin/descriptors/SourceFile.java:21-32
#include "SourceFile.hpp"

#include <bit>
#include <functional>
#include <sstream>
#include <typeinfo>

namespace org::jetbrains::kotlin::descriptors {

namespace {

// Transliterated from: core/compiler.common/src/org/jetbrains/kotlin/descriptors/SourceFile.java:22-28
class NoSourceFile final : public SourceFile {
 public:
  // Transliterated from: core/compiler.common/src/org/jetbrains/kotlin/descriptors/SourceFile.java:23-27
  std::optional<std::u16string> get_name() const override { return std::nullopt; }
};

// NOTE(port): Static storage retains the source singleton for compiler lifetime.
NoSourceFile no_source_file;

}  // namespace

// Transliterated from: core/compiler.common/src/org/jetbrains/kotlin/descriptors/SourceFile.java:22-28
SourceFile& SourceFile::NO_SOURCE_FILE = no_source_file;

// NOTE(port): Inherited Object operations use the same compiler-object
// adaptation as DeclarationDescriptor; derived source overrides remain virtual.
// Transliterated from: libraries/stdlib/jvm/builtins/Any.kt:28-28
bool SourceFile::equals(const std::any& other) const {
  const auto* object = std::any_cast<std::reference_wrapper<const SourceFile>>(&other);
  return object != nullptr && this == &object->get();
}

// Transliterated from: libraries/stdlib/jvm/builtins/Any.kt:37-37
std::int32_t SourceFile::hash_code() const {
  const auto bits = static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(this));
  return std::bit_cast<std::int32_t>(bits);
}

// Transliterated from: libraries/stdlib/jvm/builtins/Any.kt:42-42
std::string SourceFile::to_string() const {
  std::ostringstream result;
  result << typeid(*this).name() << '@' << std::hex << static_cast<std::uint32_t>(hash_code());
  return result.str();
}

}  // namespace org::jetbrains::kotlin::descriptors
