/*
 * Copyright 2010-2024 JetBrains s.r.o. and Kotlin Programming Language contributors.
 * Use of this source code is governed by the Apache 2.0 license that can be found in the license/LICENSE.txt file.
 */
// port-lint: source libraries/stdlib/native-wasm/src/kotlin/collections/Collection.kt
// Transliterated from: libraries/stdlib/native-wasm/src/kotlin/collections/Collection.kt:92-160
#pragma once
// port-lint: source libraries/stdlib/native-wasm/src/kotlin/collections/Collections.kt
#include "Collections.hpp"
#include "MutableIterator.hpp"
namespace kotlin::collections {
template <typename E> class MutableIterable;
template <typename E> class MutableCollection;
namespace detail {
// NOTE(port): These are abstract source operations at the same C++ erased
// boundary as read-only Collection. No backing collection is supplied here.
// Transliterated from: libraries/stdlib/native-wasm/src/kotlin/collections/Collection.kt:92-160
class MutableCollectionObject : public virtual CollectionObject {
 public:
/**
     * Removes all elements from this collection.
     *
     * @sample samples.collections.Collections.Collections.clear
     */
// Transliterated from: libraries/stdlib/native-wasm/src/kotlin/collections/Collection.kt:159-159
virtual void clear() = 0;
protected:
/**
     * Adds the specified element to the collection.
     *
     * @return `true` if the element has been added, `false` if the collection does not support duplicates
     * and the element is already contained in the collection.
     *
     * @sample samples.collections.Collections.Lists.add
     * @sample samples.collections.Collections.Sets.add
     */
// Transliterated from: libraries/stdlib/native-wasm/src/kotlin/collections/Collection.kt:107-107
virtual bool add_dispatch(const std::any& element) = 0;
/**
     * Removes a single instance of the specified element from this
     * collection, if the collection contains it.
     *
     * @return `true` if the element has been successfully removed; `false` if it was not contained in the collection.
     *
     * @sample samples.collections.Collections.Lists.remove
     * @sample samples.collections.Collections.Sets.remove
     */
// Transliterated from: libraries/stdlib/native-wasm/src/kotlin/collections/Collection.kt:119-119
virtual bool remove_dispatch(const std::any& element) = 0;
/**
     * Adds all of the elements of the specified collection to this collection.
     *
     * @return `true` if any of the specified elements was added to the collection, `false` if the collection was not modified.
     *
     * @sample samples.collections.Collections.Lists.addAll
     * @sample samples.collections.Collections.Sets.addAll
     */
// Transliterated from: libraries/stdlib/native-wasm/src/kotlin/collections/Collection.kt:131-131
virtual bool add_all_dispatch(const CollectionObject& elements) = 0;
/**
     * Removes all of this collection's elements that are also contained in the specified collection.
     *
     * @return `true` if any of the specified elements was removed from the collection, `false` if the collection was not modified.
     *
     * @sample samples.collections.Collections.Lists.removeAll
     * @sample samples.collections.Collections.Sets.removeAll
     */
// Transliterated from: libraries/stdlib/native-wasm/src/kotlin/collections/Collection.kt:142-142
virtual bool remove_all_dispatch(const CollectionObject& elements) = 0;
/**
     * Retains only the elements in this collection that are contained in the specified collection.
     *
     * @return `true` if any element was removed from the collection, `false` if the collection was not modified.
     *
     * @sample samples.collections.Collections.Collections.retainAll
     */
// Transliterated from: libraries/stdlib/native-wasm/src/kotlin/collections/Collection.kt:152-152
virtual bool retain_all_dispatch(const CollectionObject& elements) = 0;
template <typename> friend class kotlin::collections::MutableCollection;
};
// Transliterated from: libraries/stdlib/native-wasm/src/kotlin/collections/Collections.kt:36-41
template <typename... Types>
struct CovariantBases<MutableIterable, std::tuple<Types...>> : public virtual MutableIterable<Types>... {};
}  // namespace detail
/**
 * Classes that inherit from this interface can be represented as a sequence of elements that can
 * be iterated over and that supports removing elements during iteration.
 * @param T the type of element being iterated over. The mutable iterator is invariant in its element type.
 */
// Transliterated from: libraries/stdlib/native-wasm/src/kotlin/collections/Collections.kt:36-41
template <typename E>
class MutableIterable : public virtual Iterable<E>,
    public detail::CovariantBases<MutableIterable, typename detail::ElementSupertypes<E>::Types> {
 public:
/**
     * Returns an iterator over the elements of this sequence that supports removing elements during iteration.
     */
// Transliterated from: libraries/stdlib/native-wasm/src/kotlin/collections/Collections.kt:40-40
std::unique_ptr<MutableIterator<E>> iterator() const {
    auto object = this->iterator_dispatch();
    auto& typed = dynamic_cast<MutableIterator<E>&>(*object);
    object.release();
    return std::unique_ptr<MutableIterator<E>>(&typed);
  }
};
/**
 * A generic collection of elements that supports iterating, adding and removing elements, as well as checking if the
 * collection contains some elements. Complex operations are built upon this
 * functionality and provided in form of [kotlin.collections] extension functions.
 *
 * If a particular use case does not require collection's modification,
 * a read-only counterpart, [Collection] could be used instead.
 *
 * [MutableCollection] extends [Collection] contract with functions allowing to add or remove elements.
 *
 * [MutableCollection] is a top-level interface for mutable objects aggregating multiple different homogenous elements.
 * Other more specific interfaces, like [MutableList], [MutableSet], and [MutableMap] extend [MutableCollection] to provide
 * more specific guarantees on how elements are stored, accessed and modified, as well as provide richer functionality.
 *
 * Unlike [Collection], an iterator returned by [iterator] allows removing elements during iteration.
 *
 * Until stated otherwise, [MutableCollection] implementations are not thread-safe and their modification without
 * explicit synchronization may result in data corruption, loss, and runtime errors.
 *
 * @param E the type of elements contained in the collection. The mutable collection is invariant in its element type.
 */
// Transliterated from: libraries/stdlib/native-wasm/src/kotlin/collections/Collection.kt:92-160
template <typename E>
class MutableCollection : public virtual Collection<E>, public virtual MutableIterable<E>,
    public virtual detail::MutableCollectionObject {
 public:
// Transliterated from: libraries/stdlib/native-wasm/src/kotlin/collections/Collection.kt:94-94
std::unique_ptr<MutableIterator<E>> iterator() const { return MutableIterable<E>::iterator(); }
/**
     * Adds the specified element to the collection.
     *
     * @return `true` if the element has been added, `false` if the collection does not support duplicates
     * and the element is already contained in the collection.
     *
     * @sample samples.collections.Collections.Lists.add
     * @sample samples.collections.Collections.Sets.add
     */
// Transliterated from: libraries/stdlib/native-wasm/src/kotlin/collections/Collection.kt:107-107
bool add(E element) { return add_dispatch(detail::ElementCodec<E>::box(std::move(element))); }
/**
     * Removes a single instance of the specified element from this
     * collection, if the collection contains it.
     *
     * @return `true` if the element has been successfully removed; `false` if it was not contained in the collection.
     *
     * @sample samples.collections.Collections.Lists.remove
     * @sample samples.collections.Collections.Sets.remove
     */
// Transliterated from: libraries/stdlib/native-wasm/src/kotlin/collections/Collection.kt:119-119
bool remove(E element) { return remove_dispatch(detail::ElementCodec<E>::box(std::move(element))); }
/**
     * Adds all of the elements of the specified collection to this collection.
     *
     * @return `true` if any of the specified elements was added to the collection, `false` if the collection was not modified.
     *
     * @sample samples.collections.Collections.Lists.addAll
     * @sample samples.collections.Collections.Sets.addAll
     */
// Transliterated from: libraries/stdlib/native-wasm/src/kotlin/collections/Collection.kt:131-131
bool add_all(const Collection<E>& elements) { return add_all_dispatch(elements); }
/**
     * Removes all of this collection's elements that are also contained in the specified collection.
     *
     * @return `true` if any of the specified elements was removed from the collection, `false` if the collection was not modified.
     *
     * @sample samples.collections.Collections.Lists.removeAll
     * @sample samples.collections.Collections.Sets.removeAll
     */
// Transliterated from: libraries/stdlib/native-wasm/src/kotlin/collections/Collection.kt:142-142
bool remove_all(const Collection<E>& elements) { return remove_all_dispatch(elements); }
/**
     * Retains only the elements in this collection that are contained in the specified collection.
     *
     * @return `true` if any element was removed from the collection, `false` if the collection was not modified.
     *
     * @sample samples.collections.Collections.Collections.retainAll
     */
// Transliterated from: libraries/stdlib/native-wasm/src/kotlin/collections/Collection.kt:152-152
bool retain_all(const Collection<E>& elements) { return retain_all_dispatch(elements); }
/**
     * Removes all elements from this collection.
     *
     * @sample samples.collections.Collections.Collections.clear
     */
// Transliterated from: libraries/stdlib/native-wasm/src/kotlin/collections/Collection.kt:159-159
void clear() override = 0;
};
}  // namespace kotlin::collections
