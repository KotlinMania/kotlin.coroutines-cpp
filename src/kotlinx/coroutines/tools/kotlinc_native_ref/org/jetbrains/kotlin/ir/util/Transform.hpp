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
// port-lint: source compiler/ir/ir.tree/src/org/jetbrains/kotlin/ir/util/transform.kt
// Transliterated from: compiler/ir/ir.tree/src/org/jetbrains/kotlin/ir/util/transform.kt:31-137
#pragma once
#include "../IrElementBase.hpp"
#include "../../../../../kotlin/collections/MutableList.hpp"
#include "../../../../../kotlin/collections/CollectionFunctions.hpp"
#include <bit>
#include <concepts>
#include <memory>
// NOTE(port): This is the real source class dependency, not a backing-list
// implementation. Copying still requires the untranslated actual ArrayList.
// Transliterated from: libraries/stdlib/native-wasm/src/kotlin/collections/ArrayList.kt:47-61
namespace kotlin::collections { template <typename E> class ArrayList; }
namespace org::jetbrains::kotlin::ir::declarations {
class IrDeclaration;
class IrTypeParameter;
}
namespace org::jetbrains::kotlin::ir::util {
namespace detail {
// Transliterated from: compiler/ir/ir.tree/src/org/jetbrains/kotlin/ir/util/transform.kt:31-38
template <typename T> requires std::derived_from<T, IrElement>
void transform_in_place_nodes(::kotlin::collections::MutableList<T*>& list,
                              visitors::detail::IrTransformerDispatch& transformer) {
  const auto size = list.get_size();
  for (std::int32_t i = 0; i < size; ++i) {
    // Cast to IrElementBase to avoid casting to interface and invokeinterface, both of which are slow.
    auto& element = dynamic_cast<IrElementBase&>(*list.get(i));
    list.set(i, &dynamic_cast<T&>(transformer.transform(element)));
  }
}
// Transliterated from: compiler/ir/ir.tree/src/org/jetbrains/kotlin/ir/util/transform.kt:126-137
// Transliterated from: libraries/stdlib/common/src/generated/_Collections.kt:1835-1837
// Transliterated from: libraries/stdlib/src/kotlin/collections/Iterables.kt:24-26
// Transliterated from: libraries/stdlib/src/kotlin/collections/Iterators.kt:38-42
template <typename T> requires std::derived_from<T, IrElement>
std::shared_ptr<::kotlin::collections::List<T*>> transform_if_needed_nodes(
    const std::shared_ptr<::kotlin::collections::List<T*>>& list,
    visitors::detail::IrTransformerDispatch& transformer) {
  std::shared_ptr<::kotlin::collections::ArrayList<T*>> result;
  // NOTE(port): Lower the source withIndex wrapper to its exact iterator/index
  // operations. UInt storage preserves Int wrapping, including overflow checks
  // before iterator.next(); list and node identities retain existing ownership.
  auto iterator = list->iterator();
  std::uint32_t index = 0;
  while (iterator->has_next()) {
    const auto i = ::kotlin::collections::check_index_overflow(std::bit_cast<std::int32_t>(index++));
    auto* item = iterator->next();
    auto* transformed = &dynamic_cast<T&>(transformer.transform(*item));
    if (transformed != item && result == nullptr) {
      result = std::make_shared<::kotlin::collections::ArrayList<T*>>(*list);
    }
    if (result != nullptr) result->set(i, transformed);
  }
  return result != nullptr ? std::shared_ptr<::kotlin::collections::List<T*>>(result) : list;
}
}  // namespace detail
// Transliterated from: compiler/ir/ir.tree/src/org/jetbrains/kotlin/ir/util/transform.kt:31-38
template <typename T, typename D> requires std::derived_from<T, IrElement>
void transform_in_place(::kotlin::collections::MutableList<T*>& list,
                         visitors::IrTransformer<D>& transformer, D data) {
  visitors::detail::TypedIrTransformerDispatch<D> dispatch(transformer, data);
  detail::transform_in_place_nodes(list, dispatch);
}
/**
 * Transforms the list of elements with the given transformer. Return the same List instance if no element instances have changed.
 */
// Transliterated from: compiler/ir/ir.tree/src/org/jetbrains/kotlin/ir/util/transform.kt:126-137
template <typename T, typename D> requires std::derived_from<T, IrElement>
std::shared_ptr<::kotlin::collections::List<T*>> transform_if_needed(
    const std::shared_ptr<::kotlin::collections::List<T*>>& list,
    visitors::IrTransformer<D>& transformer, D data) {
  visitors::detail::TypedIrTransformerDispatch<D> dispatch(transformer, data);
  return detail::transform_if_needed_nodes(list, dispatch);
}
// NOTE(port): The existing generic virtual dispatch boundary calls these two
// actual declaration specializations. Their nongeneric bodies are in .cpp.
// Transliterated from: compiler/ir/ir.tree/src/org/jetbrains/kotlin/ir/util/transform.kt:31-38
void transform_in_place(::kotlin::collections::MutableList<declarations::IrDeclaration*>& list,
                         visitors::detail::IrTransformerDispatch& transformer);
// Transliterated from: compiler/ir/ir.tree/src/org/jetbrains/kotlin/ir/util/transform.kt:126-137
std::shared_ptr<::kotlin::collections::List<declarations::IrTypeParameter*>> transform_if_needed(
    const std::shared_ptr<::kotlin::collections::List<declarations::IrTypeParameter*>>& list,
    visitors::detail::IrTransformerDispatch& transformer);
}  // namespace org::jetbrains::kotlin::ir::util
