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
// port-lint: source core/descriptors/src/org/jetbrains/kotlin/descriptors/DeclarationDescriptorNonRoot.java
// Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/DeclarationDescriptorNonRoot.java:21-27
#pragma once

#include "DeclarationDescriptorWithSource.hpp"

namespace org::jetbrains::kotlin::descriptors {

// Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/DeclarationDescriptorNonRoot.java:21-27
class DeclarationDescriptorNonRoot : public virtual DeclarationDescriptorWithSource {
 public:
  // NOTE(port): Keep the pointer return required by the nullable base override.
  // Source @NotNull is retained as Clang's nonnull return contract; no proof guard.
  // Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/DeclarationDescriptorNonRoot.java:23-25
  DeclarationDescriptor* get_containing_declaration() const override
      __attribute__((returns_nonnull)) = 0;
};

}  // namespace org::jetbrains::kotlin::descriptors
