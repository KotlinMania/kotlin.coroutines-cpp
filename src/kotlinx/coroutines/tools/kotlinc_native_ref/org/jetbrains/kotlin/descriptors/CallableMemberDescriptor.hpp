/*
 * Copyright 2010-2015 JetBrains s.r.o. Use of this source code is governed by the Apache 2.0 license.
 */
// port-lint: source core/descriptors/src/org/jetbrains/kotlin/descriptors/CallableMemberDescriptor.java
// Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/CallableMemberDescriptor.java:28-103
#pragma once

#include "CallableDescriptor.hpp"
#include "MemberDescriptor.hpp"
#include <type_traits>

namespace org::jetbrains::kotlin::types { class TypeSubstitution; }
namespace org::jetbrains::kotlin::descriptors {
// Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/CallableMemberDescriptor.java:28-103
class CallableMemberDescriptor : public virtual CallableDescriptor,
                                 public virtual MemberDescriptor {
 public:
  // Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/CallableMemberDescriptor.java:39-49
  enum class Kind { DECLARATION, FAKE_OVERRIDE, DELEGATION, SYNTHESIZED };
  template <typename D> class CopyBuilder;

  // Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/CallableMemberDescriptor.java:29-31
  std::shared_ptr<const ::kotlin::collections::Collection<CallableMemberDescriptor*>>
  get_overridden_descriptors() const {
    auto object = get_overridden_descriptors_dispatch();
    auto& typed = dynamic_cast<const ::kotlin::collections::Collection<CallableMemberDescriptor*>&>(*object);
    return {std::move(object), &typed};
  }
  // Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/CallableMemberDescriptor.java:33-35
  CallableMemberDescriptor& get_original() const override = 0;
  // Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/CallableMemberDescriptor.java:37-37
  virtual void set_overridden_descriptors(
      const ::kotlin::collections::Collection<CallableMemberDescriptor*>& overridden_descriptors) = 0;
  /** Is this a real function or function projection. */
  // Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/CallableMemberDescriptor.java:51-55
  virtual Kind get_kind() const = 0;
  // Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/CallableMemberDescriptor.java:57-58
  virtual std::shared_ptr<CallableMemberDescriptor> copy(
      DeclarationDescriptor& new_owner, Modality modality, const DescriptorVisibility& visibility,
      Kind kind, bool copy_overrides) const = 0;
  // NOTE(port): The source out-projected builder is exposed through its upper
  // bound. Concrete builder variance still requires its real source hierarchy.
  // Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/CallableMemberDescriptor.java:60-61
  virtual std::unique_ptr<CopyBuilder<CallableMemberDescriptor>> new_copy_builder() const = 0;
};

// NOTE(port): C++ enum classes cannot declare instance methods. The source
// Kind.isReal receiver is explicit in this free function.
// Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/CallableMemberDescriptor.java:46-48
bool is_real(CallableMemberDescriptor::Kind kind);

// Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/CallableMemberDescriptor.java:63-102
template <typename D>
class CallableMemberDescriptor::CopyBuilder {
  static_assert(std::is_base_of_v<CallableMemberDescriptor, D>);
 public:
  virtual ~CopyBuilder() = default;
  // Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/CallableMemberDescriptor.java:64-65
  virtual CopyBuilder& set_owner(DeclarationDescriptor& owner) = 0;
  // Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/CallableMemberDescriptor.java:67-68
  virtual CopyBuilder& set_modality(Modality modality) = 0;
  // Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/CallableMemberDescriptor.java:70-71
  virtual CopyBuilder& set_visibility(const DescriptorVisibility& visibility) = 0;
  // Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/CallableMemberDescriptor.java:73-74
  virtual CopyBuilder& set_kind(Kind kind) = 0;
  // Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/CallableMemberDescriptor.java:76-77
  virtual CopyBuilder& set_type_parameters(const ::kotlin::collections::List<TypeParameterDescriptor*>& parameters) = 0;
  // Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/CallableMemberDescriptor.java:79-80
  virtual CopyBuilder& set_dispatch_receiver_parameter(ReceiverParameterDescriptor* receiver) = 0;
  // Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/CallableMemberDescriptor.java:82-83
  virtual CopyBuilder& set_substitution(types::TypeSubstitution& substitution) = 0;
  // Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/CallableMemberDescriptor.java:85-86
  virtual CopyBuilder& set_copy_overrides(bool copy_overrides) = 0;
  // Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/CallableMemberDescriptor.java:88-89
  virtual CopyBuilder& set_name(name::Name name) = 0;
  // Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/CallableMemberDescriptor.java:91-92
  virtual CopyBuilder& set_original(CallableMemberDescriptor* original) = 0;
  // Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/CallableMemberDescriptor.java:94-95
  virtual CopyBuilder& set_preserve_source_element() = 0;
  // Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/CallableMemberDescriptor.java:97-98
  virtual CopyBuilder& set_return_type(types::KotlinType& type) = 0;
  // Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/CallableMemberDescriptor.java:100-101
  virtual D* build() = 0;
};
}  // namespace org::jetbrains::kotlin::descriptors
