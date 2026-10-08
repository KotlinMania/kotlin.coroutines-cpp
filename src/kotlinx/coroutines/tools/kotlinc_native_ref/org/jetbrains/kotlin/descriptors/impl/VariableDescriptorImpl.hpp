/*
 * Copyright 2010-2015 JetBrains s.r.o. Use of this source code is governed by the Apache 2.0 license.
 */
// port-lint: source core/descriptors/src/org/jetbrains/kotlin/descriptors/impl/VariableDescriptorImpl.java
// Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/impl/VariableDescriptorImpl.java:31-123
#pragma once

#include "DeclarationDescriptorNonRootImpl.hpp"
#include "../VariableDescriptor.hpp"

namespace org::jetbrains::kotlin::descriptors::impl {
// Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/impl/VariableDescriptorImpl.java:31-123
class VariableDescriptorImpl : public DeclarationDescriptorNonRootImpl,
                                public virtual VariableDescriptor {
 public:
  // Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/impl/VariableDescriptorImpl.java:34-44
  VariableDescriptorImpl(DeclarationDescriptor& containing_declaration,
      const annotations::Annotations& annotations, name::Name name,
      types::KotlinType* out_type, const SourceElement& source);
  // Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/impl/VariableDescriptorImpl.java:46-50
  types::KotlinType* get_type() const override __attribute__((returns_nonnull));
  // Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/impl/VariableDescriptorImpl.java:52-55
  void set_out_type(types::KotlinType* out_type);
  // Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/impl/VariableDescriptorImpl.java:57-61
  VariableDescriptor& get_original() const override;
  // NOTE(port): Resolve C++ final-overrider ambiguity between ValueDescriptor's
  // explicit contract and the inherited source NonRootImpl implementation.
  // Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/impl/DeclarationDescriptorNonRootImpl.java:52-56
  DeclarationDescriptor* get_containing_declaration() const override __attribute__((returns_nonnull));
  // Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/impl/VariableDescriptorImpl.java:63-67
  const ::kotlin::collections::List<ValueParameterDescriptor*>& get_value_parameters() const override;
  // Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/impl/VariableDescriptorImpl.java:69-72
  bool has_stable_parameter_names() const override;
  // Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/impl/VariableDescriptorImpl.java:74-77
  bool has_synthesized_parameter_names() const override;
  // Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/impl/VariableDescriptorImpl.java:85-89
  const ::kotlin::collections::List<TypeParameterDescriptor*>& get_type_parameters() const override;
  // Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/impl/VariableDescriptorImpl.java:91-95
  const ::kotlin::collections::List<ReceiverParameterDescriptor*>& get_context_receiver_parameters() const override;
  // Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/impl/VariableDescriptorImpl.java:97-100
  ReceiverParameterDescriptor* get_extension_receiver_parameter() const override;
  // Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/impl/VariableDescriptorImpl.java:102-105
  ReceiverParameterDescriptor* get_dispatch_receiver_parameter() const override;
  // Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/impl/VariableDescriptorImpl.java:107-111
  types::KotlinType* get_return_type() const override;
  // Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/impl/VariableDescriptorImpl.java:113-116
  bool is_const() const override;
 protected:
  // Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/impl/VariableDescriptorImpl.java:79-83
  std::shared_ptr<const ::kotlin::collections::detail::CollectionObject>
  get_overridden_descriptors_dispatch() const override;
  // Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/impl/VariableDescriptorImpl.java:118-122
  std::any get_user_data_dispatch(const detail::UserDataKeyObject& key) const override;
  types::KotlinType* out_type_;
};
}  // namespace org::jetbrains::kotlin::descriptors::impl
