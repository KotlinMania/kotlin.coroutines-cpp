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
// port-lint: source core/descriptors/src/org/jetbrains/kotlin/descriptors/ParameterDescriptor.java
// Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/ParameterDescriptor.java:21-25
#pragma once

#include "ValueDescriptor.hpp"
namespace org::jetbrains::kotlin::descriptors {
  // Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/ParameterDescriptor.java:21-25
class ParameterDescriptor : public virtual ValueDescriptor {
 public:
  // Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/ParameterDescriptor.java:22-24
  ParameterDescriptor& get_original() const override = 0;
};
}  // namespace org::jetbrains::kotlin::descriptors
