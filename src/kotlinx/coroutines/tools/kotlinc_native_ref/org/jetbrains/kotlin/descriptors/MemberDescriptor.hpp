/*
 * Copyright 2010-2015 JetBrains s.r.o. Use of this source code is governed by the Apache 2.0 license.
 */
// port-lint: source core/descriptors/src/org/jetbrains/kotlin/descriptors/MemberDescriptor.java
// Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/MemberDescriptor.java:21-34
#pragma once

#include "DeclarationDescriptorNonRoot.hpp"
#include "DeclarationDescriptorWithVisibility.hpp"
#include "Modality.hpp"

namespace org::jetbrains::kotlin::descriptors {
// Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/MemberDescriptor.java:21-34
class MemberDescriptor : public virtual DeclarationDescriptorNonRoot,
                         public virtual DeclarationDescriptorWithVisibility {
 public:
  // Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/MemberDescriptor.java:22-23
  virtual Modality get_modality() const = 0;
  // Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/MemberDescriptor.java:25-27
  const DescriptorVisibility& get_visibility() const override = 0;
  // Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/MemberDescriptor.java:29-29
  virtual bool is_expect() const = 0;
  // Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/MemberDescriptor.java:31-31
  virtual bool is_actual() const = 0;
  // Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/MemberDescriptor.java:33-33
  virtual bool is_external() const = 0;
};
}  // namespace org::jetbrains::kotlin::descriptors
