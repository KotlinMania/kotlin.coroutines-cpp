/*
 * Copyright 2010-2023 JetBrains s.r.o. and Kotlin Programming Language contributors.
 * Use of this source code is governed by the Apache 2.0 license that can be found in the license/LICENSE.txt file.
 */
// port-lint: source core/descriptors/src/org/jetbrains/kotlin/descriptors/ValidateableDescriptor.java
// Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/ValidateableDescriptor.java:8-10
#include "ValidateableDescriptor.hpp"

namespace org::jetbrains::kotlin::descriptors {

// The source interface supplies this empty default; concrete descriptors can override it.
// Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/ValidateableDescriptor.java:9-9
void ValidateableDescriptor::validate() {}

}  // namespace org::jetbrains::kotlin::descriptors
