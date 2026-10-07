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
// port-lint: source core/descriptors/src/org/jetbrains/kotlin/descriptors/annotations/Annotations.kt
// Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/annotations/Annotations.kt:22-24
#pragma once

namespace org::jetbrains::kotlin::descriptors::annotations {

class Annotations;

// Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/annotations/Annotations.kt:22-24
class Annotated {
 public:
  virtual ~Annotated() = default;
  // NOTE(port): The compiler owns the annotation collection, which is borrowed.
  // Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/annotations/Annotations.kt:23-23
  virtual const Annotations& get_annotations() const = 0;
};

}  // namespace org::jetbrains::kotlin::descriptors::annotations
