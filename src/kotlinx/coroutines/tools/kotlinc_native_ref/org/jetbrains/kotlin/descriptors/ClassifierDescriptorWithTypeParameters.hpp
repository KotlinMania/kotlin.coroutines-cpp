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
// port-lint: source core/descriptors/src/org/jetbrains/kotlin/descriptors/ClassifierDescriptorWithTypeParameters.java
// Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/ClassifierDescriptorWithTypeParameters.java:26-37
#pragma once
#include "ClassifierDescriptor.hpp"
#include "DeclarationDescriptorWithVisibility.hpp"
#include "MemberDescriptor.hpp"
#include "Substitutable.hpp"
namespace kotlin::collections { template <typename> class List; }
namespace org::jetbrains::kotlin::types { class TypeConstructor; class SimpleType; }
namespace org::jetbrains::kotlin::descriptors {
class TypeParameterDescriptor;
// Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/ClassifierDescriptorWithTypeParameters.java:26-37
class ClassifierDescriptorWithTypeParameters : public virtual ClassifierDescriptor, public virtual DeclarationDescriptorWithVisibility, public virtual MemberDescriptor, public virtual Substitutable<ClassifierDescriptorWithTypeParameters>, public virtual mpp::ClassLikeSymbolMarker, public virtual mpp::ClassifierSymbolMarker {
 public:
/**
     * @return <code>true</code> if this class contains a reference to its outer class (as opposed to static nested class)
     */
  // Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/ClassifierDescriptorWithTypeParameters.java:29-32
  virtual bool is_inner() const = 0;
  // Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/ClassifierDescriptorWithTypeParameters.java:34-36
  virtual ::kotlin::collections::List<TypeParameterDescriptor*>& get_declared_type_parameters() const = 0;
};
}  // namespace org::jetbrains::kotlin::descriptors
