/*
 * Copyright 2010-2024 JetBrains s.r.o. and Kotlin Programming Language contributors.
 * Use of this source code is governed by the Apache 2.0 license that can be found in the license/LICENSE.txt file.
 */
// port-lint: source libraries/stdlib/native-wasm/src/kotlin/collections/Iterator.kt
// Transliterated from: libraries/stdlib/native-wasm/src/kotlin/collections/Iterator.kt:30-112
#pragma once
#include "Iterator.hpp"
namespace kotlin::collections {
template <typename T> class MutableIterator;
template <typename T> class MutableListIterator;
namespace detail {
// NOTE(port): Typed source variance uses the existing generic virtual boundary.
// Transliterated from: libraries/stdlib/native-wasm/src/kotlin/collections/Iterator.kt:30-38
class MutableIteratorObject : public virtual IteratorObject {
 public:
/**
     * Removes from the underlying collection the last element returned by this iterator.
     *
     * @throws IllegalStateException if [next] has not been called yet,
     * or the most recent [next] call has already been followed by a [remove] call.
     */
// Transliterated from: libraries/stdlib/native-wasm/src/kotlin/collections/Iterator.kt:37-37
virtual void remove() = 0;
};
// Transliterated from: libraries/stdlib/native-wasm/src/kotlin/collections/Iterator.kt:81-112
class MutableListIteratorObject : public virtual ListIteratorObject, public virtual MutableIteratorObject {
 protected:
/**
     * Replaces the last element returned by [next] or [previous] with the specified element [element].
     *
     * @throws IllegalStateException if neither [next] nor [previous] has not been called yet,
     * or the most recent [next] or [previous] call has already been followed by a [remove] or [add] call.
     */
// Transliterated from: libraries/stdlib/native-wasm/src/kotlin/collections/Iterator.kt:101-101
virtual void set_dispatch(const std::any& element) = 0;
/**
     * Adds the specified element [element] into the underlying collection immediately before the element that would be
     * returned by [next], if any, and after the element that would be returned by [previous], if any.
     * (If the collection contains no elements, the new element becomes the sole element in the collection.)
     * The new element is inserted before the implicit cursor: a subsequent call to [next] would be unaffected,
     * and a subsequent call to [previous] would return the new element. (This call increases by one the value \
     * that would be returned by a call to [nextIndex] or [previousIndex].)
     */
// Transliterated from: libraries/stdlib/native-wasm/src/kotlin/collections/Iterator.kt:111-111
virtual void add_dispatch(const std::any& element) = 0;
template <typename> friend class kotlin::collections::MutableListIterator;
};
// Transliterated from: libraries/stdlib/native-wasm/src/kotlin/collections/Iterator.kt:30-38
template <typename... Types>
struct CovariantBases<MutableIterator, std::tuple<Types...>>
    : public virtual MutableIteratorObject, public virtual MutableIterator<Types>... {
  // Transliterated from: libraries/stdlib/native-wasm/src/kotlin/collections/Iterator.kt:23-23
  bool has_next() const override = 0;
  // Transliterated from: libraries/stdlib/native-wasm/src/kotlin/collections/Iterator.kt:37-37
  void remove() override = 0;
};
}  // namespace detail
/**
 * An iterator over a mutable collection. Provides the ability to remove elements while iterating.
 * @see MutableCollection.iterator
 */
// Transliterated from: libraries/stdlib/native-wasm/src/kotlin/collections/Iterator.kt:30-38
template <typename T>
class MutableIterator : public virtual Iterator<T>, public virtual detail::MutableIteratorObject,
    public detail::CovariantBases<MutableIterator, typename detail::ElementSupertypes<T>::Types> {
 public:
/**
     * Removes from the underlying collection the last element returned by this iterator.
     *
     * @throws IllegalStateException if [next] has not been called yet,
     * or the most recent [next] call has already been followed by a [remove] call.
     */
// Transliterated from: libraries/stdlib/native-wasm/src/kotlin/collections/Iterator.kt:37-37
void remove() override = 0;
// Transliterated from: libraries/stdlib/native-wasm/src/kotlin/collections/Iterator.kt:23-23
bool has_next() const override = 0;
};
/**
 * An iterator over a mutable collection that supports indexed access. Provides the ability
 * to add, modify and remove elements while iterating.
 * @see MutableList.listIterator
 */
// Transliterated from: libraries/stdlib/native-wasm/src/kotlin/collections/Iterator.kt:81-112
template <typename T>
class MutableListIterator : public virtual ListIterator<T>, public virtual MutableIterator<T>,
    public virtual detail::MutableListIteratorObject {
 public:
// Transliterated from: libraries/stdlib/native-wasm/src/kotlin/collections/Iterator.kt:83-83
T next() { return Iterator<T>::next(); }
// Transliterated from: libraries/stdlib/native-wasm/src/kotlin/collections/Iterator.kt:84-84
bool has_next() const override = 0;
/**
     * Removes from the underlying collection the last element returned by this iterator.
     *
     * @throws IllegalStateException if neither [next] nor [previous] has not been called yet,
     * or the most recent [next] or [previous] call has already been followed by a [remove] or [add] call.
     */
// Transliterated from: libraries/stdlib/native-wasm/src/kotlin/collections/Iterator.kt:93-93
void remove() override = 0;
/**
     * Replaces the last element returned by [next] or [previous] with the specified element [element].
     *
     * @throws IllegalStateException if neither [next] nor [previous] has not been called yet,
     * or the most recent [next] or [previous] call has already been followed by a [remove] or [add] call.
     */
// Transliterated from: libraries/stdlib/native-wasm/src/kotlin/collections/Iterator.kt:101-101
void set(T element) { set_dispatch(detail::ElementCodec<T>::box(std::move(element))); }
/**
     * Adds the specified element [element] into the underlying collection immediately before the element that would be
     * returned by [next], if any, and after the element that would be returned by [previous], if any.
     * (If the collection contains no elements, the new element becomes the sole element in the collection.)
     * The new element is inserted before the implicit cursor: a subsequent call to [next] would be unaffected,
     * and a subsequent call to [previous] would return the new element. (This call increases by one the value \
     * that would be returned by a call to [nextIndex] or [previousIndex].)
     */
// Transliterated from: libraries/stdlib/native-wasm/src/kotlin/collections/Iterator.kt:111-111
void add(T element) { add_dispatch(detail::ElementCodec<T>::box(std::move(element))); }
// Transliterated from: libraries/stdlib/native-wasm/src/kotlin/collections/Iterator.kt:52-52
bool has_previous() const override = 0;
// Transliterated from: libraries/stdlib/native-wasm/src/kotlin/collections/Iterator.kt:66-66
std::int32_t next_index() const override = 0;
// Transliterated from: libraries/stdlib/native-wasm/src/kotlin/collections/Iterator.kt:73-73
std::int32_t previous_index() const override = 0;
};
}  // namespace kotlin::collections
