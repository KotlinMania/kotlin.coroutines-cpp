/*
 * Copyright 2010-2016 JetBrains s.r.o.
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
// port-lint: source compiler/ir/ir.tree/src/org/jetbrains/kotlin/ir/IrElementBase.kt
// port-lint: source compiler/ir/ir.tree/src/org/jetbrains/kotlin/ir/IrAttribute.kt
// Transliterated from: compiler/ir/ir.tree/src/org/jetbrains/kotlin/ir/IrElementBase.kt:23-190
#pragma once

#include "IrElement.hpp"
#include "../../../../kotlin/collections/Map.hpp"
#include "../../../../kotlin/Array.hpp"

namespace org::jetbrains::kotlin::ir {
template <typename E, typename T> class IrAttribute;
class IrElementBase;

namespace detail {
// NOTE(port): This private boundary represents IrAttribute<*, *> on the same
// actual key object. Only the source IrAttribute template can construct it.
// Shared handles preserve source array/map retention and canonical key identity.
// No key instance or debug/delegate implementation is supplied by this boundary.
// Transliterated from: compiler/ir/ir.tree/src/org/jetbrains/kotlin/ir/IrAttribute.kt:87-91
class IrAttributeObject {
 public:
  virtual ~IrAttributeObject() = default;
 protected:
  // Transliterated from: compiler/ir/ir.tree/src/org/jetbrains/kotlin/ir/IrAttribute.kt:90-90
  virtual bool copy_by_default() const = 0;
 private:
  IrAttributeObject() = default;
  IrAttributeObject(const IrAttributeObject&) = delete;
  IrAttributeObject& operator=(const IrAttributeObject&) = delete;
  template <typename, typename> friend class ::org::jetbrains::kotlin::ir::IrAttribute;
  friend class ::org::jetbrains::kotlin::ir::IrElementBase;
};
}  // namespace detail

// Transliterated from: compiler/ir/ir.tree/src/org/jetbrains/kotlin/ir/IrElementBase.kt:23-190
class IrElementBase : public virtual IrElement {
 public:
  /**
     * Returns a snapshot of all attributes held by this element.
     * Designated mainly for debugging.
     */
  // Transliterated from: compiler/ir/ir.tree/src/org/jetbrains/kotlin/ir/IrElementBase.kt:48-60
  std::shared_ptr<::kotlin::collections::Map<std::shared_ptr<detail::IrAttributeObject>, std::any>> attributes() const;

  // NOTE(port): Source internal generic operations retain their typed keys and
  // nullable results. Only the private storage algorithm erases the value type.
  // Transliterated from: compiler/ir/ir.tree/src/org/jetbrains/kotlin/ir/IrElementBase.kt:62-70
  template <typename E, typename T>
  typename ::kotlin::collections::detail::NullableMapValue<T>::Result get_attribute_internal(
      const std::shared_ptr<IrAttribute<E, T>>& key) const {
    const auto found_index = find_attribute_index(key);
    if (found_index < 0) {
      return {};
    } else {
      return ::kotlin::collections::detail::NullableMapValue<T>::take(
          attributes_.value().get(found_index + 1).value());
    }
  }

  // Transliterated from: compiler/ir/ir.tree/src/org/jetbrains/kotlin/ir/IrElementBase.kt:72-81
  template <typename E, typename T>
  typename ::kotlin::collections::detail::NullableMapValue<T>::Result set_attribute_internal(
      const std::shared_ptr<IrAttribute<E, T>>& key,
      typename ::kotlin::collections::detail::NullableMapValue<T>::Result value) {
    const auto found_index = find_attribute_index(key);
    const auto previous_value = found_index >= 0
        ? ::kotlin::collections::detail::NullableMapValue<T>::take(
              attributes_.value().get(found_index + 1).value())
        : typename ::kotlin::collections::detail::NullableMapValue<T>::Result{};
    put_attribute(found_index, key, box_attribute_value<T>(std::move(value)));
    return previous_value;
  }

  // Transliterated from: compiler/ir/ir.tree/src/org/jetbrains/kotlin/ir/IrElementBase.kt:156-189
  void copy_attributes_from(const IrElementBase& other, bool include_all);

 protected:
  IrElementBase() = default;
  IrElementBase(const IrElementBase&) = delete;
  IrElementBase& operator=(const IrElementBase&) = delete;
  // Transliterated from: compiler/ir/ir.tree/src/org/jetbrains/kotlin/ir/IrElementBase.kt:32-33
  void transform_dispatch(visitors::detail::IrTransformerDispatch& dispatch) override;
  // Transliterated from: compiler/ir/ir.tree/src/org/jetbrains/kotlin/ir/IrElementBase.kt:35-37
  void accept_children_dispatch(visitors::detail::IrVisitorDispatch& dispatch) override;
  // Transliterated from: compiler/ir/ir.tree/src/org/jetbrains/kotlin/ir/IrElementBase.kt:39-41
  void transform_children_dispatch(visitors::detail::IrTransformerDispatch& dispatch) override;

 private:
  /**
     * The array stores dense pairs of keys and values, followed by remaining nulls.
     * This is, the layout may look like this: `[key, value, key, value, null, null, null, ...]`
     * Keys are of type [IrAttribute].
     * Values are arbitrary objects but cannot be null.
     */
  // NOTE(port): Outer optional models the source absent allocation; inner
  // optional models initialized nullable slots in the actual C++ array. This
  // compiler storage is not Native ArrayHeader layout or a runtime GC object.
  // Transliterated from: compiler/ir/ir.tree/src/org/jetbrains/kotlin/ir/IrElementBase.kt:30-30
  std::optional<::kotlin::Array<std::optional<std::any>>> attributes_;

  // Transliterated from: compiler/ir/ir.tree/src/org/jetbrains/kotlin/ir/IrElementBase.kt:83-98
  std::int32_t find_attribute_index(const std::shared_ptr<detail::IrAttributeObject>& key) const;
  // Transliterated from: compiler/ir/ir.tree/src/org/jetbrains/kotlin/ir/IrElementBase.kt:100-106
  void initialize_attributes(const std::shared_ptr<detail::IrAttributeObject>& first_key, const std::any& first_value);
  // Transliterated from: compiler/ir/ir.tree/src/org/jetbrains/kotlin/ir/IrElementBase.kt:108-123
  void put_attribute(std::int32_t existing_index, const std::shared_ptr<detail::IrAttributeObject>& key,
                     const std::optional<std::any>& value);
  // Transliterated from: compiler/ir/ir.tree/src/org/jetbrains/kotlin/ir/IrElementBase.kt:125-135
  void add_attribute_at(std::int32_t index, const std::shared_ptr<detail::IrAttributeObject>& key, const std::any& value);
  // Transliterated from: compiler/ir/ir.tree/src/org/jetbrains/kotlin/ir/IrElementBase.kt:137-154
  void remove_attribute_at(std::int32_t key_index);

  // NOTE(port): Normalize source T? once at the existing generic virtual
  // boundary. Nullable pointer/handle/optional/Any types retain their null
  // representation; nonnullable T values arrive in optional<T>.
  // Transliterated from: compiler/ir/ir.tree/src/org/jetbrains/kotlin/ir/IrElementBase.kt:72-81
  template <typename T>
  static std::optional<std::any> box_attribute_value(
      typename ::kotlin::collections::detail::NullableMapValue<T>::Result value) {
    using Nullable = typename ::kotlin::collections::detail::NullableMapValue<T>::Result;
    if constexpr (std::is_same_v<T, std::any>) {
      return value.has_value() ? std::optional<std::any>(std::move(value)) : std::nullopt;
    } else if constexpr (std::is_same_v<Nullable, std::optional<T>>) {
      return value ? std::optional<std::any>(::kotlin::collections::detail::ElementCodec<T>::box(std::move(*value)))
                   : std::nullopt;
    } else {
      return value ? std::optional<std::any>(::kotlin::collections::detail::ElementCodec<T>::box(std::move(value)))
                   : std::nullopt;
    }
  }
};
}  // namespace org::jetbrains::kotlin::ir
