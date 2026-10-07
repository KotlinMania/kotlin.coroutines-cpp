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
// port-lint: source core/descriptors/src/org/jetbrains/kotlin/descriptors/CallableDescriptor.java
// Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/CallableDescriptor.java:28-77
#pragma once

#include "DeclarationDescriptor.hpp"
#include "../../../../kotlin/collections/CollectionElement.hpp"

namespace org::jetbrains::kotlin::descriptors {
class CallableDescriptor;
class CallableMemberDescriptor;
class MemberDescriptor;
class ValueDescriptor;
class VariableDescriptor;
class ParameterDescriptor;
class ValueParameterDescriptor;
}

namespace kotlin::collections::detail {
// NOTE(port): These are the consumed source descriptor covariance edges. Each
// collection remains the same object; no copied vector or alternate API is used.
  // Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/CallableDescriptor.java:28-29
template <> struct ElementSupertypes<org::jetbrains::kotlin::descriptors::CallableDescriptor*> {
  using Types = std::tuple<org::jetbrains::kotlin::descriptors::DeclarationDescriptor*>;
};
// Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/MemberDescriptor.java:21-21
template <> struct ElementSupertypes<org::jetbrains::kotlin::descriptors::MemberDescriptor*> {
  using Types = std::tuple<org::jetbrains::kotlin::descriptors::DeclarationDescriptor*>;
};
// Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/CallableMemberDescriptor.java:28-28
template <> struct ElementSupertypes<org::jetbrains::kotlin::descriptors::CallableMemberDescriptor*> {
  using Types = std::tuple<org::jetbrains::kotlin::descriptors::CallableDescriptor*,
                           org::jetbrains::kotlin::descriptors::MemberDescriptor*>;
};
  // Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/ValueDescriptor.java:22-29
template <> struct ElementSupertypes<org::jetbrains::kotlin::descriptors::ValueDescriptor*> {
  using Types = std::tuple<org::jetbrains::kotlin::descriptors::CallableDescriptor*>;
};
  // Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/VariableDescriptor.java:25-46
template <> struct ElementSupertypes<org::jetbrains::kotlin::descriptors::VariableDescriptor*> {
  using Types = std::tuple<org::jetbrains::kotlin::descriptors::ValueDescriptor*>;
};
  // Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/ParameterDescriptor.java:21-25
template <> struct ElementSupertypes<org::jetbrains::kotlin::descriptors::ParameterDescriptor*> {
  using Types = std::tuple<org::jetbrains::kotlin::descriptors::ValueDescriptor*>;
};
  // Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/ValueParameterDescriptor.kt:24-60
template <> struct ElementSupertypes<org::jetbrains::kotlin::descriptors::ValueParameterDescriptor*> {
  using Types = std::tuple<org::jetbrains::kotlin::descriptors::VariableDescriptor*, org::jetbrains::kotlin::descriptors::ParameterDescriptor*>;
};

// NOTE(port): Reference-object elements cross the erased virtual boundary using
// the existing canonical DeclarationDescriptor boxing. Source checked casts
// recover the original object, including adjustment for multiple inheritance.
template <typename T> struct DescriptorElementCodec {
  static std::any box(T* value) {
    if (value == nullptr) return std::any{};
    return std::cref(static_cast<const org::jetbrains::kotlin::descriptors::DeclarationDescriptor&>(*value));
  }
  static T* unbox(const std::any& value) {
    if (!value.has_value()) return nullptr;
    const auto& descriptor = std::any_cast<std::reference_wrapper<const
        org::jetbrains::kotlin::descriptors::DeclarationDescriptor>>(value).get();
    // Kotlin references remain mutable handles when read through a const C++
    // collection view. This reverses only the boxing view's const qualification.
    return &const_cast<T&>(dynamic_cast<const T&>(descriptor));
  }
};
template <> struct ElementCodec<org::jetbrains::kotlin::descriptors::DeclarationDescriptor*>
    : DescriptorElementCodec<org::jetbrains::kotlin::descriptors::DeclarationDescriptor> {};
template <> struct ElementCodec<org::jetbrains::kotlin::descriptors::CallableDescriptor*>
    : DescriptorElementCodec<org::jetbrains::kotlin::descriptors::CallableDescriptor> {};
template <> struct ElementCodec<org::jetbrains::kotlin::descriptors::MemberDescriptor*>
    : DescriptorElementCodec<org::jetbrains::kotlin::descriptors::MemberDescriptor> {};
template <> struct ElementCodec<org::jetbrains::kotlin::descriptors::CallableMemberDescriptor*>
    : DescriptorElementCodec<org::jetbrains::kotlin::descriptors::CallableMemberDescriptor> {};
template <> struct ElementCodec<org::jetbrains::kotlin::descriptors::ValueDescriptor*>
    : DescriptorElementCodec<org::jetbrains::kotlin::descriptors::ValueDescriptor> {};
template <> struct ElementCodec<org::jetbrains::kotlin::descriptors::VariableDescriptor*>
    : DescriptorElementCodec<org::jetbrains::kotlin::descriptors::VariableDescriptor> {};
template <> struct ElementCodec<org::jetbrains::kotlin::descriptors::ParameterDescriptor*>
    : DescriptorElementCodec<org::jetbrains::kotlin::descriptors::ParameterDescriptor> {};
template <> struct ElementCodec<org::jetbrains::kotlin::descriptors::ValueParameterDescriptor*>
    : DescriptorElementCodec<org::jetbrains::kotlin::descriptors::ValueParameterDescriptor> {};

}  // namespace kotlin::collections::detail
