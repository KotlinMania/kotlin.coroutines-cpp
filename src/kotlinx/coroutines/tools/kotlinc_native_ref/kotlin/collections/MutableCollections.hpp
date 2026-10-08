/*
 * Copyright 2010-2020 JetBrains s.r.o. and Kotlin Programming Language contributors.
 * Use of this source code is governed by the Apache 2.0 license that can be found in the license/LICENSE.txt file.
 */
// port-lint: source libraries/stdlib/src/kotlin/collections/MutableCollections.kt
// Transliterated from: libraries/stdlib/src/kotlin/collections/MutableCollections.kt:219-245
// Transliterated from: libraries/stdlib/src/kotlin/collections/MutableCollections.kt:284-323
#pragma once

#include "MutableList.hpp"
#include "RandomAccess.hpp"
#include <bit>
#include <concepts>
#include <functional>

namespace kotlin::collections {
namespace detail {
// NOTE(port): These private generic bodies must be visible to the public generic
// predicate overloads. Predicate references retain the same callable and capture
// state through the source's delegation; no collection or iterator is replaced.
// Transliterated from: libraries/stdlib/src/kotlin/collections/MutableCollections.kt:235-245
template <typename T, typename Predicate>
bool filter_in_place(MutableIterable<T>& iterable, Predicate& predicate,
                     bool predicate_result_to_remove) {
  bool result = false;
  auto iterator = iterable.iterator();
  while (iterator->has_next()) {
    if (std::invoke(predicate, iterator->next()) == predicate_result_to_remove) {
      iterator->remove();
      result = true;
    }
  }
  return result;
}

// Transliterated from: libraries/stdlib/src/kotlin/collections/MutableCollections.kt:300-323
template <typename T, typename Predicate>
bool filter_in_place(MutableList<T>& list, Predicate& predicate,
                     bool predicate_result_to_remove) {
  if (dynamic_cast<RandomAccess*>(&list) == nullptr) {
    return filter_in_place(static_cast<MutableIterable<T>&>(list), predicate,
                           predicate_result_to_remove);
  }

  std::int32_t write_index = 0;
  // NOTE(port): Evaluate each source range endpoint once. Widened loop cursors
  // preserve an inclusive Int progression even at INT_MAX; Kotlin Int arithmetic
  // for lastIndex and writeIndex wraps without C++ signed-overflow undefined behavior.
  const auto last_index = std::bit_cast<std::int32_t>(
      static_cast<std::uint32_t>(list.get_size()) - 1U);
  for (std::int64_t read_index = 0; read_index <= last_index; ++read_index) {
    const auto element = list.get(static_cast<std::int32_t>(read_index));
    if (std::invoke(predicate, element) == predicate_result_to_remove) continue;

    if (write_index != read_index) list.set(write_index, element);
    write_index = std::bit_cast<std::int32_t>(
        static_cast<std::uint32_t>(write_index) + 1U);
  }
  if (write_index < list.get_size()) {
    const auto last_remove_index = std::bit_cast<std::int32_t>(
        static_cast<std::uint32_t>(list.get_size()) - 1U);
    for (std::int64_t remove_index = last_remove_index;
         remove_index >= write_index; --remove_index) {
      list.remove_at(static_cast<std::int32_t>(remove_index));
    }
    return true;
  } else {
    return false;
  }
}
}  // namespace detail

/**
 * Removes all elements from this [MutableIterable] that match the given [predicate].
 *
 * @return `true` if any element was removed from this collection, or `false` when no elements were removed and collection was not modified.
 */
// Transliterated from: libraries/stdlib/src/kotlin/collections/MutableCollections.kt:225-225
template <typename T, typename Predicate> requires std::predicate<Predicate&, T>
bool remove_all(MutableIterable<T>& iterable, Predicate&& predicate) {
  return detail::filter_in_place(iterable, predicate, true);
}

/**
 * Retains only elements of this [MutableIterable] that match the given [predicate].
 *
 * @return `true` if any element was removed from this collection, or `false` when all elements were retained and collection was not modified.
 */
// Transliterated from: libraries/stdlib/src/kotlin/collections/MutableCollections.kt:233-233
template <typename T, typename Predicate> requires std::predicate<Predicate&, T>
bool retain_all(MutableIterable<T>& iterable, Predicate&& predicate) {
  return detail::filter_in_place(iterable, predicate, false);
}

/**
 * Removes all elements from this [MutableList] that match the given [predicate].
 *
 * @return `true` if any element was removed from this collection, or `false` when no elements were removed and collection was not modified.
 */
// Transliterated from: libraries/stdlib/src/kotlin/collections/MutableCollections.kt:290-290
template <typename T, typename Predicate> requires std::predicate<Predicate&, T>
bool remove_all(MutableList<T>& list, Predicate&& predicate) {
  return detail::filter_in_place(list, predicate, true);
}

/**
 * Retains only elements of this [MutableList] that match the given [predicate].
 *
 * @return `true` if any element was removed from this collection, or `false` when all elements were retained and collection was not modified.
 */
// Transliterated from: libraries/stdlib/src/kotlin/collections/MutableCollections.kt:298-298
template <typename T, typename Predicate> requires std::predicate<Predicate&, T>
bool retain_all(MutableList<T>& list, Predicate&& predicate) {
  return detail::filter_in_place(list, predicate, false);
}
}  // namespace kotlin::collections
