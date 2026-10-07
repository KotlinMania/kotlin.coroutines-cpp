/*
 * Copyright 2010-2015 JetBrains s.r.o. Use of this source code is governed by the Apache 2.0 license.
 */
// port-lint: source core/descriptors/src/org/jetbrains/kotlin/descriptors/impl/VariableDescriptorImpl.java
// Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/impl/VariableDescriptorImpl.java:31-123
#include "VariableDescriptorImpl.hpp"
#include "../../types/type_util/TypeUtils.hpp"
#include "../../../../../kotlin/collections/Sets.hpp"
#include <cassert>
#include <utility>

namespace org::jetbrains::kotlin::descriptors::impl {
// Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/impl/VariableDescriptorImpl.java:34-44
VariableDescriptorImpl::VariableDescriptorImpl(DeclarationDescriptor& containing_declaration,
    const annotations::Annotations& annotations, name::Name name,
    types::KotlinType* out_type, const SourceElement& source)
    : DeclarationDescriptorNonRootImpl(containing_declaration, annotations, std::move(name), source),
      out_type_(out_type) {}
// Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/impl/VariableDescriptorImpl.java:46-50
types::KotlinType* VariableDescriptorImpl::get_type() const { return out_type_; }
// Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/impl/VariableDescriptorImpl.java:52-55
void VariableDescriptorImpl::set_out_type(types::KotlinType* out_type) {
  assert(out_type_ == nullptr || types::type_util::should_be_updated(out_type_));
  out_type_ = out_type;
}
// Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/impl/VariableDescriptorImpl.java:57-61
VariableDescriptor& VariableDescriptorImpl::get_original() const {
  return dynamic_cast<VariableDescriptor&>(DeclarationDescriptorNonRootImpl::get_original());
}
// Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/impl/DeclarationDescriptorNonRootImpl.java:52-56
DeclarationDescriptor* VariableDescriptorImpl::get_containing_declaration() const {
  return DeclarationDescriptorNonRootImpl::get_containing_declaration();
}
// Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/impl/VariableDescriptorImpl.java:63-67
const ::kotlin::collections::List<ValueParameterDescriptor*>&
VariableDescriptorImpl::get_value_parameters() const {
  return *::kotlin::collections::empty_list<ValueParameterDescriptor*>();
}
// Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/impl/VariableDescriptorImpl.java:69-72
bool VariableDescriptorImpl::has_stable_parameter_names() const { return false; }
// Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/impl/VariableDescriptorImpl.java:74-77
bool VariableDescriptorImpl::has_synthesized_parameter_names() const { return false; }
// Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/impl/VariableDescriptorImpl.java:79-83
std::shared_ptr<const ::kotlin::collections::detail::CollectionObject>
VariableDescriptorImpl::get_overridden_descriptors_dispatch() const {
  return ::kotlin::collections::empty_set<CallableDescriptor*>();
}
// Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/impl/VariableDescriptorImpl.java:85-89
const ::kotlin::collections::List<TypeParameterDescriptor*>& VariableDescriptorImpl::get_type_parameters() const {
  return *::kotlin::collections::empty_list<TypeParameterDescriptor*>();
}
// Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/impl/VariableDescriptorImpl.java:91-95
const ::kotlin::collections::List<ReceiverParameterDescriptor*>&
VariableDescriptorImpl::get_context_receiver_parameters() const {
  return *::kotlin::collections::empty_list<ReceiverParameterDescriptor*>();
}
// Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/impl/VariableDescriptorImpl.java:97-100
ReceiverParameterDescriptor* VariableDescriptorImpl::get_extension_receiver_parameter() const { return nullptr; }
// Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/impl/VariableDescriptorImpl.java:102-105
ReceiverParameterDescriptor* VariableDescriptorImpl::get_dispatch_receiver_parameter() const { return nullptr; }
// Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/impl/VariableDescriptorImpl.java:107-111
types::KotlinType* VariableDescriptorImpl::get_return_type() const { return get_type(); }
// Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/impl/VariableDescriptorImpl.java:113-116
bool VariableDescriptorImpl::is_const() const { return false; }
// Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/impl/VariableDescriptorImpl.java:118-122
std::any VariableDescriptorImpl::get_user_data_dispatch(const detail::UserDataKeyObject&) const { return {}; }
}  // namespace org::jetbrains::kotlin::descriptors::impl
