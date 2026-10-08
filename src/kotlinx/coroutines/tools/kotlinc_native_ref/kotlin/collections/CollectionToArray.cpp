/*
 * Copyright 2010-2023 JetBrains s.r.o. and Kotlin Programming Language contributors.
 * Use of this source code is governed by the Apache 2.0 license that can be found in the license/LICENSE.txt file.
 */
// port-lint: source libraries/stdlib/src/kotlin/collections/Collections.kt
// Transliterated from: libraries/stdlib/src/kotlin/collections/Collections.kt:514-545
#include "CollectionToArray.hpp"
#include "Arrays.hpp"
#include <bit>

namespace kotlin::collections {
// Transliterated from: libraries/stdlib/src/kotlin/collections/Collections.kt:514-526
Array<std::any> collection_to_array_common_impl(const Collection<std::any>& collection) {
  if (collection.is_empty()) return ::kotlin::empty_array();

  auto destination = ::kotlin::array_of_nulls<std::any>(collection.get_size());

  auto iterator = collection.iterator();
  std::int32_t index = 0;
  while (iterator->has_next()) {
    // NOTE(port): Kotlin evaluates the array/index receiver before next().
    // Its Int increment wraps; C++ must not invoke signed-overflow behavior.
    const auto current_index = index;
    index = std::bit_cast<std::int32_t>(static_cast<std::uint32_t>(index) + 1U);
    destination.set(current_index, iterator->next());
  }

  return destination;
}

// Transliterated from: libraries/stdlib/src/kotlin/collections/Collections.kt:528-545
Array<std::any> collection_to_array_common_impl(const Collection<std::any>& collection,
                                              Array<std::any> array) {
  if (collection.is_empty()) return terminate_collection_to_array(0, array);

  auto destination = array.get_size() < collection.get_size()
      ? array_of_nulls(array, collection.get_size()) : array;

  auto iterator = collection.iterator();
  std::int32_t index = 0;
  while (iterator->has_next()) {
    const auto current_index = index;
    index = std::bit_cast<std::int32_t>(static_cast<std::uint32_t>(index) + 1U);
    destination.set(current_index, iterator->next());
  }

  return terminate_collection_to_array(collection.get_size(), destination);
}

}  // namespace kotlin::collections
