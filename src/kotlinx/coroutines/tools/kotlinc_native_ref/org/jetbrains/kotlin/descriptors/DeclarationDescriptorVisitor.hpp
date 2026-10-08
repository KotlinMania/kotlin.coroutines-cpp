/*
 * Copyright 2010-2015 JetBrains s.r.o.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
// port-lint: source core/descriptors/src/org/jetbrains/kotlin/descriptors/DeclarationDescriptorVisitor.java
// Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/DeclarationDescriptorVisitor.java:19-49
#pragma once

namespace org::jetbrains::kotlin::descriptors {

class PackageFragmentDescriptor;
class PackageViewDescriptor;
class VariableDescriptor;
class FunctionDescriptor;
class TypeParameterDescriptor;
class ClassDescriptor;
class TypeAliasDescriptor;
class ModuleDescriptor;
class ConstructorDescriptor;
class ScriptDescriptor;
class PropertyDescriptor;
class ValueParameterDescriptor;
class PropertyGetterDescriptor;
class PropertySetterDescriptor;
class ReceiverParameterDescriptor;

// Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/DeclarationDescriptorVisitor.java:19-49
template <typename R, typename D>
class DeclarationDescriptorVisitor {
 public:
  virtual ~DeclarationDescriptorVisitor() = default;
  // Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/DeclarationDescriptorVisitor.java:20-20
  virtual R visit_package_fragment_descriptor(PackageFragmentDescriptor& descriptor, D data) = 0;

  // Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/DeclarationDescriptorVisitor.java:22-22
  virtual R visit_package_view_descriptor(PackageViewDescriptor& descriptor, D data) = 0;

  // Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/DeclarationDescriptorVisitor.java:24-24
  virtual R visit_variable_descriptor(VariableDescriptor& descriptor, D data) = 0;

  // Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/DeclarationDescriptorVisitor.java:26-26
  virtual R visit_function_descriptor(FunctionDescriptor& descriptor, D data) = 0;

  // Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/DeclarationDescriptorVisitor.java:28-28
  virtual R visit_type_parameter_descriptor(TypeParameterDescriptor& descriptor, D data) = 0;

  // Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/DeclarationDescriptorVisitor.java:30-30
  virtual R visit_class_descriptor(ClassDescriptor& descriptor, D data) = 0;

  // Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/DeclarationDescriptorVisitor.java:32-32
  virtual R visit_type_alias_descriptor(TypeAliasDescriptor& descriptor, D data) = 0;

  // Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/DeclarationDescriptorVisitor.java:34-34
  virtual R visit_module_declaration(ModuleDescriptor& descriptor, D data) = 0;

  // Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/DeclarationDescriptorVisitor.java:36-36
  virtual R visit_constructor_descriptor(ConstructorDescriptor& constructor_descriptor, D data) = 0;

  // Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/DeclarationDescriptorVisitor.java:38-38
  virtual R visit_script_descriptor(ScriptDescriptor& script_descriptor, D data) = 0;

  // Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/DeclarationDescriptorVisitor.java:40-40
  virtual R visit_property_descriptor(PropertyDescriptor& descriptor, D data) = 0;

  // Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/DeclarationDescriptorVisitor.java:42-42
  virtual R visit_value_parameter_descriptor(ValueParameterDescriptor& descriptor, D data) = 0;

  // Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/DeclarationDescriptorVisitor.java:44-44
  virtual R visit_property_getter_descriptor(PropertyGetterDescriptor& descriptor, D data) = 0;

  // Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/DeclarationDescriptorVisitor.java:46-46
  virtual R visit_property_setter_descriptor(PropertySetterDescriptor& descriptor, D data) = 0;

  // Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/DeclarationDescriptorVisitor.java:48-48
  virtual R visit_receiver_parameter_descriptor(ReceiverParameterDescriptor& descriptor, D data) = 0;

};

}  // namespace org::jetbrains::kotlin::descriptors
