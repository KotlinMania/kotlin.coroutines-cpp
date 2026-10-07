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
// port-lint: source core/descriptors/src/org/jetbrains/kotlin/descriptors/DeclarationDescriptor.java
// Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/DeclarationDescriptor.java:24-40
#pragma once

#include "annotations/Annotated.hpp"
#include "Named.hpp"
#include "ValidateableDescriptor.hpp"
#include "DescriptorVisitorDispatch.hpp"
#include "../mpp/DeclarationSymbolMarkers.hpp"

#include <any>
#include <cstddef>
#include <cstdint>
#include <string>

namespace org::jetbrains::kotlin::descriptors {

// Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/DeclarationDescriptor.java:24-40
class DeclarationDescriptor : public virtual annotations::Annotated,
                              public virtual Named,
                              public virtual ValidateableDescriptor,
                              public virtual mpp::DeclarationSymbolMarker {
 public:
  /**
   * @return The descriptor that corresponds to the original declaration of this element.
   *         A descriptor can be obtained from its original by substituting type arguments (of the declaring class
   *         or of the element itself).
   *         returns <code>this</code> object if the current descriptor is original itself
   */
  // Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/DeclarationDescriptor.java:25-32
  virtual DeclarationDescriptor& get_original() const = 0;
  // Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/DeclarationDescriptor.java:34-35
  virtual DeclarationDescriptor* get_containing_declaration() const = 0;

  // NOTE(port): Source generic dispatch crosses a private virtual boundary;
  // R and D remain typed. The descriptor chooses its exact source visit method.
  // Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/DeclarationDescriptor.java:37-37
  template <typename R, typename D>
  R accept(DeclarationDescriptorVisitor<R, D>& visitor, D data) {
    detail::TypedDescriptorVisitorDispatch<R, D> dispatch(visitor, data);
    accept_dispatch(dispatch);
    return dispatch.take_result();
  }

  // NOTE(port): Java Void maps to void for results, nullptr_t for the null context.
  // Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/DeclarationDescriptor.java:39-39
  virtual void accept_void(DeclarationDescriptorVisitor<void, std::nullptr_t>& visitor) = 0;

  // NOTE(port): Java interfaces inherit Object's equality/hash/string contract.
  // Compiler reference objects are boxed as reference_wrapper<const Descriptor>
  // in any. C++ object identity/hash and RTTI supply the platform implementation;
  // these are compiler objects, not a Kotlin/Native runtime object layout.
  // Transliterated from: libraries/stdlib/jvm/builtins/Any.kt:28-28
  virtual bool equals(const std::any& other) const;
  // Transliterated from: libraries/stdlib/jvm/builtins/Any.kt:37-37
  virtual std::int32_t hash_code() const;
  // Transliterated from: libraries/stdlib/jvm/builtins/Any.kt:42-42
  virtual std::string to_string() const;
  // NOTE(port): Lower Kotlin structural == through its virtual equals method.
  // Transliterated from: libraries/stdlib/jvm/builtins/Any.kt:28-28
  bool operator==(const DeclarationDescriptor& other) const;

 protected:
  DeclarationDescriptor() = default;
  // NOTE(port): Kotlin/Java descriptors are reference objects with stable identity.
  DeclarationDescriptor(const DeclarationDescriptor&) = delete;
  DeclarationDescriptor& operator=(const DeclarationDescriptor&) = delete;
  virtual void accept_dispatch(detail::DescriptorVisitorDispatch& dispatch) = 0;
};

}  // namespace org::jetbrains::kotlin::descriptors
