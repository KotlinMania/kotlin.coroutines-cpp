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
// port-lint: source core/descriptors/src/org/jetbrains/kotlin/descriptors/VariableDescriptor.java
// Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/VariableDescriptor.java:25-46
#pragma once

#include "ValueDescriptor.hpp"

namespace org::jetbrains::kotlin::resolve::constants { template <typename T> class ConstantValue; }
namespace org::jetbrains::kotlin::descriptors {
  // Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/VariableDescriptor.java:25-46
class VariableDescriptor : public virtual ValueDescriptor {
 public:
  // Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/VariableDescriptor.java:26-27
  VariableDescriptor& substitute(types::TypeSubstitutor& substitutor) override = 0;
  // Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/VariableDescriptor.java:29-29
  virtual bool is_var() const = 0;
  // NOTE(port): Source ConstantValue<?> is its out-projected Any value view.
  // ConstantValue and its concrete value/type/visitor algorithms remain real
  // forward-referenced compiler dependencies, not replacement constant types.
  // Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/VariableDescriptor.java:31-32
  virtual const resolve::constants::ConstantValue<std::any>* get_compile_time_initializer() const = 0;
    /**
     * ONLY FOR IDE USE! Please don't use the method inside the compiler
     */
  // Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/VariableDescriptor.java:34-37
  virtual void clean_compile_time_initializer_cache() = 0;
  /**
   * @return true if iff original declaration has appropriate flags and type, e.g. `const` modifier in Kotlin.
   * It completely does not means that if isConst then `getCompileTimeInitializer` is not null
   */
  // Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/VariableDescriptor.java:39-43
  virtual bool is_const() const = 0;
  // Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/VariableDescriptor.java:45-45
  virtual bool is_late_init() const = 0;
};
}  // namespace org::jetbrains::kotlin::descriptors
