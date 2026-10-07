/*
 * Copyright 2010-2015 JetBrains s.r.o. Use of this source code is governed by the Apache 2.0 license.
 */
// port-lint: source core/descriptors/src/org/jetbrains/kotlin/descriptors/impl/DeclarationDescriptorImpl.java
// Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/impl/DeclarationDescriptorImpl.java:27-70
#pragma once

#include "../DeclarationDescriptor.hpp"
#include "../annotations/AnnotatedImpl.hpp"

namespace org::jetbrains::kotlin::descriptors::impl {
// Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/impl/DeclarationDescriptorImpl.java:27-70
class DeclarationDescriptorImpl : public annotations::AnnotatedImpl,
                                  public virtual DeclarationDescriptor {
 public:
  // Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/impl/DeclarationDescriptorImpl.java:32-35
  DeclarationDescriptorImpl(const annotations::Annotations& annotations, name::Name name);
  // Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/impl/DeclarationDescriptorImpl.java:37-41
  const name::Name& get_name() const override;
  // Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/impl/DeclarationDescriptorImpl.java:43-47
  DeclarationDescriptor& get_original() const override;
  // Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/impl/DeclarationDescriptorImpl.java:49-52
  void accept_void(DeclarationDescriptorVisitor<void, std::nullptr_t>& visitor) override;
  // Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/impl/DeclarationDescriptorImpl.java:54-57
  std::string to_string() const override;
  // Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/impl/DeclarationDescriptorImpl.java:59-69
  static std::string to_string(const DeclarationDescriptor& descriptor);
 private:
  const name::Name name_;
};
}  // namespace org::jetbrains::kotlin::descriptors::impl
