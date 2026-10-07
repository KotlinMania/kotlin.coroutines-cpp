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
// port-lint: source core/descriptors/src/org/jetbrains/kotlin/descriptors/ValueParameterDescriptor.kt
// Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/ValueParameterDescriptor.kt:24-60
#pragma once

#include "VariableDescriptor.hpp"
#include "ParameterDescriptor.hpp"
namespace org::jetbrains::kotlin::descriptors {
  // Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/ValueParameterDescriptor.kt:24-24
class ValueParameterDescriptor : public virtual VariableDescriptor,
                                 public virtual ParameterDescriptor,
                                 public virtual mpp::ValueParameterSymbolMarker {
 public:
  // Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/ValueParameterDescriptor.kt:25-25
  CallableDescriptor* get_containing_declaration() const override
      __attribute__((returns_nonnull)) = 0;
    /**
     * Returns the 0-based index of the value parameter in the parameter list of its containing function.

     * @return the parameter index
     */
  // Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/ValueParameterDescriptor.kt:32-32
  virtual std::int32_t get_index() const = 0;
    /**
     * @return true iff this parameter belongs to a declared function (not a fake override) and declares the default value,
     * i.e. explicitly specifies it in the function signature. Also see 'hasDefaultValue' extension in DescriptorUtils.kt
     */
  // Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/ValueParameterDescriptor.kt:38-38
  virtual bool declares_default_value() const = 0;
  // Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/ValueParameterDescriptor.kt:40-40
  virtual types::KotlinType* get_vararg_element_type() const = 0;
  // Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/ValueParameterDescriptor.kt:42-42
  ValueParameterDescriptor& get_original() const override = 0;
  // Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/ValueParameterDescriptor.kt:44-44
  ValueParameterDescriptor& substitute(types::TypeSubstitutor& substitutor) override = 0;
  // Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/ValueParameterDescriptor.kt:46-46
  virtual std::shared_ptr<ValueParameterDescriptor> copy(CallableDescriptor& new_owner,
                                                        name::Name new_name,
                                                        std::int32_t new_index) const = 0;
    /**
     * Parameter p1 overrides p2 iff
     * a) their respective owners (function declarations) f1 override f2
     * b) p1 and p2 have the same indices in the owners' parameter lists
     */
  // Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/ValueParameterDescriptor.kt:53-53
  std::shared_ptr<const ::kotlin::collections::Collection<ValueParameterDescriptor*>>
  get_overridden_descriptors() const {
    auto object = get_overridden_descriptors_dispatch();
    auto& typed = dynamic_cast<const ::kotlin::collections::Collection<ValueParameterDescriptor*>&>(*object);
    return {std::move(object), &typed};
  }
  // Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/ValueParameterDescriptor.kt:55-55
  virtual bool is_crossinline() const = 0;
  // Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/ValueParameterDescriptor.kt:57-57
  virtual bool is_noinline() const = 0;
  // Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/ValueParameterDescriptor.kt:59-59
  bool is_late_init() const override;
};
}  // namespace org::jetbrains::kotlin::descriptors
