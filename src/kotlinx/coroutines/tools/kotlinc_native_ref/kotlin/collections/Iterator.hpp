/*
 * Copyright 2010-2024 JetBrains s.r.o. and Kotlin Programming Language contributors.
 * Use of this source code is governed by the Apache 2.0 license that can be found in the license/LICENSE.txt file.
 */
// port-lint: source libraries/stdlib/native-wasm/src/kotlin/collections/Iterator.kt
// Transliterated from: libraries/stdlib/native-wasm/src/kotlin/collections/Iterator.kt:12-74
#pragma once

#include "CollectionElement.hpp"
#include <cstdint>

namespace kotlin::collections {
template <typename T> class Iterator;
template <typename T> class ListIterator;

namespace detail {
// NOTE(port): C++ cannot override value-return functions covariantly. This
// private virtual boundary carries the source implementation once; typed next
// methods decode its result without copying reference-object elements.
class IteratorObject {
 public:
  virtual ~IteratorObject() = default;
  // Transliterated from: libraries/stdlib/native-wasm/src/kotlin/collections/Iterator.kt:23-23
  virtual bool has_next() const = 0;
 protected:
  IteratorObject() = default;
  IteratorObject(const IteratorObject&) = delete;
  IteratorObject& operator=(const IteratorObject&) = delete;
  // Transliterated from: libraries/stdlib/native-wasm/src/kotlin/collections/Iterator.kt:18-18
  virtual std::any next_dispatch() = 0;
  template <typename> friend class kotlin::collections::Iterator;
};
class ListIteratorObject : public virtual IteratorObject {
 public:
  // Transliterated from: libraries/stdlib/native-wasm/src/kotlin/collections/Iterator.kt:52-52
  virtual bool has_previous() const = 0;
  // Transliterated from: libraries/stdlib/native-wasm/src/kotlin/collections/Iterator.kt:66-66
  virtual std::int32_t next_index() const = 0;
  // Transliterated from: libraries/stdlib/native-wasm/src/kotlin/collections/Iterator.kt:73-73
  virtual std::int32_t previous_index() const = 0;
 protected:
  // Transliterated from: libraries/stdlib/native-wasm/src/kotlin/collections/Iterator.kt:59-59
  virtual std::any previous_dispatch() = 0;
  template <typename> friend class kotlin::collections::ListIterator;
};
// NOTE(port): Join identical virtual contracts at the intermediate covariance
// base as well as the typed interface. C++ checks final overriders when that
// intermediate class instantiates, before its enclosing interface is complete.
template <typename... Types>
struct CovariantBases<Iterator, std::tuple<Types...>>
    : public virtual IteratorObject, public virtual Iterator<Types>... {
  bool has_next() const override = 0;
};
template <typename... Types>
struct CovariantBases<ListIterator, std::tuple<Types...>>
    : public virtual ListIteratorObject, public virtual ListIterator<Types>... {
  bool has_next() const override = 0;
  bool has_previous() const override = 0;
  std::int32_t next_index() const override = 0;
  std::int32_t previous_index() const override = 0;
};
}  // namespace detail

/**
 * An iterator over a collection or another entity that can be represented as a sequence of elements.
 * Allows to sequentially access the elements.
 */
// Transliterated from: libraries/stdlib/native-wasm/src/kotlin/collections/Iterator.kt:12-24
template <typename T>
class Iterator : public virtual detail::IteratorObject,
                 public detail::CovariantBases<Iterator, typename detail::ElementSupertypes<T>::Types> {
 public:
  /**
   * Returns the next element in the iteration.
   *
   * @throws NoSuchElementException if the iteration has no next element.
   */
  // Transliterated from: libraries/stdlib/native-wasm/src/kotlin/collections/Iterator.kt:13-18
  T next() { return detail::ElementCodec<T>::unbox(next_dispatch()); }
  /**
   * Returns `true` if the iteration has more elements.
   */
  // Transliterated from: libraries/stdlib/native-wasm/src/kotlin/collections/Iterator.kt:20-23
  bool has_next() const override = 0;
};

/**
 * An iterator over a collection that supports indexed access.
 * @see List.listIterator
 */
// Transliterated from: libraries/stdlib/native-wasm/src/kotlin/collections/Iterator.kt:44-74
template <typename T>
class ListIterator : public virtual Iterator<T>, public virtual detail::ListIteratorObject,
                     public detail::CovariantBases<ListIterator, typename detail::ElementSupertypes<T>::Types> {
 public:
  // Transliterated from: libraries/stdlib/native-wasm/src/kotlin/collections/Iterator.kt:46-46
  T next() { return Iterator<T>::next(); }
  // Transliterated from: libraries/stdlib/native-wasm/src/kotlin/collections/Iterator.kt:47-47
  bool has_next() const override = 0;
  /**
   * Returns `true` if there are elements in the iteration before the current element.
   */
  // Transliterated from: libraries/stdlib/native-wasm/src/kotlin/collections/Iterator.kt:49-52
  bool has_previous() const override = 0;
  /**
   * Returns the previous element in the iteration and moves the cursor position backwards.
   *
   * @throws NoSuchElementException if the iteration has no previous element.
   */
  // Transliterated from: libraries/stdlib/native-wasm/src/kotlin/collections/Iterator.kt:54-59
  T previous() { return detail::ElementCodec<T>::unbox(this->previous_dispatch()); }
  /**
   * Returns the index of the element that would be returned by a subsequent call to [next].
   *
   * Returns collection size if the iteration is at the end of the collection.
   */
  // Transliterated from: libraries/stdlib/native-wasm/src/kotlin/collections/Iterator.kt:61-66
  std::int32_t next_index() const override = 0;
  /**
   * Returns the index of the element that would be returned by a subsequent call to [previous].
   *
   * Returns -1 if the iteration is at the beginning of the collection.
   */
  // Transliterated from: libraries/stdlib/native-wasm/src/kotlin/collections/Iterator.kt:68-73
  std::int32_t previous_index() const override = 0;
};

}  // namespace kotlin::collections
