/*
 * Copyright 2010-2024 JetBrains s.r.o. and Kotlin Programming Language contributors.
 * Use of this source code is governed by the Apache 2.0 license that can be found in the license/LICENSE.txt file.
 */
// port-lint: source libraries/stdlib/native-wasm/src/kotlin/collections/List.kt
// Transliterated from: libraries/stdlib/native-wasm/src/kotlin/collections/List.kt:133-243
#pragma once
#include "Collections.hpp"
#include "MutableCollection.hpp"
#include "MutableIterator.hpp"
namespace kotlin::collections {
template <typename E> class MutableList;
namespace detail {
// NOTE(port): Reuse the existing erased virtual collection boundary. These
// source operations are abstract; actual lists supply their implementation.
// Transliterated from: libraries/stdlib/native-wasm/src/kotlin/collections/List.kt:133-243
class MutableListObject : public virtual ListObject,
                          public virtual MutableCollectionObject {
 protected:
// Transliterated from: libraries/stdlib/native-wasm/src/kotlin/collections/List.kt:177-177
  virtual bool add_all_at_dispatch(std::int32_t index, const CollectionObject& elements) = 0;
// Transliterated from: libraries/stdlib/native-wasm/src/kotlin/collections/List.kt:196-196
  virtual std::any set_at_dispatch(std::int32_t index, const std::any& element) = 0;
// Transliterated from: libraries/stdlib/native-wasm/src/kotlin/collections/List.kt:209-209
  virtual void add_at_dispatch(std::int32_t index, const std::any& element) = 0;
// Transliterated from: libraries/stdlib/native-wasm/src/kotlin/collections/List.kt:223-223
  virtual std::any remove_at_dispatch(std::int32_t index) = 0;
  template <typename> friend class kotlin::collections::MutableList;
};
}  // namespace detail
/**
 * A generic ordered collection of elements that supports adding, replacing and removing elements, as well as
 * iterating over contained elements, accessing them by an index and checking if a collection contains a particular value.
 *
 * If a particular use case does not require list's modification,
 * a read-only counterpart, [List] could be used instead.
 *
 * [MutableList] extends [List] contract with functions allowing to add, replace and remove elements.
 *
 * Unlike [List], iterators returned by [iterator] and [listIterator] allow modifying the list during iteration.
 * A view returned by [subList] also allows modifications of the underlying list.
 *
 * Until stated otherwise, [MutableList] implementations are not thread-safe and their modification without
 * explicit synchronization may result in data corruption, loss, and runtime errors.
 *
 * @param E the type of elements contained in the list. The mutable list is invariant in its element type.
 */
// Transliterated from: libraries/stdlib/native-wasm/src/kotlin/collections/List.kt:133-243
template <typename E>
class MutableList : public virtual List<E>, public virtual MutableCollection<E>,
                    public virtual detail::MutableListObject {
 public:
  // NOTE(port): Join inherited read-only and mutable names without changing
  // their source contracts. MutableList stays invariant in E.
// Transliterated from: libraries/stdlib/native-wasm/src/kotlin/collections/List.kt:41-41
  std::int32_t get_size() const override = 0;
// Transliterated from: libraries/stdlib/native-wasm/src/kotlin/collections/List.kt:42-42
  bool is_empty() const override = 0;
// Transliterated from: libraries/stdlib/native-wasm/src/kotlin/collections/Collection.kt:94-94
  std::unique_ptr<MutableIterator<E>> iterator() const { return MutableCollection<E>::iterator(); }
    /**
     * Adds the specified element to the end of this list.
     *
     * @return `true` because the list is always modified as the result of this operation.
     *
     * @sample samples.collections.Collections.Lists.add
     */
// Transliterated from: libraries/stdlib/native-wasm/src/kotlin/collections/List.kt:143-143
  bool add(E element) { return MutableCollection<E>::add(std::move(element)); }
// Transliterated from: libraries/stdlib/native-wasm/src/kotlin/collections/List.kt:146-146
  bool remove(E element) { return MutableCollection<E>::remove(std::move(element)); }
    /**
     * Adds all of the elements of the specified collection to the end of this list.
     *
     * The elements are appended in the order they appear in the [elements] collection.
     *
     * @return `true` if the list was changed as the result of the operation.
     *
     * @sample samples.collections.Collections.Lists.addAll
     */
// Transliterated from: libraries/stdlib/native-wasm/src/kotlin/collections/List.kt:159-159
  bool add_all(const Collection<E>& elements) { return MutableCollection<E>::add_all(elements); }
    /**
     * Inserts all of the elements of the specified collection [elements] into this list at the specified [index].
     *
     * The elements are inserted in the order they appear in the [elements] collection.
     *
     * All elements that initially were stored at indices `index .. index + size - 1` are shifted `elements.size` positions to the end.
     *
     * If [index] is equal to [size], [elements] will be appended to the list.
     *
     * @return `true` if the list was changed as the result of the operation.
     *
     * @throws IndexOutOfBoundsException if [index] less than zero or greater than [size] of this list.
     *
     * @sample samples.collections.Collections.Lists.addAllAt
     */
// Transliterated from: libraries/stdlib/native-wasm/src/kotlin/collections/List.kt:177-177
  bool add_all(std::int32_t index, const Collection<E>& elements) {
    return this->add_all_at_dispatch(index, elements);
  }
// Transliterated from: libraries/stdlib/native-wasm/src/kotlin/collections/List.kt:180-180
  bool remove_all(const Collection<E>& elements) { return MutableCollection<E>::remove_all(elements); }
// Transliterated from: libraries/stdlib/native-wasm/src/kotlin/collections/List.kt:182-182
  bool retain_all(const Collection<E>& elements) { return MutableCollection<E>::retain_all(elements); }
// Transliterated from: libraries/stdlib/native-wasm/src/kotlin/collections/List.kt:183-183
  void clear() override = 0;
    /**
     * Replaces the element at the specified position in this list with the specified element.
     *
     * @return the element previously at the specified position.
     *
     * @throws IndexOutOfBoundsException if [index] is less than zero or greater than or equal to [size] of this list.
     *
     * @sample samples.collections.Collections.Lists.set
     */
// Transliterated from: libraries/stdlib/native-wasm/src/kotlin/collections/List.kt:196-196
  E set(std::int32_t index, E element) {
    return detail::ElementCodec<E>::unbox(this->set_at_dispatch(index, detail::ElementCodec<E>::box(std::move(element))));
  }
    /**
     * Inserts an element into the list at the specified [index].
     *
     * All elements that had indices `index .. index + size - 1` are shifted 1 position right.
     *
     * If [index] is equal to [size], [element] will be appended to this list.
     *
     * @throws IndexOutOfBoundsException if [index] is less than zero or greater than [size] of this list.
     *
     * @sample samples.collections.Collections.Lists.addAt
     */
// Transliterated from: libraries/stdlib/native-wasm/src/kotlin/collections/List.kt:209-209
  void add(std::int32_t index, E element) {
    this->add_at_dispatch(index, detail::ElementCodec<E>::box(std::move(element)));
  }
    /**
     * Removes an element at the specified [index] from the list.
     *
     * All elements placed after [index] are shifted 1 position left.
     *
     * @return the element that has been removed.
     *
     * @throws IndexOutOfBoundsException if [index] is less than zero or greater than or equal to [size] of this list.
     *
     * @sample samples.collections.Collections.Lists.removeAt
     */
// Transliterated from: libraries/stdlib/native-wasm/src/kotlin/collections/List.kt:223-223
  E remove_at(std::int32_t index) {
    return detail::ElementCodec<E>::unbox(this->remove_at_dispatch(index));
  }
// Transliterated from: libraries/stdlib/native-wasm/src/kotlin/collections/List.kt:226-226
  std::unique_ptr<MutableListIterator<E>> list_iterator() const {
    auto object = this->list_iterator_dispatch();
    auto& typed = dynamic_cast<MutableListIterator<E>&>(*object);
    object.release();
    return std::unique_ptr<MutableListIterator<E>>(&typed);
  }
// Transliterated from: libraries/stdlib/native-wasm/src/kotlin/collections/List.kt:228-228
  std::unique_ptr<MutableListIterator<E>> list_iterator(std::int32_t index) const {
    auto object = this->list_iterator_dispatch(index);
    auto& typed = dynamic_cast<MutableListIterator<E>&>(*object);
    object.release();
    return std::unique_ptr<MutableListIterator<E>>(&typed);
  }
    /**
     * Returns a view of the portion of this list between the specified [fromIndex] (inclusive) and [toIndex] (exclusive).
     * The returned list is backed by this list, so changes in the returned list are reflected in this list, and vice-versa.
     *
     * Structural changes in the base list make the behavior of the view unspecified.
     *
     * @throws IndexOutOfBoundsException if [fromIndex] less than zero or [toIndex] greater than [size] of this list.
     * @throws IllegalArgumentException of [fromIndex] is greater than [toIndex].
     *
     * @sample samples.collections.Collections.Lists.subList
     */
// Transliterated from: libraries/stdlib/native-wasm/src/kotlin/collections/List.kt:242-242
  std::shared_ptr<MutableList<E>> sub_list(std::int32_t from_index, std::int32_t to_index) const {
    auto object = this->sub_list_dispatch(from_index, to_index);
    auto& typed = dynamic_cast<MutableList<E>&>(*object);
    return {std::move(object), &typed};
  }
};
}  // namespace kotlin::collections
