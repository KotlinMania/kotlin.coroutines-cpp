/*
 * Copyright 2010-2015 JetBrains s.r.o. Use of this source code is governed by the Apache 2.0 license.
 */
// port-lint: source core/descriptors/src/org/jetbrains/kotlin/descriptors/CallableMemberDescriptor.java
// Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/CallableMemberDescriptor.java:28-103
#include "CallableMemberDescriptor.hpp"

namespace org::jetbrains::kotlin::descriptors {
// Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/CallableMemberDescriptor.java:46-48
bool is_real(CallableMemberDescriptor::Kind kind) {
  return kind != CallableMemberDescriptor::Kind::FAKE_OVERRIDE;
}
}  // namespace org::jetbrains::kotlin::descriptors
