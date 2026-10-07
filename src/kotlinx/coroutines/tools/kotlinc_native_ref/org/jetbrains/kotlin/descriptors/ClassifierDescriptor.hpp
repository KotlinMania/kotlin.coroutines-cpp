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
// port-lint: source core/descriptors/src/org/jetbrains/kotlin/descriptors/ClassifierDescriptor.java
// Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/ClassifierDescriptor.java:24-34
#pragma once
#include "DeclarationDescriptorNonRoot.hpp"
namespace kotlin::collections { template <typename> class List; }
namespace org::jetbrains::kotlin::types { class TypeConstructor; class SimpleType; }
namespace org::jetbrains::kotlin::descriptors {
class TypeParameterDescriptor;
// Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/ClassifierDescriptor.java:24-34
class ClassifierDescriptor : public virtual DeclarationDescriptorNonRoot, public virtual mpp::ClassifierSymbolMarker {
 public:
  // Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/ClassifierDescriptor.java:25-26
  virtual types::TypeConstructor& get_type_constructor() const = 0;
  // Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/ClassifierDescriptor.java:28-29
  virtual types::SimpleType& get_default_type() const = 0;
  // Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/ClassifierDescriptor.java:31-33
  ClassifierDescriptor& get_original() const override = 0;
};
}  // namespace org::jetbrains::kotlin::descriptors
