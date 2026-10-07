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
// port-lint: source core/descriptors/src/org/jetbrains/kotlin/descriptors/Named.java
// Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/Named.java:22-25
#pragma once

#include "../name/Name.hpp"

namespace org::jetbrains::kotlin::descriptors {

// Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/Named.java:22-25
class Named {
 public:
  virtual ~Named() = default;
  // NOTE(port): Borrow the immutable Name owned by the concrete descriptor.
  // Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/Named.java:23-24
  virtual const name::Name& get_name() const = 0;
};

}  // namespace org::jetbrains::kotlin::descriptors
