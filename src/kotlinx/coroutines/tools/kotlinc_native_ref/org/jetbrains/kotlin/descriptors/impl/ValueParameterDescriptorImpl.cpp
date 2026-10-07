/*
 * Copyright 2010-2015 JetBrains s.r.o. Use of this source code is governed by the Apache 2.0 license.
 */
// port-lint: source core/descriptors/src/org/jetbrains/kotlin/descriptors/impl/ValueParameterDescriptorImpl.kt
// Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/impl/ValueParameterDescriptorImpl.kt:26-133
#include "ValueParameterDescriptorImpl.hpp"
#include "../CallableMemberDescriptor.hpp"
#include "../DescriptorVisibilities.hpp"
#include "../../types/TypeSubstitutor.hpp"
#include "../../../../../kotlin/Lazy.hpp"
#include "../../../../../kotlin/collections/_Collections.hpp"
#include <stdexcept>
#include <utility>

namespace org::jetbrains::kotlin::descriptors::impl {
// Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/impl/ValueParameterDescriptorImpl.kt:26-38
// Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/impl/ValueParameterDescriptorImpl.kt:94-94
ValueParameterDescriptorImpl::ValueParameterDescriptorImpl(
    CallableDescriptor& containing_declaration, ValueParameterDescriptor* original, std::int32_t index,
    const annotations::Annotations& annotations, name::Name name, types::KotlinType& out_type,
    bool declares_default_value, bool is_crossinline, bool is_noinline,
    types::KotlinType* vararg_element_type, const SourceElement& source)
    : VariableDescriptorImpl(containing_declaration, annotations, std::move(name), &out_type, source),
      original_(original != nullptr ? original : this), index_(index),
      declares_default_value_(declares_default_value), is_crossinline_(is_crossinline),
      is_noinline_(is_noinline), vararg_element_type_(vararg_element_type) {}
// Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/impl/ValueParameterDescriptorImpl.kt:29-29
std::int32_t ValueParameterDescriptorImpl::get_index() const { return index_; }
// Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/impl/ValueParameterDescriptorImpl.kt:34-34
bool ValueParameterDescriptorImpl::is_crossinline() const { return is_crossinline_; }
// Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/impl/ValueParameterDescriptorImpl.kt:35-35
bool ValueParameterDescriptorImpl::is_noinline() const { return is_noinline_; }
// Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/impl/ValueParameterDescriptorImpl.kt:36-36
types::KotlinType* ValueParameterDescriptorImpl::get_vararg_element_type() const { return vararg_element_type_; }
// Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/impl/ValueParameterDescriptorImpl.kt:42-43
std::shared_ptr<const ::kotlin::collections::List<VariableDescriptor*>>
ValueParameterDescriptorImpl::get_destructuring_variables_or_null(const ValueParameterDescriptor& parameter) {
  const auto* destructuring = dynamic_cast<const WithDestructuringDeclaration*>(&parameter);
  return destructuring != nullptr ? destructuring->get_destructuring_variables() : nullptr;
}
// Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/impl/ValueParameterDescriptorImpl.kt:46-63
std::shared_ptr<ValueParameterDescriptorImpl> ValueParameterDescriptorImpl::create_with_destructuring_declarations(
    CallableDescriptor& containing_declaration, ValueParameterDescriptor* original, std::int32_t index,
    const annotations::Annotations& annotations, name::Name name, types::KotlinType& out_type,
    bool declares_default_value, bool is_crossinline, bool is_noinline,
    types::KotlinType* vararg_element_type, const SourceElement& source,
    std::function<std::shared_ptr<const ::kotlin::collections::List<VariableDescriptor*>>()> destructuring_variables) {
  if (!destructuring_variables) {
    return std::make_shared<ValueParameterDescriptorImpl>(containing_declaration, original, index,
        annotations, std::move(name), out_type, declares_default_value, is_crossinline,
        is_noinline, vararg_element_type, source);
  }
  return std::make_shared<WithDestructuringDeclaration>(containing_declaration, original, index,
      annotations, std::move(name), out_type, declares_default_value, is_crossinline,
      is_noinline, vararg_element_type, source, std::move(destructuring_variables));
}
// Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/impl/ValueParameterDescriptorImpl.kt:66-84
ValueParameterDescriptorImpl::WithDestructuringDeclaration::WithDestructuringDeclaration(
    CallableDescriptor& containing_declaration, ValueParameterDescriptor* original, std::int32_t index,
    const annotations::Annotations& annotations, name::Name name, types::KotlinType& out_type,
    bool declares_default_value, bool is_crossinline, bool is_noinline,
    types::KotlinType* vararg_element_type, const SourceElement& source,
    std::function<std::shared_ptr<const ::kotlin::collections::List<VariableDescriptor*>>()> destructuring_variables)
    : ValueParameterDescriptorImpl(containing_declaration, original, index, annotations, std::move(name),
          out_type, declares_default_value, is_crossinline, is_noinline, vararg_element_type, source),
      destructuring_variables_(::kotlin::lazy(std::move(destructuring_variables))) {}
// Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/impl/ValueParameterDescriptorImpl.kt:81-84
std::shared_ptr<const ::kotlin::collections::List<VariableDescriptor*>>
ValueParameterDescriptorImpl::WithDestructuringDeclaration::get_destructuring_variables() const {
  return destructuring_variables_->get_value();
}
// Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/impl/ValueParameterDescriptorImpl.kt:86-91
std::shared_ptr<ValueParameterDescriptor> ValueParameterDescriptorImpl::WithDestructuringDeclaration::copy(
    CallableDescriptor& new_owner, name::Name new_name, std::int32_t new_index) const {
  // NOTE(port): The source closure retains the original lazy value. Retain that
  // same delegate in C++ so copying never borrows a destroyed source descriptor.
  auto variables = destructuring_variables_;
  return std::make_shared<WithDestructuringDeclaration>(new_owner, nullptr, new_index,
      get_annotations(), std::move(new_name), *get_type(), declares_default_value(),
      is_crossinline(), is_noinline(), get_vararg_element_type(), SourceElement::NO_SOURCE,
      [variables = std::move(variables)] { return variables->get_value(); });
}
// Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/impl/ValueParameterDescriptorImpl.kt:96-96
CallableDescriptor* ValueParameterDescriptorImpl::get_containing_declaration() const {
  return &dynamic_cast<CallableDescriptor&>(*VariableDescriptorImpl::get_containing_declaration());
}
// Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/impl/ValueParameterDescriptorImpl.kt:98-100
bool ValueParameterDescriptorImpl::declares_default_value() const {
  return declares_default_value_ && is_real(dynamic_cast<CallableMemberDescriptor&>(
      *get_containing_declaration()).get_kind());
}
// Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/impl/ValueParameterDescriptorImpl.kt:102-102
ValueParameterDescriptor& ValueParameterDescriptorImpl::get_original() const {
  return original_ == this ? const_cast<ValueParameterDescriptorImpl&>(*this) : original_->get_original();
}
// Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/impl/ValueParameterDescriptorImpl.kt:104-107
ValueParameterDescriptor& ValueParameterDescriptorImpl::substitute(types::TypeSubstitutor& substitutor) {
  if (substitutor.is_empty()) return *this;
  // NOTE(port): Source nonempty substitution throws UnsupportedOperationException;
  // the compiler's standard exception boundary represents it as logic_error.
  throw std::logic_error("");
}
// Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/impl/ValueParameterDescriptorImpl.kt:109-111
void ValueParameterDescriptorImpl::accept_dispatch(detail::DescriptorVisitorDispatch& dispatch) {
  dispatch.visit_value_parameter_descriptor(*this);
}
// Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/impl/ValueParameterDescriptorImpl.kt:113-113
bool ValueParameterDescriptorImpl::is_var() const { return false; }
// Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/impl/ValueParameterDescriptorImpl.kt:115-115
const resolve::constants::ConstantValue<std::any>* ValueParameterDescriptorImpl::get_compile_time_initializer() const {
  return nullptr;
}
// Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/impl/ValueParameterDescriptorImpl.kt:117-117
void ValueParameterDescriptorImpl::clean_compile_time_initializer_cache() {}
// Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/impl/ValueParameterDescriptorImpl.kt:119-124
std::shared_ptr<ValueParameterDescriptor> ValueParameterDescriptorImpl::copy(
    CallableDescriptor& new_owner, name::Name new_name, std::int32_t new_index) const {
  return std::make_shared<ValueParameterDescriptorImpl>(new_owner, nullptr, new_index,
      get_annotations(), std::move(new_name), *get_type(), declares_default_value(),
      is_crossinline(), is_noinline(), get_vararg_element_type(), SourceElement::NO_SOURCE);
}
// Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/impl/ValueParameterDescriptorImpl.kt:126-126
const DescriptorVisibility& ValueParameterDescriptorImpl::get_visibility() const { return DescriptorVisibilities::LOCAL; }
// Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/impl/ValueParameterDescriptorImpl.kt:128-132
std::shared_ptr<const ::kotlin::collections::detail::CollectionObject>
ValueParameterDescriptorImpl::get_overridden_descriptors_dispatch() const {
  return ::kotlin::collections::map(*get_containing_declaration()->get_overridden_descriptors(),
      [this](CallableDescriptor* owner) { return owner->get_value_parameters().get(index_); });
}
}  // namespace org::jetbrains::kotlin::descriptors::impl
