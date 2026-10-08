/*
 * Copyright 2000-2018 JetBrains s.r.o. and Kotlin Programming Language contributors.
 * Use of this source code is governed by the Apache 2.0 license that can be found in the license/LICENSE.txt file.
 */
// port-lint: source core/descriptors/src/org/jetbrains/kotlin/descriptors/ClassDescriptor.java
// Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/ClassDescriptor.java:20-122
#pragma once
#include "ClassifierDescriptorWithTypeParameters.hpp"
#include "ClassOrPackageFragmentDescriptor.hpp"
#include "ClassKind.hpp"
#include "Modality.hpp"
#include "DescriptorVisibility.hpp"
namespace kotlin::collections { template <typename> class List; template <typename> class Collection; }
namespace org::jetbrains::kotlin::resolve::scopes { class MemberScope; }
namespace org::jetbrains::kotlin::types { class TypeProjection; class TypeSubstitution; }
namespace org::jetbrains::kotlin::descriptors {
class ClassConstructorDescriptor;
class ReceiverParameterDescriptor;
template <typename> class ValueClassRepresentation;
// Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/ClassDescriptor.java:20-122
class ClassDescriptor : public virtual ClassifierDescriptorWithTypeParameters, public virtual ClassOrPackageFragmentDescriptor, public virtual mpp::RegularClassSymbolMarker {
 public:
  // Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/ClassDescriptor.java:23-23
  virtual resolve::scopes::MemberScope& get_member_scope(const ::kotlin::collections::List<types::TypeProjection*>& type_arguments) const = 0;
  // Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/ClassDescriptor.java:26-26
  virtual resolve::scopes::MemberScope& get_member_scope(types::TypeSubstitution& type_substitution) const = 0;
  // Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/ClassDescriptor.java:29-29
  virtual resolve::scopes::MemberScope& get_unsubstituted_member_scope() const = 0;
  // Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/ClassDescriptor.java:32-32
  virtual resolve::scopes::MemberScope& get_unsubstituted_inner_classes_scope() const = 0;
  // Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/ClassDescriptor.java:35-35
  virtual resolve::scopes::MemberScope& get_static_scope() const = 0;
  // Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/ClassDescriptor.java:39-39
  virtual ::kotlin::collections::Collection<ClassConstructorDescriptor*>& get_constructors() const = 0;
  // Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/ClassDescriptor.java:43-43
  DeclarationDescriptor* get_containing_declaration() const override __attribute__((returns_nonnull)) = 0;

    /**
     * @return type A&lt;T&gt; for the class A&lt;T&gt;
     */
  // Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/ClassDescriptor.java:50-50
  types::SimpleType& get_default_type() const override = 0;

    /**
     * @return nested object declared as 'companion' if one is present.
     */
  // Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/ClassDescriptor.java:56-56
  virtual ClassDescriptor* get_companion_object_descriptor() const = 0;
  // Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/ClassDescriptor.java:59-59
  virtual ClassKind get_kind() const = 0;
  // Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/ClassDescriptor.java:63-63
  Modality get_modality() const override = 0;
  // Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/ClassDescriptor.java:67-67
  DescriptorVisibility& get_visibility() const override = 0;
  // Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/ClassDescriptor.java:69-69
  virtual bool is_companion_object() const = 0;
  // Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/ClassDescriptor.java:71-71
  virtual bool is_data() const = 0;
  // Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/ClassDescriptor.java:73-73
  virtual bool is_inline() const = 0;
  // Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/ClassDescriptor.java:75-75
  virtual bool is_fun() const = 0;
  // Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/ClassDescriptor.java:77-77
  virtual bool is_value() const = 0;
  // Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/ClassDescriptor.java:80-80
  virtual ReceiverParameterDescriptor& get_this_as_receiver_parameter() const = 0;
  // Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/ClassDescriptor.java:84-84
  virtual ::kotlin::collections::List<ReceiverParameterDescriptor*>& get_context_receivers() const = 0;
  // Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/ClassDescriptor.java:87-87
  virtual ClassConstructorDescriptor* get_unsubstituted_primary_constructor() const = 0;

    /**
     * It may differ from 'typeConstructor.parameters' in current class is inner, 'typeConstructor.parameters' contains
     * captured parameters from outer declaration.
     * @return list of type parameters actually declared type parameters in current class
     */
  // Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/ClassDescriptor.java:97-97
  ::kotlin::collections::List<TypeParameterDescriptor*>& get_declared_type_parameters() const override = 0;

    /**
     * @return direct subclasses of this class if it's a sealed class, empty list otherwise
     */
  // Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/ClassDescriptor.java:104-104
  virtual ::kotlin::collections::Collection<ClassDescriptor*>& get_sealed_subclasses() const = 0;
  // Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/ClassDescriptor.java:107-107
  virtual ValueClassRepresentation<types::SimpleType>* get_value_class_representation() const = 0;
  // Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/ClassDescriptor.java:111-111
  ClassDescriptor& get_original() const override = 0;

    // Use SingleAbstractMethodUtils.getFunctionTypeForSamInterface() where possible. This is only a fallback
  // Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/ClassDescriptor.java:115-115
  virtual types::SimpleType* get_default_function_type_for_sam_interface() const = 0;

    /**
     * May return false even in case when the class is not SAM interface, but returns true only if it's definitely not a SAM.
     * But it should work much faster than the exact check.
     */
  // Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/ClassDescriptor.java:121-121
  virtual bool is_definitely_not_sam_interface() const = 0;
};
}  // namespace org::jetbrains::kotlin::descriptors
