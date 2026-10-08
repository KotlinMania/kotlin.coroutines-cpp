/*
 * Copyright 2010-2024 JetBrains s.r.o. and Kotlin Programming Language contributors.
 * Use of this source code is governed by the Apache 2.0 license that can be found in the license/LICENSE.txt file.
 */
// port-lint: source libraries/stdlib/native-wasm/src/kotlin/collections/Collections.kt
// port-lint: source libraries/stdlib/native-wasm/src/kotlin/collections/Collection.kt
// port-lint: source libraries/stdlib/native-wasm/src/kotlin/collections/List.kt
// Transliterated from: libraries/stdlib/native-wasm/src/kotlin/collections/Collections.kt:24-29
// Transliterated from: libraries/stdlib/native-wasm/src/kotlin/collections/Collection.kt:33-69
// Transliterated from: libraries/stdlib/native-wasm/src/kotlin/collections/List.kt:39-114
#pragma once

#include "Iterator.hpp"
#include <memory>
#include <string>

namespace kotlin::collections {
template <typename E> class Iterable;
template <typename E> class Collection;
template <typename E> class List;

namespace detail {
// NOTE(port): Iterator factories transfer their newly created C++ iterator to
// the caller. Collection and sub-list shared handles retain source object/view
// identity; their implementations determine backing-store ownership.
class IterableObject {
 public:
  virtual ~IterableObject() = default;
 protected:
  IterableObject() = default;
  IterableObject(const IterableObject&) = delete;
  IterableObject& operator=(const IterableObject&) = delete;
  // Transliterated from: libraries/stdlib/native-wasm/src/kotlin/collections/Collections.kt:28-28
  virtual std::unique_ptr<IteratorObject> iterator_dispatch() const = 0;
  template <typename> friend class kotlin::collections::Iterable;
};
class CollectionObject : public virtual IterableObject {
 public:
  // Transliterated from: libraries/stdlib/native-wasm/src/kotlin/collections/Collection.kt:44-44
  virtual std::int32_t get_size() const = 0;
  // Transliterated from: libraries/stdlib/native-wasm/src/kotlin/collections/Collection.kt:51-51
  virtual bool is_empty() const = 0;
 protected:
  // Transliterated from: libraries/stdlib/native-wasm/src/kotlin/collections/Collection.kt:58-58
  virtual bool contains_dispatch(const std::any& element) const = 0;
  // Transliterated from: libraries/stdlib/native-wasm/src/kotlin/collections/Collection.kt:68-68
  virtual bool contains_all_dispatch(const CollectionObject& elements) const = 0;
  template <typename> friend class kotlin::collections::Collection;
};
class ListObject : public virtual CollectionObject {
 public:
  // NOTE(port): Source List explicitly requires structural equals/hash/text
  // implementations. Keep those requirements abstract at this boundary.
  // Transliterated from: libraries/stdlib/native-wasm/src/kotlin/collections/List.kt:23-35
  virtual bool equals(const std::any& other) const = 0;
  // Transliterated from: libraries/stdlib/native-wasm/src/kotlin/collections/List.kt:23-35
  virtual std::int32_t hash_code() const = 0;
  // Transliterated from: libraries/stdlib/native-wasm/src/kotlin/collections/List.kt:23-35
  virtual std::u16string to_string() const = 0;
 protected:
  // Transliterated from: libraries/stdlib/native-wasm/src/kotlin/collections/List.kt:57-57
  virtual std::any get_dispatch(std::int32_t index) const = 0;
  // Transliterated from: libraries/stdlib/native-wasm/src/kotlin/collections/List.kt:68-68
  virtual std::int32_t index_of_dispatch(const std::any& element) const = 0;
  // Transliterated from: libraries/stdlib/native-wasm/src/kotlin/collections/List.kt:78-78
  virtual std::int32_t last_index_of_dispatch(const std::any& element) const = 0;
  // Transliterated from: libraries/stdlib/native-wasm/src/kotlin/collections/List.kt:90-90
  virtual std::unique_ptr<ListIteratorObject> list_iterator_dispatch() const = 0;
  // Transliterated from: libraries/stdlib/native-wasm/src/kotlin/collections/List.kt:98-98
  virtual std::unique_ptr<ListIteratorObject> list_iterator_dispatch(std::int32_t index) const = 0;
  // Transliterated from: libraries/stdlib/native-wasm/src/kotlin/collections/List.kt:113-113
  virtual std::shared_ptr<ListObject> sub_list_dispatch(std::int32_t from_index,
                                                     std::int32_t to_index) const = 0;
  template <typename> friend class kotlin::collections::List;
};
// NOTE(port): Join the source size/empty contracts across covariance diamonds
// in this private intermediate C++ base. They remain abstract source operations.
template <typename... Types>
struct CovariantBases<Collection, std::tuple<Types...>>
    : public virtual CollectionObject, public virtual Collection<Types>... {
  std::int32_t get_size() const override = 0;
  bool is_empty() const override = 0;
};
template <typename... Types>
struct CovariantBases<List, std::tuple<Types...>>
    : public virtual ListObject, public virtual List<Types>... {
  std::int32_t get_size() const override = 0;
  bool is_empty() const override = 0;
};
}  // namespace detail

/**
 * Classes that inherit from this interface can be represented as a sequence of elements that can
 * be iterated over.
 * @param T the type of element being iterated over. The iterator is covariant in its element type.
 */
// Transliterated from: libraries/stdlib/native-wasm/src/kotlin/collections/Collections.kt:24-29
template <typename E>
class Iterable : public virtual detail::IterableObject,
                 public detail::CovariantBases<Iterable, typename detail::ElementSupertypes<E>::Types> {
 public:
  /**
   * Returns an iterator over the elements of this object.
   */
  // Transliterated from: libraries/stdlib/native-wasm/src/kotlin/collections/Collections.kt:25-28
  std::unique_ptr<Iterator<E>> iterator() const {
    auto object = iterator_dispatch();
    auto& typed = dynamic_cast<Iterator<E>&>(*object);
    object.release();
    return std::unique_ptr<Iterator<E>>(&typed);
  }
};

/**
 * A generic collection of elements. The interface allows iterating over contained elements
 * and checking whether something is contained within the collection. Complex operations are built upon this
 * functionality and provided in form of [kotlin.collections] extension functions.
 *
 * Functions in this interface support only read-only access to the collection;
 * read/write access is supported through the [MutableCollection] interface.
 *
 * [Collection] is a top-level interface for objects aggregating multiple different homogenous elements. Other more specific interfaces,
 * like [List], [Set], and [Map] extend [Collection] to provide more specific guarantees on how elements are stored and accessed, as well
 * as provide richer functionality.
 *
 * [Collection] implementation may have different guarantees on the order and uniqueness of contained elements,
 * for example, elements contained in a [List] are ordered and could contain duplicates, while elements contained in
 * a [Set] may not contain duplicates and there is no particular order imposed on them.
 *
 * [Collection.contains] behavior is implementation-specific, but usually, it uses [Any.equals] to compare elements
 * for equality.
 *
 * [Collection] does not impose any requirements for [toString], [equals] and [hashCode] functions
 * and implementations are free to inherit a default behavior.
 * More specialized interfaces extending [Collection] (like [List], [Set] and [Map]) may impose stricter requirements.
 *
 * @param E the type of elements contained in the collection. The collection is covariant in its element type.
 */
// Transliterated from: libraries/stdlib/native-wasm/src/kotlin/collections/Collection.kt:33-69
template <typename E>
class Collection : public virtual Iterable<E>, public virtual detail::CollectionObject,
                   public detail::CovariantBases<Collection, typename detail::ElementSupertypes<E>::Types> {
 public:
  /**
   * Returns the size of the collection.
   *
   * If a collection contains more than [Int.MAX_VALUE] elements, the value of this property is unspecified.
   * For implementations allowing to have more than [Int.MAX_VALUE] elements,
   * it is recommended to explicitly document behavior of this property.
   *
   * @sample samples.collections.Collections.Collections.collectionSize
   */
  // Transliterated from: libraries/stdlib/native-wasm/src/kotlin/collections/Collection.kt:35-44
  std::int32_t get_size() const override = 0;
  /**
   * Returns `true` if the collection is empty (contains no elements), `false` otherwise.
   *
   * @sample samples.collections.Collections.Collections.collectionIsEmpty
   */
  // Transliterated from: libraries/stdlib/native-wasm/src/kotlin/collections/Collection.kt:46-51
  bool is_empty() const override = 0;
  /**
   * Checks if the specified element is contained in this collection.
   *
   * @sample samples.collections.Collections.Collections.collectionContains
   */
  // Transliterated from: libraries/stdlib/native-wasm/src/kotlin/collections/Collection.kt:53-58
  bool contains(E element) const {
    return contains_dispatch(detail::ElementCodec<E>::box(std::move(element)));
  }
  // Transliterated from: libraries/stdlib/native-wasm/src/kotlin/collections/Collection.kt:60-60
  std::unique_ptr<Iterator<E>> iterator() const { return Iterable<E>::iterator(); }
  /**
   * Checks if all elements in the specified collection are contained in this collection.
   *
   * @sample samples.collections.Collections.Collections.collectionContainsAll
   */
  // Transliterated from: libraries/stdlib/native-wasm/src/kotlin/collections/Collection.kt:63-68
  bool contains_all(const Collection<E>& elements) const {
    return contains_all_dispatch(elements);
  }
};

/**
 * A generic ordered collection of elements. The interface allows iterating over contained elements,
 * accessing elements by index, checking if a list contains some elements, and searching indices for particular values.
 * Complex operations are built upon this functionality and provided in form of [kotlin.collections] extension functions.
 *
 * Functions in this interface support only read-only access to the list;
 * read/write access is supported through the [MutableList] interface.
 *
 * In addition to a regular iteration, it is possible to obtain [ListIterator] using [listIterator] that provides
 * bidirectional iteration facilities, and allows accessing elements' indices in addition to their values.
 *
 * It is possible to get a view over a continuous span of elements using [subList].
 *
 * Unlike [Set], lists can contain duplicate elements.
 *
 * Unlike [Collection] implementations, [List] implementations must override [Any.toString], [Any.equals] and [Any.hashCode] functions
 * and provide implementations such that:
 * - [List.toString] should return a string containing string representation of contained elements in exact same order
 *   these elements are stored within the list.
 * - [List.equals] should consider two lists equal if and only if they contain the same number of elements and each element
 *   in one list is equal to an element in another list at the same index. Unlike some other `equals` implementations, [List.equals]
 *   should consider two lists equal even if they are instances of different classes; the only requirement here is that both lists have
 *   to implement [List] interface.
 * - [List.hashCode] should be computed as a combination of elements' hash codes using the following algorithm:
 *   ```kotlin
 *   var hashCode: Int = 1
 *   for (element in this) hashCode = hashCode * 31 + element.hashCode()
 *   ```
 *
 * @param E the type of elements contained in the list. The list is covariant in its element type.
 */
// Transliterated from: libraries/stdlib/native-wasm/src/kotlin/collections/List.kt:39-114
template <typename E>
class List : public virtual Collection<E>, public virtual detail::ListObject,
             public detail::CovariantBases<List, typename detail::ElementSupertypes<E>::Types> {
 public:
  // Transliterated from: libraries/stdlib/native-wasm/src/kotlin/collections/List.kt:41-41
  std::int32_t get_size() const override = 0;
  // Transliterated from: libraries/stdlib/native-wasm/src/kotlin/collections/List.kt:42-42
  bool is_empty() const override = 0;
  // Transliterated from: libraries/stdlib/native-wasm/src/kotlin/collections/List.kt:43-43
  bool contains(E element) const { return Collection<E>::contains(std::move(element)); }
  // Transliterated from: libraries/stdlib/native-wasm/src/kotlin/collections/List.kt:44-44
  std::unique_ptr<Iterator<E>> iterator() const { return Collection<E>::iterator(); }
  // Transliterated from: libraries/stdlib/native-wasm/src/kotlin/collections/List.kt:47-47
  bool contains_all(const Collection<E>& elements) const { return Collection<E>::contains_all(elements); }
  /**
   * Returns the element at the specified index in the list.
   *
   * @throws IndexOutOfBoundsException if [index] is less than zero or greater than or equal to [size] of this list.
   *
   * @sample samples.collections.Collections.Lists.get
   */
  // Transliterated from: libraries/stdlib/native-wasm/src/kotlin/collections/List.kt:50-57
  E get(std::int32_t index) const { return detail::ElementCodec<E>::unbox(this->get_dispatch(index)); }
  /**
   * Returns the index of the first occurrence of the specified element in the list, or `-1` if the specified
   * element is not contained in the list.
   *
   * For lists containing more than [Int.MAX_VALUE] elements, a result of this function is unspecified.
   *
   * @sample samples.collections.Collections.Lists.indexOf
   */
  // Transliterated from: libraries/stdlib/native-wasm/src/kotlin/collections/List.kt:60-68
  std::int32_t index_of(E element) const { return this->index_of_dispatch(detail::ElementCodec<E>::box(std::move(element))); }
  /**
   * Returns the index of the last occurrence of the specified element in the list, or -1 if the specified
   * element is not contained in the list.
   *
   * For lists containing more than [Int.MAX_VALUE] elements, a result of this function is unspecified.
   *
   * @sample samples.collections.Collections.Lists.lastIndexOf
   */
  // Transliterated from: libraries/stdlib/native-wasm/src/kotlin/collections/List.kt:70-78
  std::int32_t last_index_of(E element) const { return this->last_index_of_dispatch(detail::ElementCodec<E>::box(std::move(element))); }
  /**
   * Returns a list iterator over the elements in this list (in proper sequence).
   *
   * If the list needs to be iterated starting from a specific index,
   * a [listIterator] overload accepting the [Int] parameter could be used instead
   * of using this function and manually iterating until the required index is reached.
   *
   * @sample samples.collections.Collections.Lists.listIterator
   */
  // Transliterated from: libraries/stdlib/native-wasm/src/kotlin/collections/List.kt:81-90
  std::unique_ptr<ListIterator<E>> list_iterator() const {
    auto object = this->list_iterator_dispatch();
    auto& typed = dynamic_cast<ListIterator<E>&>(*object);
    object.release();
    return std::unique_ptr<ListIterator<E>>(&typed);
  }
  /**
   * Returns a list iterator over the elements in this list (in proper sequence), starting at the specified [index].
   *
   * @throws IndexOutOfBoundsException if [index] is less than zero or greater than [size] of this list.
   * @sample samples.collections.Collections.Lists.listIteratorWithIndex
   */
  // Transliterated from: libraries/stdlib/native-wasm/src/kotlin/collections/List.kt:92-98
  std::unique_ptr<ListIterator<E>> list_iterator(std::int32_t index) const {
    auto object = this->list_iterator_dispatch(index);
    auto& typed = dynamic_cast<ListIterator<E>&>(*object);
    object.release();
    return std::unique_ptr<ListIterator<E>>(&typed);
  }
  /**
   * Returns a view of the portion of this list between the specified [fromIndex] (inclusive) and [toIndex] (exclusive).
   * The returned list is backed by this list, so non-structural changes in the returned list are reflected in this list.
   *
   * Structural changes in the base list make the behavior of the view unspecified.
   *
   * @throws IndexOutOfBoundsException if [fromIndex] less than zero or [toIndex] greater than [size] of this list.
   * @throws IllegalArgumentException of [fromIndex] is greater than [toIndex].
   *
   * @sample samples.collections.Collections.Lists.subList
   */
  // Transliterated from: libraries/stdlib/native-wasm/src/kotlin/collections/List.kt:101-113
  std::shared_ptr<List<E>> sub_list(std::int32_t from_index, std::int32_t to_index) const {
    auto object = this->sub_list_dispatch(from_index, to_index);
    auto& typed = dynamic_cast<List<E>&>(*object);
    return {std::move(object), &typed};
  }
};

}  // namespace kotlin::collections
