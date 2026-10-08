/*
 * Copyright 2010-2024 JetBrains s.r.o. and Kotlin Programming Language contributors.
 * Use of this source code is governed by the Apache 2.0 license that can be found in the license/LICENSE.txt file.
 */
// port-lint: source kotlin-native/runtime/src/main/kotlin/kotlin/Array.kt
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/Array.kt:15-92
#pragma once

#include "collections/Iterator.hpp"
#include "../../../Exceptions.hpp"
#include <cstdint>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace kotlin {
template <typename T> class Array;
namespace collections::detail { class ArrayStorageAccess; }

/**
 * A generic array of objects.
 * Array instances can be created using the [arrayOf], [arrayOfNulls] and [emptyArray]
 * standard library functions.
 *
 * See [Kotlin language documentation](https://kotlinlang.org/docs/arrays.html)
 * for more information on arrays.
 */
// NOTE(port): This compiler-owned C++ storage retains source array identity and
// fixes its length before initialization. It does not implement Kotlin/Native
// ObjHeader/ArrayHeader layout, GC roots, or GCUnsafeCall entry points. Those
// runtime dependencies remain required for direct Kotlin object handoff.
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/Array.kt:24-86
template <typename T>
class Array final {
 public:
  /**
   * Creates a new array of the specified [size], where each element is calculated by calling the specified
   * [init] function.
   *
   * The function [init] is called for each array element sequentially starting from the first one.
   * It should return the value for an array element given its index.
   *
   * @throws RuntimeException if the specified [size] is negative.
   */
  // Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/Array.kt:25-39
  template <typename Init>
  Array(std::int32_t size, Init init) : storage_(allocate_storage(size)) {
    for (std::int32_t index = 0; index < size; ++index) {
      set(index, init(index));
    }
  }

  /**
   * Returns the array element at the given [index].
   *
   * This method can be called using the index operator:
   * ```
   * value = array[index]
   * ```
   *
   * If the [index] is out of bounds of this array, throws an [IndexOutOfBoundsException].
   */
  // NOTE(port): Native get is external Kotlin_Array_get, whose implementation
  // reads an object-reference slot and updates the caller's return root.
  // Compiler-owned C++ values use explicit storage, without claiming that ABI.
  // Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/Array.kt:46-57
  T get(std::int32_t index) const {
    return storage_->at(static_cast<std::size_t>(index)).value();
  }

  /**
   * Sets the array element at the given [index] to the given [value].
   *
   * This method can be called using the index operator:
   * ```
   * array[index] = value
   * ```
   *
   * If the [index] is out of bounds of this array, throws an [IndexOutOfBoundsException].
   */
  // NOTE(port): Native set is external Kotlin_Array_set and uses UpdateHeapRef.
  // This compiler-owned value assignment does not replace that runtime operation.
  // Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/Array.kt:59-70
  void set(std::int32_t index, T value) {
    storage_->at(static_cast<std::size_t>(index)).emplace(std::move(value));
  }

  /**
   * Returns the number of elements in the array.
   */
  // Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/Array.kt:72-76
  std::int32_t get_size() const {
    return get_array_length();
  }

  /** Creates an [Iterator] for iterating over the elements of the array. */
  // Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/Array.kt:78-80
  std::unique_ptr<collections::Iterator<T>> iterator() const;

 private:
  // NOTE(port): Empty slots represent compiler storage during initialization
  // and after source reset operations. Reading one throws bad_optional_access,
  // as permitted by ArrayUtil's implementation-dependent uninitialized reads.
  using Storage = std::vector<std::optional<T>>;
  struct Uninitialized {};
  // Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/Array.kt:41-43
  Array(std::int32_t size, Uninitialized) : storage_(allocate_storage(size)) {}
  friend class collections::detail::ArrayStorageAccess;
  static std::shared_ptr<Storage> allocate_storage(std::int32_t size) {
    if (size < 0) throw std::invalid_argument("Negative array size: " + std::to_string(size));
    return std::make_shared<Storage>(static_cast<std::size_t>(size));
  }
  std::shared_ptr<Storage> storage_;
  // NOTE(port): Native length reads ArrayHeader::count_ via its external entry.
  // Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/Array.kt:82-85
  std::int32_t get_array_length() const {
    return static_cast<std::int32_t>(storage_->size());
  }
};

namespace detail {
// NOTE(port): The source class is private. Its generic implementation is visible
// here because the public Array<T>::iterator must instantiate it for each T.
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/Array.kt:88-92
template <typename T>
class ArrayIterator final : public collections::Iterator<T> {
 public:
  // Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/Array.kt:88-89
  explicit ArrayIterator(Array<T> array) : array_(std::move(array)), index_(0) {}
  // Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/Array.kt:90-90
  bool has_next() const override { return index_ < array_.get_size(); }
  // Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/Array.kt:91-91
  T next() {
    if (index_ < array_.get_size()) return array_.get(index_++);
    throw kotlinx::coroutines::NoSuchElementException(std::to_string(index_));
  }
 private:
  // NOTE(port): Typed and covariant views use this same source iterator/cursor.
  // Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/Array.kt:91-91
  std::any next_dispatch() override {
    return collections::detail::ElementCodec<T>::box(next());
  }
  Array<T> array_;
  std::int32_t index_;
};
}  // namespace detail

// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/Array.kt:78-80
template <typename T>
std::unique_ptr<collections::Iterator<T>> Array<T>::iterator() const {
  return std::make_unique<detail::ArrayIterator<T>>(*this);
}
}  // namespace kotlin
