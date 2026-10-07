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
// Transliterated from: compiler/ir/ir.tree/src/org/jetbrains/kotlin/ir/IrElementBase.kt:48-60
// Transliterated from: compiler/ir/ir.tree/src/org/jetbrains/kotlin/ir/IrElementBase.kt:156-189
#include "IrElementBase.hpp"
#include "../../../../kotlin/collections/ArraysNative.hpp"
#include "../../../../kotlin/collections/Maps.hpp"
#include "../../../../java/util/IdentityHashMap.hpp"

namespace org::jetbrains::kotlin::ir {
// Transliterated from: compiler/ir/ir.tree/src/org/jetbrains/kotlin/ir/IrElementBase.kt:48-60
std::shared_ptr<::kotlin::collections::Map<std::shared_ptr<detail::IrAttributeObject>, std::any>>
IrElementBase::attributes() const {
  const auto attributes = attributes_;
  if (!attributes) {
    return ::kotlin::collections::empty_map<std::shared_ptr<detail::IrAttributeObject>, std::any>();
  }
  return ::kotlin::collections::build_map<std::shared_ptr<detail::IrAttributeObject>, std::any>(
      attributes->get_size() / 2, [&](auto& map) {
        for (std::int32_t i = 0; i < attributes->get_size(); i += 2) {
          const auto key_slot = attributes->get(i);
          if (!key_slot) break;
          const auto key = std::any_cast<std::shared_ptr<detail::IrAttributeObject>>(*key_slot);
          const auto value = attributes->get(i + 1).value();
          map.put(key, value);
        }
      });
}

// Transliterated from: compiler/ir/ir.tree/src/org/jetbrains/kotlin/ir/IrElementBase.kt:156-189
void IrElementBase::copy_attributes_from(const IrElementBase& other, bool include_all) {
  const auto src_attributes = other.attributes_;
  if (!src_attributes) return;
  auto dst_attributes = attributes_;
  // NOTE(port): Kotlin Int arithmetic wraps before division/allocation.
  const auto combined_slots = std::bit_cast<std::int32_t>(
      static_cast<std::uint32_t>(src_attributes->get_size()) +
      static_cast<std::uint32_t>(dst_attributes ? dst_attributes->get_size() : 0));
  ::java::util::IdentityHashMap<std::shared_ptr<detail::IrAttributeObject>, std::optional<std::any>>
      merged_attributes(combined_slots / 2);

  if (dst_attributes) {
    for (std::int32_t i = 0; i < dst_attributes->get_size(); i += 2) {
      const auto attr_slot = dst_attributes->get(i);
      if (!attr_slot) break;
      const auto attr = std::any_cast<std::shared_ptr<detail::IrAttributeObject>>(*attr_slot);
      merged_attributes.put(attr, dst_attributes->get(i + 1));
    }
  }
  for (std::int32_t i = 0; i < src_attributes->get_size(); i += 2) {
    const auto attr_slot = src_attributes->get(i);
    if (!attr_slot) break;
    const auto attr = std::any_cast<std::shared_ptr<detail::IrAttributeObject>>(*attr_slot);
    if (attr->copy_by_default() || include_all) {
      merged_attributes.put(attr, src_attributes->get(i + 1));
    }
  }

  if (merged_attributes.is_empty()) return;
  const auto merged_slots = std::bit_cast<std::int32_t>(static_cast<std::uint32_t>(merged_attributes.size()) * 2U);
  if (!dst_attributes || dst_attributes->get_size() <= merged_slots) {
    dst_attributes = ::kotlin::array_of_nulls<std::optional<std::any>>(merged_slots);
    attributes_ = dst_attributes;
  }
  std::int32_t i = 0;
  auto entries = merged_attributes.entry_set().iterator();
  while (entries->has_next()) {
    const auto entry = entries->next();
    dst_attributes->set(i, std::any(entry->get_key()));
    dst_attributes->set(i + 1, entry->get_value());
    i += 2;
  }
}
}  // namespace org::jetbrains::kotlin::ir
