/*
 * Copyright 2010-2016 JetBrains s.r.o.
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
// port-lint: source core/descriptors/src/org/jetbrains/kotlin/descriptors/ValueDescriptor.java
// Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/ValueDescriptor.java:22-29
#pragma once

#include "CallableDescriptor.hpp"

namespace org::jetbrains::kotlin::descriptors {
  // Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/ValueDescriptor.java:22-29
class ValueDescriptor : public virtual CallableDescriptor {
 public:
  // Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/ValueDescriptor.java:23-24
  // NOTE(port): Preserve the source pointer and its nonnull declaration contract.
  // VariableDescriptorImpl stores a nullable type while being initialized;
  // constructing a C++ reference from that field would add undefined behavior.
  virtual types::KotlinType* get_type() const __attribute__((returns_nonnull)) = 0;
  // Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/ValueDescriptor.java:26-28
  DeclarationDescriptor* get_containing_declaration() const override
      __attribute__((returns_nonnull)) = 0;
};
}  // namespace org::jetbrains::kotlin::descriptors
