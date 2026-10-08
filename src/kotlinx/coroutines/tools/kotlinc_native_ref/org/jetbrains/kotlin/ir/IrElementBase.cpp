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
// Transliterated from: compiler/ir/ir.tree/src/org/jetbrains/kotlin/ir/IrElementBase.kt:32-154
#include "IrElementBase.hpp"
#include "../../../../kotlin/collections/ArraysNative.hpp"

namespace org::jetbrains::kotlin::ir {
// Transliterated from: compiler/ir/ir.tree/src/org/jetbrains/kotlin/ir/IrElementBase.kt:32-33
void IrElementBase::transform_dispatch(visitors::detail::IrTransformerDispatch& dispatch) {
  dispatch.accept(*this);
}

// Transliterated from: compiler/ir/ir.tree/src/org/jetbrains/kotlin/ir/IrElementBase.kt:35-37
void IrElementBase::accept_children_dispatch(visitors::detail::IrVisitorDispatch&) {
  // No children by default
}

// Transliterated from: compiler/ir/ir.tree/src/org/jetbrains/kotlin/ir/IrElementBase.kt:39-41
void IrElementBase::transform_children_dispatch(visitors::detail::IrTransformerDispatch&) {
  // No children by default
}

// Transliterated from: compiler/ir/ir.tree/src/org/jetbrains/kotlin/ir/IrElementBase.kt:83-98
std::int32_t IrElementBase::find_attribute_index(const std::shared_ptr<detail::IrAttributeObject>& key) const {
  const auto attributes = attributes_;
  if (!attributes) return -1;

  std::int32_t i = 0;
  while (i < attributes->get_size()) {
    const auto found_key = attributes->get(i);
    if (!found_key) break;
    if (std::any_cast<std::shared_ptr<detail::IrAttributeObject>>(*found_key).get() == key.get()) {
      return i;
    }
    i += 2;
  }
  return ~i;
}

// Transliterated from: compiler/ir/ir.tree/src/org/jetbrains/kotlin/ir/IrElementBase.kt:100-106
void IrElementBase::initialize_attributes(const std::shared_ptr<detail::IrAttributeObject>& first_key,
                                          const std::any& first_value) {
  const std::int32_t initial_slots = 1;
  auto attributes = ::kotlin::array_of_nulls<std::optional<std::any>>(initial_slots * 2);
  attributes.set(0, std::any(first_key));
  attributes.set(1, first_value);
  attributes_ = attributes;
}

// Transliterated from: compiler/ir/ir.tree/src/org/jetbrains/kotlin/ir/IrElementBase.kt:108-123
void IrElementBase::put_attribute(std::int32_t existing_index,
                                  const std::shared_ptr<detail::IrAttributeObject>& key,
                                  const std::optional<std::any>& value) {
  if (existing_index >= 0) {
    if (!value) {
      remove_attribute_at(existing_index);
    } else {
      attributes_.value().set(existing_index + 1, value);
    }
  } else if (value) {
    if (!attributes_) {
      initialize_attributes(key, *value);
    } else {
      const auto new_entry_index = ~existing_index;
      add_attribute_at(new_entry_index, key, *value);
    }
  }
}

// Transliterated from: compiler/ir/ir.tree/src/org/jetbrains/kotlin/ir/IrElementBase.kt:125-135
void IrElementBase::add_attribute_at(std::int32_t index,
                                     const std::shared_ptr<detail::IrAttributeObject>& key,
                                     const std::any& value) {
  auto attributes = attributes_.value();
  if (attributes.get_size() <= index) {
    const std::int32_t new_slots = 2;
    // NOTE(port): Kotlin Int addition wraps; preserve it without C++ signed overflow.
    const auto new_size = std::bit_cast<std::int32_t>(
        static_cast<std::uint32_t>(attributes.get_size()) + static_cast<std::uint32_t>(new_slots * 2));
    attributes = ::kotlin::collections::copy_of(attributes, new_size);
    attributes_ = attributes;
  }
  attributes.set(index, std::any(key));
  attributes.set(index + 1, value);
}

// Transliterated from: compiler/ir/ir.tree/src/org/jetbrains/kotlin/ir/IrElementBase.kt:137-154
void IrElementBase::remove_attribute_at(std::int32_t key_index) {
  // It is expected that during the compilation process, attributes are mostly appended
  // and rarely removed, hence no need to shrink the array.
  auto attributes = attributes_.value();
  auto last_key_index = attributes.get_size() - 2;
  while (last_key_index > key_index && !attributes.get(last_key_index)) {
    last_key_index -= 2;
  }
  if (last_key_index > key_index) {
    attributes.set(key_index, attributes.get(last_key_index));
    attributes.set(key_index + 1, attributes.get(last_key_index + 1));
  }
  attributes.set(last_key_index, std::nullopt);
  attributes.set(last_key_index + 1, std::nullopt);
}
}  // namespace org::jetbrains::kotlin::ir
