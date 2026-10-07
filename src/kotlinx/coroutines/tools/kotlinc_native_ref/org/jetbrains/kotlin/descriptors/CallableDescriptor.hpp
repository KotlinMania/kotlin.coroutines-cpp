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
// Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/CallableDescriptor.java:28-78
#pragma once

#include "DeclarationDescriptorWithVisibility.hpp"
#include "DeclarationDescriptorNonRoot.hpp"
#include "Substitutable.hpp"
#include "DescriptorCollectionTypes.hpp"
#include "../../../../kotlin/collections/Collections.hpp"

#include <any>
#include <memory>
#include <optional>

namespace org::jetbrains::kotlin::types { class KotlinType; }
namespace org::jetbrains::kotlin::descriptors {
class ReceiverParameterDescriptor;
class TypeParameterDescriptor;
class ValueParameterDescriptor;

namespace detail {
// NOTE(port): The source methodless generic key keeps object identity. A private
// common base permits C++ virtual dispatch without erasing the public key type.
  // Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/CallableDescriptor.java:73-73
class UserDataKeyObject {
 public:
  virtual ~UserDataKeyObject() = default;
 protected:
  UserDataKeyObject() = default;
  UserDataKeyObject(const UserDataKeyObject&) = delete;
  UserDataKeyObject& operator=(const UserDataKeyObject&) = delete;
};
}  // namespace detail

  // Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/CallableDescriptor.java:28-78
class CallableDescriptor : public virtual DeclarationDescriptorWithVisibility,
                           public virtual DeclarationDescriptorNonRoot,
                           public virtual Substitutable<CallableDescriptor>,
                           public virtual mpp::CallableSymbolMarker {
 public:
  // Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/CallableDescriptor.java:30-32
  virtual const ::kotlin::collections::List<ReceiverParameterDescriptor*>& get_context_receiver_parameters() const = 0;
  // Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/CallableDescriptor.java:34-35
  virtual ReceiverParameterDescriptor* get_extension_receiver_parameter() const = 0;
  // Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/CallableDescriptor.java:37-38
  virtual ReceiverParameterDescriptor* get_dispatch_receiver_parameter() const = 0;
  // Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/CallableDescriptor.java:40-42
  virtual const ::kotlin::collections::List<TypeParameterDescriptor*>& get_type_parameters() const = 0;
    /**
     * Method may return null for not yet fully initialized object or if error occurred.
     */
  // Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/CallableDescriptor.java:44-48
  virtual types::KotlinType* get_return_type() const = 0;
  // Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/CallableDescriptor.java:50-53
  CallableDescriptor& get_original() const override = 0;
  // Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/CallableDescriptor.java:55-56
  virtual const ::kotlin::collections::List<ValueParameterDescriptor*>& get_value_parameters() const = 0;
    /**
     * Kotlin functions always have stable parameter names that can be reliably used when calling them with named arguments.
     * Functions loaded from platform definitions (e.g. Java binaries or JS) may have unstable parameter names that vary from
     * one platform installation to another. These names can not be used reliably for calls with named arguments.
     */
  // Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/CallableDescriptor.java:57-62
  virtual bool has_stable_parameter_names() const = 0;
    /**
     * Sometimes parameter names are not available at all (e.g. Java binaries with not enough debug information).
     * In this case, getName() returns synthetic names such as "p0", "p1" etc.
     */
  // Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/CallableDescriptor.java:64-68
  virtual bool has_synthesized_parameter_names() const = 0;
  // NOTE(port): A private virtual boundary retains the actual covariant
  // collection object. Shared results own newly constructed source collections;
  // stored results may share their existing compiler-owned collection.
  // Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/CallableDescriptor.java:71-72
  std::shared_ptr<const ::kotlin::collections::Collection<CallableDescriptor*>>
  get_overridden_descriptors() const {
    auto object = get_overridden_descriptors_dispatch();
    auto& typed = dynamic_cast<const ::kotlin::collections::Collection<CallableDescriptor*>&>(*object);
    return {std::move(object), &typed};
  }
  // Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/CallableDescriptor.java:73-73
  template <typename V> class UserDataKey : public detail::UserDataKeyObject {
   protected:
    UserDataKey() = default;
  };
  // NOTE(port): C++ has no virtual generic function. The typed key and nullable
  // value remain public; the source implementation supplies the erased value.
  // Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/CallableDescriptor.java:76-77
  template <typename V>
  std::optional<V> get_user_data(const UserDataKey<V>& key) const {
    auto value = get_user_data_dispatch(key);
    if (!value.has_value()) return std::nullopt;
    return std::any_cast<V>(std::move(value));
  }
 protected:
  // Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/CallableDescriptor.java:71-72
  virtual std::shared_ptr<const ::kotlin::collections::detail::CollectionObject>
  get_overridden_descriptors_dispatch() const = 0;
  // Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/CallableDescriptor.java:76-77
  virtual std::any get_user_data_dispatch(const detail::UserDataKeyObject& key) const = 0;
};

}  // namespace org::jetbrains::kotlin::descriptors
