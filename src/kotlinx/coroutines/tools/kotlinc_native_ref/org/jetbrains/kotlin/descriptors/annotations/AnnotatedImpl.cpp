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
// port-lint: source core/descriptors/src/org/jetbrains/kotlin/descriptors/annotations/AnnotatedImpl.java
// Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/annotations/AnnotatedImpl.java:21-33
#include "AnnotatedImpl.hpp"

namespace org::jetbrains::kotlin::descriptors::annotations {

// Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/annotations/AnnotatedImpl.java:24-26
AnnotatedImpl::AnnotatedImpl(const Annotations& annotations) : annotations_(annotations) {}

// Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/annotations/AnnotatedImpl.java:28-32
const Annotations& AnnotatedImpl::get_annotations() const { return annotations_; }

}  // namespace org::jetbrains::kotlin::descriptors::annotations
