/*
 * Copyright 2010-2015 JetBrains s.r.o. Use of this source code is governed by the Apache 2.0 license.
 */
// port-lint: source core/descriptors/src/org/jetbrains/kotlin/descriptors/impl/ValueParameterDescriptorImpl.kt
// Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/impl/ValueParameterDescriptorImpl.kt:26-133
#pragma once

#include "VariableDescriptorImpl.hpp"
#include "../ValueParameterDescriptor.hpp"
#include <functional>

namespace kotlin { template <typename T> class Lazy; }
namespace org::jetbrains::kotlin::descriptors::impl {
// Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/impl/ValueParameterDescriptorImpl.kt:26-133
class ValueParameterDescriptorImpl : public VariableDescriptorImpl,
                                    public virtual ValueParameterDescriptor {
 public:
  class WithDestructuringDeclaration;
  // Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/impl/ValueParameterDescriptorImpl.kt:26-38
// Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/impl/ValueParameterDescriptorImpl.kt:94-94
  ValueParameterDescriptorImpl(CallableDescriptor& containing_declaration, ValueParameterDescriptor* original,
      std::int32_t index, const annotations::Annotations& annotations, name::Name name,
      types::KotlinType& out_type, bool declares_default_value, bool is_crossinline,
      bool is_noinline, types::KotlinType* vararg_element_type, const SourceElement& source);
  // Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/impl/ValueParameterDescriptorImpl.kt:29-29
  std::int32_t get_index() const override;
  // Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/impl/ValueParameterDescriptorImpl.kt:34-34
  bool is_crossinline() const override;
  // Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/impl/ValueParameterDescriptorImpl.kt:35-35
  bool is_noinline() const override;
  // Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/impl/ValueParameterDescriptorImpl.kt:36-36
  types::KotlinType* get_vararg_element_type() const override;
  // Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/impl/ValueParameterDescriptorImpl.kt:42-43
  static std::shared_ptr<const ::kotlin::collections::List<VariableDescriptor*>>
  get_destructuring_variables_or_null(const ValueParameterDescriptor& value_parameter_descriptor);
  // Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/impl/ValueParameterDescriptorImpl.kt:46-63
  static std::shared_ptr<ValueParameterDescriptorImpl> create_with_destructuring_declarations(
      CallableDescriptor& containing_declaration, ValueParameterDescriptor* original,
      std::int32_t index, const annotations::Annotations& annotations, name::Name name,
      types::KotlinType& out_type, bool declares_default_value, bool is_crossinline,
      bool is_noinline, types::KotlinType* vararg_element_type, const SourceElement& source,
      std::function<std::shared_ptr<const ::kotlin::collections::List<VariableDescriptor*>>()> destructuring_variables);
  // Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/impl/ValueParameterDescriptorImpl.kt:96-96
  CallableDescriptor* get_containing_declaration() const override __attribute__((returns_nonnull));
  // Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/impl/ValueParameterDescriptorImpl.kt:98-100
  bool declares_default_value() const override;
  // Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/impl/ValueParameterDescriptorImpl.kt:102-102
  ValueParameterDescriptor& get_original() const override;
  // Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/impl/ValueParameterDescriptorImpl.kt:104-107
  ValueParameterDescriptor& substitute(types::TypeSubstitutor& substitutor) override;
  // Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/impl/ValueParameterDescriptorImpl.kt:113-113
  bool is_var() const override;
  // Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/impl/ValueParameterDescriptorImpl.kt:115-115
  const resolve::constants::ConstantValue<std::any>* get_compile_time_initializer() const override;
  // Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/impl/ValueParameterDescriptorImpl.kt:117-117
  void clean_compile_time_initializer_cache() override;
  // Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/impl/ValueParameterDescriptorImpl.kt:119-124
  std::shared_ptr<ValueParameterDescriptor> copy(CallableDescriptor& new_owner, name::Name new_name,
                                               std::int32_t new_index) const override;
  // Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/impl/ValueParameterDescriptorImpl.kt:126-126
  const DescriptorVisibility& get_visibility() const override;
 protected:
  // Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/impl/ValueParameterDescriptorImpl.kt:109-111
  void accept_dispatch(detail::DescriptorVisitorDispatch& dispatch) override;
  // Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/impl/ValueParameterDescriptorImpl.kt:128-132
  std::shared_ptr<const ::kotlin::collections::detail::CollectionObject>
  get_overridden_descriptors_dispatch() const override;
 private:
  ValueParameterDescriptor* original_;
  const std::int32_t index_;
  const bool declares_default_value_;
  const bool is_crossinline_;
  const bool is_noinline_;
  types::KotlinType* const vararg_element_type_;
};

// Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/impl/ValueParameterDescriptorImpl.kt:66-92
class ValueParameterDescriptorImpl::WithDestructuringDeclaration final : public ValueParameterDescriptorImpl {
 public:
  // NOTE(port): Kotlin internal construction uses the compiler module boundary.
  // Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/impl/ValueParameterDescriptorImpl.kt:66-84
  WithDestructuringDeclaration(CallableDescriptor& containing_declaration, ValueParameterDescriptor* original,
      std::int32_t index, const annotations::Annotations& annotations, name::Name name,
      types::KotlinType& out_type, bool declares_default_value, bool is_crossinline,
      bool is_noinline, types::KotlinType* vararg_element_type, const SourceElement& source,
      std::function<std::shared_ptr<const ::kotlin::collections::List<VariableDescriptor*>>()> destructuring_variables);
  // It's forced to be lazy because its resolution depends on receiver of relevant lambda, that is being created at the same moment
  // as value parameters.
  // Must be forced via ForceResolveUtil.forceResolveAllContents()
  // Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/impl/ValueParameterDescriptorImpl.kt:81-84
  std::shared_ptr<const ::kotlin::collections::List<VariableDescriptor*>> get_destructuring_variables() const;
  // Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/impl/ValueParameterDescriptorImpl.kt:86-91
  std::shared_ptr<ValueParameterDescriptor> copy(CallableDescriptor& new_owner, name::Name new_name,
                                               std::int32_t new_index) const override;
 private:
  std::shared_ptr<::kotlin::Lazy<std::shared_ptr<const ::kotlin::collections::List<VariableDescriptor*>>>>
      destructuring_variables_;
};
}  // namespace org::jetbrains::kotlin::descriptors::impl
