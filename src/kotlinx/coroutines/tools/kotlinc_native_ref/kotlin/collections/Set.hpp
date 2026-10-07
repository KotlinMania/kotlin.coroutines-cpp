/*
 * Copyright 2010-2024 JetBrains s.r.o. and Kotlin Programming Language contributors.
 * Use of this source code is governed by the Apache 2.0 license that can be found in the license/LICENSE.txt file.
 */
// port-lint: source libraries/stdlib/native-wasm/src/kotlin/collections/Set.kt
// Transliterated from: libraries/stdlib/native-wasm/src/kotlin/collections/Set.kt:40-98
#pragma once
#include "MutableCollection.hpp"
namespace kotlin::collections {
template <typename E> class Set;
template <typename E> class MutableSet;
namespace detail {
// NOTE(port): Set requires structural equals/hash/text from implementations.
// Transliterated from: libraries/stdlib/native-wasm/src/kotlin/collections/Set.kt:40-51
class SetObject : public virtual CollectionObject {
 public:
// Transliterated from: libraries/stdlib/native-wasm/src/kotlin/collections/Set.kt:25-36
virtual bool equals(const std::any& other) const = 0;
// Transliterated from: libraries/stdlib/native-wasm/src/kotlin/collections/Set.kt:25-36
virtual std::int32_t hash_code() const = 0;
// Transliterated from: libraries/stdlib/native-wasm/src/kotlin/collections/Set.kt:25-36
virtual std::u16string to_string() const = 0;
};
// Transliterated from: libraries/stdlib/native-wasm/src/kotlin/collections/Set.kt:40-51
template <typename... Types>
struct CovariantBases<Set, std::tuple<Types...>>
    : public virtual SetObject, public virtual Set<Types>... {
  // Transliterated from: libraries/stdlib/native-wasm/src/kotlin/collections/Set.kt:42-42
  std::int32_t get_size() const override = 0;
  // Transliterated from: libraries/stdlib/native-wasm/src/kotlin/collections/Set.kt:44-44
  bool is_empty() const override = 0;
};
}  // namespace detail
/**
 * A generic unordered collection of unique elements. The interface allows checking if an element is contained by it
 * and iterating over all elements. Complex operations are built upon this functionality
 * and provided in form of [kotlin.collections] extension functions.
 *
 * It is implementation-specific how [Set] defines element's uniqueness. If not stated otherwise, [Set] implementations are usually
 * distinguishing elements using [Any.equals]. However, it is not the only way to distinguish elements, and some implementations may use
 * referential equality or compare elements by some of their properties. It is recommended to explicitly specify how a class
 * implementing [Set] distinguish elements.
 *
 * Methods in this interface support only read-only access to the set;
 * read/write access is supported through the [MutableSet] interface.
 *
 * Unlike [List], [Set] does not guarantee any particular order for iteration. However, particular implementations
 * are free to have fixed iteration order, like "smaller", in some sense, elements are visited prior to "larger". In this case,
 * it is recommended to explicitly document ordering guarantees for the [Set] implementation.
 *
 * Unlike [Collection] implementations, [Set] implementations must override [Any.toString], [Any.equals] and [Any.hashCode] functions
 * and provide implementations such that:
 * - [Set.toString] should return a string containing string representation of contained elements in iteration order.
 * - [Set.equals] should consider two sets equal if and only if they contain the same number of elements and each element
 *   from one set is contained in another set. Unlike some other `equals` implementations, [Set.equals]
 *   should consider two sets equal even if they are instances of different classes; the only requirement here is that both sets have
 *   to implement [Set] interface.
 * - [Set.hashCode] should be computed as a sum of elements' hash codes using the following algorithm:
 *   ```kotlin
 *   var hashCode: Int = 0
 *   for (element in this) hashCode += element.hashCode()
 *   ```
 *
 * @param E the type of elements contained in the set. The set is covariant in its element type.
 */
// Transliterated from: libraries/stdlib/native-wasm/src/kotlin/collections/Set.kt:40-51
template <typename E>
class Set : public virtual Collection<E>, public virtual detail::SetObject,
    public detail::CovariantBases<Set, typename detail::ElementSupertypes<E>::Types> {
 public:
// Transliterated from: libraries/stdlib/native-wasm/src/kotlin/collections/Set.kt:42-42
std::int32_t get_size() const override = 0;
// Transliterated from: libraries/stdlib/native-wasm/src/kotlin/collections/Set.kt:44-44
bool is_empty() const override = 0;
// Transliterated from: libraries/stdlib/native-wasm/src/kotlin/collections/Set.kt:45-45
bool contains(E element) const { return Collection<E>::contains(std::move(element)); }
// Transliterated from: libraries/stdlib/native-wasm/src/kotlin/collections/Set.kt:46-46
std::unique_ptr<Iterator<E>> iterator() const { return Collection<E>::iterator(); }
// Transliterated from: libraries/stdlib/native-wasm/src/kotlin/collections/Set.kt:49-49
bool contains_all(const Collection<E>& elements) const { return Collection<E>::contains_all(elements); }
};
/**
 * A generic unordered collection of unique elements that supports adding and removing elements, iterating over them
 * and checking if a collection contains a particular value.
 *
 * If a particular use case does not require set's modification,
 * a read-only counterpart, [Set] could be used instead.
 *
 * [MutableSet] extends [Set] contact with functions allowing to add and remove elements.
 *
 * Unlike [Set], an iterator returned by [iterator] allows modifying the set during iteration.
 *
 * Until stated otherwise, [MutableSet] implementations are not thread-safe and their modification without
 * explicit synchronization may result in data corruption, loss, and runtime errors.
 *
 * @param E the type of elements contained in the set. The mutable set is invariant in its element type.
 */
// Transliterated from: libraries/stdlib/native-wasm/src/kotlin/collections/Set.kt:68-98
template <typename E>
class MutableSet : public virtual Set<E>, public virtual MutableCollection<E> {
 public:
// Transliterated from: libraries/stdlib/native-wasm/src/kotlin/collections/Set.kt:70-70
std::unique_ptr<MutableIterator<E>> iterator() const { return MutableCollection<E>::iterator(); }
/**
     * Adds the specified element to the set.
     *
     * If the set doesn't contain [element], it is added to the set and the function returns `true`.
     * If the set already contains [element], the element instance stored in the set is kept, [element] is not
     * added, and the function returns `false`.
     *
     * @sample samples.collections.Collections.Sets.add
     */
// Transliterated from: libraries/stdlib/native-wasm/src/kotlin/collections/Set.kt:84-84
bool add(E element) { return MutableCollection<E>::add(std::move(element)); }
// Transliterated from: libraries/stdlib/native-wasm/src/kotlin/collections/Set.kt:87-87
bool remove(E element) { return MutableCollection<E>::remove(std::move(element)); }
// Transliterated from: libraries/stdlib/native-wasm/src/kotlin/collections/Set.kt:91-91
bool add_all(const Collection<E>& elements) { return MutableCollection<E>::add_all(elements); }
// Transliterated from: libraries/stdlib/native-wasm/src/kotlin/collections/Set.kt:94-94
bool remove_all(const Collection<E>& elements) { return MutableCollection<E>::remove_all(elements); }
// Transliterated from: libraries/stdlib/native-wasm/src/kotlin/collections/Set.kt:96-96
bool retain_all(const Collection<E>& elements) { return MutableCollection<E>::retain_all(elements); }
// Transliterated from: libraries/stdlib/native-wasm/src/kotlin/collections/Set.kt:97-97
void clear() override = 0;
};
}  // namespace kotlin::collections
