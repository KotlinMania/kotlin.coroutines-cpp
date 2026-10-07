/*
 * Copyright 2010-2015 JetBrains s.r.o. Use of this source code is governed by the Apache 2.0 license.
 */
// port-lint: source core/descriptors/src/org/jetbrains/kotlin/descriptors/impl/DeclarationDescriptorNonRootImpl.java
// Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/impl/DeclarationDescriptorNonRootImpl.java:24-69
#pragma once

#include "DeclarationDescriptorImpl.hpp"
#include "../DeclarationDescriptorNonRoot.hpp"

namespace org::jetbrains::kotlin::descriptors::impl {
// Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/impl/DeclarationDescriptorNonRootImpl.java:24-69
class DeclarationDescriptorNonRootImpl : public DeclarationDescriptorImpl,
                                         public virtual DeclarationDescriptorNonRoot {
 public:
  // Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/impl/DeclarationDescriptorNonRootImpl.java:46-50
  DeclarationDescriptorWithSource& get_original() const override;
  // Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/impl/DeclarationDescriptorNonRootImpl.java:52-56
  DeclarationDescriptor* get_containing_declaration() const override __attribute__((returns_nonnull));
  // Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/impl/DeclarationDescriptorNonRootImpl.java:58-62
  const SourceElement& get_source() const override;
  // Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/impl/DeclarationDescriptorNonRootImpl.java:64-67
  void validate() override;
 protected:
  // Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/impl/DeclarationDescriptorNonRootImpl.java:34-44
  DeclarationDescriptorNonRootImpl(DeclarationDescriptor& containing_declaration,
      const annotations::Annotations& annotations, name::Name name, const SourceElement& source);
 private:
  DeclarationDescriptor& containing_declaration_;
  const SourceElement& source_;
};
}  // namespace org::jetbrains::kotlin::descriptors::impl
