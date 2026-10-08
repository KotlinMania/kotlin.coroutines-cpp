/*
 * Copyright 2010-2015 JetBrains s.r.o. Use of this source code is governed by the Apache 2.0 license.
 */
// port-lint: source core/descriptors/src/org/jetbrains/kotlin/descriptors/impl/DeclarationDescriptorNonRootImpl.java
// Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/impl/DeclarationDescriptorNonRootImpl.java:24-69
#include "DeclarationDescriptorNonRootImpl.hpp"
#include <utility>

namespace org::jetbrains::kotlin::descriptors::impl {
// Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/impl/DeclarationDescriptorNonRootImpl.java:34-44
DeclarationDescriptorNonRootImpl::DeclarationDescriptorNonRootImpl(
    DeclarationDescriptor& containing_declaration, const annotations::Annotations& annotations,
    name::Name name, const SourceElement& source)
    : DeclarationDescriptorImpl(annotations, std::move(name)),
      containing_declaration_(containing_declaration), source_(source) {}
// Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/impl/DeclarationDescriptorNonRootImpl.java:46-50
DeclarationDescriptorWithSource& DeclarationDescriptorNonRootImpl::get_original() const {
  return dynamic_cast<DeclarationDescriptorWithSource&>(DeclarationDescriptorImpl::get_original());
}
// Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/impl/DeclarationDescriptorNonRootImpl.java:52-56
DeclarationDescriptor* DeclarationDescriptorNonRootImpl::get_containing_declaration() const {
  return &containing_declaration_;
}
// Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/impl/DeclarationDescriptorNonRootImpl.java:58-62
const SourceElement& DeclarationDescriptorNonRootImpl::get_source() const { return source_; }
// Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/impl/DeclarationDescriptorNonRootImpl.java:64-67
void DeclarationDescriptorNonRootImpl::validate() { containing_declaration_.validate(); }
}  // namespace org::jetbrains::kotlin::descriptors::impl
