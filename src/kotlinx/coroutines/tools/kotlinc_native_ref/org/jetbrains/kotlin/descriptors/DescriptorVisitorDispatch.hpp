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

#include "DeclarationDescriptorVisitor.hpp"

#include <functional>
#include <optional>
#include <type_traits>
#include <utility>

namespace org::jetbrains::kotlin::descriptors::detail {

// NOTE(port): C++ has no virtual function templates. This internal dispatch
// preserves the source visitor's exact descriptor type at the virtual boundary;
// the typed adapter retains R and D. It does not select nodes by names or IDs.
class DescriptorVisitorDispatch {
 public:
  virtual ~DescriptorVisitorDispatch() = default;
  // Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/DeclarationDescriptorVisitor.java:20-20
  virtual void visit_package_fragment_descriptor(PackageFragmentDescriptor& descriptor) = 0;
  // Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/DeclarationDescriptorVisitor.java:22-22
  virtual void visit_package_view_descriptor(PackageViewDescriptor& descriptor) = 0;
  // Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/DeclarationDescriptorVisitor.java:24-24
  virtual void visit_variable_descriptor(VariableDescriptor& descriptor) = 0;
  // Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/DeclarationDescriptorVisitor.java:26-26
  virtual void visit_function_descriptor(FunctionDescriptor& descriptor) = 0;
  // Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/DeclarationDescriptorVisitor.java:28-28
  virtual void visit_type_parameter_descriptor(TypeParameterDescriptor& descriptor) = 0;
  // Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/DeclarationDescriptorVisitor.java:30-30
  virtual void visit_class_descriptor(ClassDescriptor& descriptor) = 0;
  // Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/DeclarationDescriptorVisitor.java:32-32
  virtual void visit_type_alias_descriptor(TypeAliasDescriptor& descriptor) = 0;
  // Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/DeclarationDescriptorVisitor.java:34-34
  virtual void visit_module_declaration(ModuleDescriptor& descriptor) = 0;
  // Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/DeclarationDescriptorVisitor.java:36-36
  virtual void visit_constructor_descriptor(ConstructorDescriptor& constructor_descriptor) = 0;
  // Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/DeclarationDescriptorVisitor.java:38-38
  virtual void visit_script_descriptor(ScriptDescriptor& script_descriptor) = 0;
  // Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/DeclarationDescriptorVisitor.java:40-40
  virtual void visit_property_descriptor(PropertyDescriptor& descriptor) = 0;
  // Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/DeclarationDescriptorVisitor.java:42-42
  virtual void visit_value_parameter_descriptor(ValueParameterDescriptor& descriptor) = 0;
  // Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/DeclarationDescriptorVisitor.java:44-44
  virtual void visit_property_getter_descriptor(PropertyGetterDescriptor& descriptor) = 0;
  // Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/DeclarationDescriptorVisitor.java:46-46
  virtual void visit_property_setter_descriptor(PropertySetterDescriptor& descriptor) = 0;
  // Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/DeclarationDescriptorVisitor.java:48-48
  virtual void visit_receiver_parameter_descriptor(ReceiverParameterDescriptor& descriptor) = 0;
};

// NOTE(port): Store one returned R without requiring it to be default
// constructible or copyable. C++ reference results retain the referenced object.
// Java Void/Unit return values map to void. Exceptions propagate through the call.
template <typename R>
class DescriptorVisitorResult {
 public:
  void set(R value) {
    if constexpr (std::is_reference_v<R>) {
      value_.emplace(std::ref(value));
    } else {
      value_.emplace(std::move(value));
    }
  }
  R take() {
    if constexpr (std::is_reference_v<R>) {
      return value_->get();
    } else {
      return std::move(*value_);
    }
  }
 private:
  using Stored = std::conditional_t<std::is_reference_v<R>,
      std::reference_wrapper<std::remove_reference_t<R>>, R>;
  std::optional<Stored> value_;
};

template <>
class DescriptorVisitorResult<void> {
 public:
  void take() {}
};

template <typename R, typename D>
class TypedDescriptorVisitorDispatch final : public DescriptorVisitorDispatch {
 public:
  TypedDescriptorVisitorDispatch(DeclarationDescriptorVisitor<R, D>& visitor, D& data)
      : visitor_(visitor), data_(data) {}
  R take_result() { return result_.take(); }
  // Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/DeclarationDescriptorVisitor.java:20-20
  void visit_package_fragment_descriptor(PackageFragmentDescriptor& descriptor) override {
    if constexpr (std::is_void_v<R>) {
      visitor_.visit_package_fragment_descriptor(descriptor, data_);
    } else {
      result_.set(visitor_.visit_package_fragment_descriptor(descriptor, data_));
    }
  }
  // Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/DeclarationDescriptorVisitor.java:22-22
  void visit_package_view_descriptor(PackageViewDescriptor& descriptor) override {
    if constexpr (std::is_void_v<R>) {
      visitor_.visit_package_view_descriptor(descriptor, data_);
    } else {
      result_.set(visitor_.visit_package_view_descriptor(descriptor, data_));
    }
  }
  // Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/DeclarationDescriptorVisitor.java:24-24
  void visit_variable_descriptor(VariableDescriptor& descriptor) override {
    if constexpr (std::is_void_v<R>) {
      visitor_.visit_variable_descriptor(descriptor, data_);
    } else {
      result_.set(visitor_.visit_variable_descriptor(descriptor, data_));
    }
  }
  // Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/DeclarationDescriptorVisitor.java:26-26
  void visit_function_descriptor(FunctionDescriptor& descriptor) override {
    if constexpr (std::is_void_v<R>) {
      visitor_.visit_function_descriptor(descriptor, data_);
    } else {
      result_.set(visitor_.visit_function_descriptor(descriptor, data_));
    }
  }
  // Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/DeclarationDescriptorVisitor.java:28-28
  void visit_type_parameter_descriptor(TypeParameterDescriptor& descriptor) override {
    if constexpr (std::is_void_v<R>) {
      visitor_.visit_type_parameter_descriptor(descriptor, data_);
    } else {
      result_.set(visitor_.visit_type_parameter_descriptor(descriptor, data_));
    }
  }
  // Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/DeclarationDescriptorVisitor.java:30-30
  void visit_class_descriptor(ClassDescriptor& descriptor) override {
    if constexpr (std::is_void_v<R>) {
      visitor_.visit_class_descriptor(descriptor, data_);
    } else {
      result_.set(visitor_.visit_class_descriptor(descriptor, data_));
    }
  }
  // Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/DeclarationDescriptorVisitor.java:32-32
  void visit_type_alias_descriptor(TypeAliasDescriptor& descriptor) override {
    if constexpr (std::is_void_v<R>) {
      visitor_.visit_type_alias_descriptor(descriptor, data_);
    } else {
      result_.set(visitor_.visit_type_alias_descriptor(descriptor, data_));
    }
  }
  // Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/DeclarationDescriptorVisitor.java:34-34
  void visit_module_declaration(ModuleDescriptor& descriptor) override {
    if constexpr (std::is_void_v<R>) {
      visitor_.visit_module_declaration(descriptor, data_);
    } else {
      result_.set(visitor_.visit_module_declaration(descriptor, data_));
    }
  }
  // Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/DeclarationDescriptorVisitor.java:36-36
  void visit_constructor_descriptor(ConstructorDescriptor& constructor_descriptor) override {
    if constexpr (std::is_void_v<R>) {
      visitor_.visit_constructor_descriptor(constructor_descriptor, data_);
    } else {
      result_.set(visitor_.visit_constructor_descriptor(constructor_descriptor, data_));
    }
  }
  // Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/DeclarationDescriptorVisitor.java:38-38
  void visit_script_descriptor(ScriptDescriptor& script_descriptor) override {
    if constexpr (std::is_void_v<R>) {
      visitor_.visit_script_descriptor(script_descriptor, data_);
    } else {
      result_.set(visitor_.visit_script_descriptor(script_descriptor, data_));
    }
  }
  // Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/DeclarationDescriptorVisitor.java:40-40
  void visit_property_descriptor(PropertyDescriptor& descriptor) override {
    if constexpr (std::is_void_v<R>) {
      visitor_.visit_property_descriptor(descriptor, data_);
    } else {
      result_.set(visitor_.visit_property_descriptor(descriptor, data_));
    }
  }
  // Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/DeclarationDescriptorVisitor.java:42-42
  void visit_value_parameter_descriptor(ValueParameterDescriptor& descriptor) override {
    if constexpr (std::is_void_v<R>) {
      visitor_.visit_value_parameter_descriptor(descriptor, data_);
    } else {
      result_.set(visitor_.visit_value_parameter_descriptor(descriptor, data_));
    }
  }
  // Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/DeclarationDescriptorVisitor.java:44-44
  void visit_property_getter_descriptor(PropertyGetterDescriptor& descriptor) override {
    if constexpr (std::is_void_v<R>) {
      visitor_.visit_property_getter_descriptor(descriptor, data_);
    } else {
      result_.set(visitor_.visit_property_getter_descriptor(descriptor, data_));
    }
  }
  // Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/DeclarationDescriptorVisitor.java:46-46
  void visit_property_setter_descriptor(PropertySetterDescriptor& descriptor) override {
    if constexpr (std::is_void_v<R>) {
      visitor_.visit_property_setter_descriptor(descriptor, data_);
    } else {
      result_.set(visitor_.visit_property_setter_descriptor(descriptor, data_));
    }
  }
  // Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/DeclarationDescriptorVisitor.java:48-48
  void visit_receiver_parameter_descriptor(ReceiverParameterDescriptor& descriptor) override {
    if constexpr (std::is_void_v<R>) {
      visitor_.visit_receiver_parameter_descriptor(descriptor, data_);
    } else {
      result_.set(visitor_.visit_receiver_parameter_descriptor(descriptor, data_));
    }
  }
 private:
  DeclarationDescriptorVisitor<R, D>& visitor_;
  D& data_;
  DescriptorVisitorResult<R> result_;
};

}  // namespace org::jetbrains::kotlin::descriptors::detail
