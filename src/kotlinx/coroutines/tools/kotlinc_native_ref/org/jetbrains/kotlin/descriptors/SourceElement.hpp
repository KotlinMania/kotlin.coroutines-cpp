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
#pragma once

#include <any>
#include <cstdint>
#include <string>
#include "SourceFile.hpp"

namespace org::jetbrains::kotlin::descriptors {

// Transliterated from: core/compiler.common/src/org/jetbrains/kotlin/descriptors/SourceElement.java:21-37
class SourceElement {
 public:
  virtual ~SourceElement() = default;
  // NOTE(port): Source static final object is a stable borrowed singleton.
  // Transliterated from: core/compiler.common/src/org/jetbrains/kotlin/descriptors/SourceElement.java:22-33
  static SourceElement& NO_SOURCE;

  // Transliterated from: core/compiler.common/src/org/jetbrains/kotlin/descriptors/SourceElement.java:35-36
  virtual const SourceFile& get_containing_file() const = 0;

  // NOTE(port): Inherited JVM Object methods use C++ compiler identity and
  // RTTI diagnostics. Canonical object boxes borrow this interface; this does
  // not implement Kotlin/Native runtime object layout or identityHashCode.
  // Transliterated from: libraries/stdlib/jvm/builtins/Any.kt:28-28
  virtual bool equals(const std::any& other) const;
  // Transliterated from: libraries/stdlib/jvm/builtins/Any.kt:37-37
  virtual std::int32_t hash_code() const;
  // Transliterated from: libraries/stdlib/jvm/builtins/Any.kt:42-42
  virtual std::string to_string() const;

 protected:
  SourceElement() = default;
  // NOTE(port): JVM source objects retain identity instead of copying.
  SourceElement(const SourceElement&) = delete;
  SourceElement& operator=(const SourceElement&) = delete;
};

}  // namespace org::jetbrains::kotlin::descriptors
