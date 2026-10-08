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
// port-lint: source core/descriptors/src/org/jetbrains/kotlin/descriptors/TypeParameterDescriptor.java
// Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/TypeParameterDescriptor.java:29-61
#pragma once
#include "ClassifierDescriptor.hpp"
#include "../types/Variance.hpp"
#include "../types/model/TypeSystemContext.hpp"
#include <cstdint>
namespace org::jetbrains::kotlin::storage { class StorageManager; }
namespace org::jetbrains::kotlin::types { class KotlinType; }
namespace org::jetbrains::kotlin::descriptors {
// Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/TypeParameterDescriptor.java:29-61
class TypeParameterDescriptor : public virtual ClassifierDescriptor,
    public virtual types::model::TypeParameterMarker,
    public virtual mpp::TypeParameterSymbolMarker {
 public:
// Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/TypeParameterDescriptor.java:30-30
  virtual bool is_reified() const = 0;
// Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/TypeParameterDescriptor.java:32-33
  virtual types::Variance get_variance() const = 0;
// Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/TypeParameterDescriptor.java:35-36
  virtual ::kotlin::collections::List<types::KotlinType*>& get_upper_bounds() const = 0;
// Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/TypeParameterDescriptor.java:38-40
  types::TypeConstructor& get_type_constructor() const override = 0;
// Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/TypeParameterDescriptor.java:42-44
  TypeParameterDescriptor& get_original() const override = 0;
// Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/TypeParameterDescriptor.java:46-46
  virtual std::int32_t get_index() const = 0;
/**
     * Is current parameter just a copy of another type parameter (getOriginal) from outer declaration
     * to be used for type constructor of inner declaration (i.e. inner class).
     *
     * If this method returns true:
     * 1. Containing declaration for current parameter is the inner one
     * 2. 'getOriginal' returns original type parameter from outer declaration
     * 3. 'getTypeConstructor' is the same as for original declaration (at least in means of 'equals')
     */
// Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/TypeParameterDescriptor.java:57-57
  virtual bool is_captured_from_outer_declaration() const = 0;
// Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/TypeParameterDescriptor.java:59-60
  virtual storage::StorageManager& get_storage_manager() const = 0;
};
}  // namespace org::jetbrains::kotlin::descriptors
