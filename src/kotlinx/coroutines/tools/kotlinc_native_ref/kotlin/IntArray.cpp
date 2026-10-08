/*
 * Copyright 2010-2024 JetBrains s.r.o. and Kotlin Programming Language contributors.
 * Use of this source code is governed by the Apache 2.0 license that can be found in the license/LICENSE.txt file.
 */
// port-lint: source kotlin-native/runtime/src/main/kotlin/kotlin/Arrays.kt
// port-lint: source kotlin-native/runtime/src/main/cpp/Arrays.cpp
// port-lint: source kotlin-native/runtime/src/main/kotlin/kotlin/collections/ArrayUtil.kt
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/Arrays.kt:258-328
#include "IntArray.hpp"
#include "collections/ArrayUtil.hpp"
#include "../../../Exceptions.hpp"
#include <cstring>
#include <stdexcept>
#include <string>

namespace kotlin {
// NOTE(port): Native constructs fixed-length, zero-initialized primitive slots.
// Compiler values use explicit C++ ownership; this is not a Native object layout.
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/Arrays.kt:258-266
struct IntArray::Storage {
  // Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/Arrays.kt:258-266
  explicit Storage(std::int32_t size)
      : size_(size), slots_(std::make_unique<std::int32_t[]>(static_cast<std::size_t>(size))) {}
  std::int32_t size_;
  std::unique_ptr<std::int32_t[]> slots_;
};

namespace {
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/Arrays.kt:324-328
class IntArrayIterator final : public collections::IntIterator {
 public:
  // Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/Arrays.kt:324-325
  explicit IntArrayIterator(IntArray array) : array_(std::move(array)), index_(0) {}
  // Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/Arrays.kt:326-326
  bool has_next() const override { return index_ < array_.get_size(); }
  // Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/Arrays.kt:327-327
  std::int32_t next_int() override {
    if (index_ < array_.get_size()) return array_.get(index_++);
    throw kotlinx::coroutines::NoSuchElementException(std::to_string(index_));
  }
 private:
  IntArray array_;
  std::int32_t index_;
};
}

// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/Arrays.kt:258-266
IntArray::IntArray(std::int32_t size) {
  if (size < 0) throw std::invalid_argument("Negative array size: " + std::to_string(size));
  storage_ = std::make_shared<Storage>(size);
}
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/Arrays.kt:291-293
// Transliterated from: kotlin-native/runtime/src/main/cpp/Arrays.cpp:30-37
// Transliterated from: kotlin-native/runtime/src/main/cpp/Arrays.cpp:74-80
// Transliterated from: kotlin-native/runtime/src/main/cpp/Arrays.cpp:519-521
std::int32_t IntArray::get(std::int32_t index) const {
  if (static_cast<std::uint32_t>(index) >= static_cast<std::uint32_t>(storage_->size_)) {
    throw std::out_of_range("");
  }
  return storage_->slots_[index];
}
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/Arrays.kt:305-307
// Transliterated from: kotlin-native/runtime/src/main/cpp/Arrays.cpp:30-37
// Transliterated from: kotlin-native/runtime/src/main/cpp/Arrays.cpp:66-72
// Transliterated from: kotlin-native/runtime/src/main/cpp/Arrays.cpp:527-529
void IntArray::set(std::int32_t index, std::int32_t value) {
  if (static_cast<std::uint32_t>(index) >= static_cast<std::uint32_t>(storage_->size_)) {
    throw std::out_of_range("");
  }
  storage_->slots_[index] = value;
}
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/Arrays.kt:312-313
std::int32_t IntArray::get_size() const { return get_array_length(); }
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/Arrays.kt:319-321
// Transliterated from: kotlin-native/runtime/src/main/cpp/Arrays.cpp:535-538
std::int32_t IntArray::get_array_length() const { return storage_->size_; }
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/Arrays.kt:316-317
std::unique_ptr<collections::IntIterator> IntArray::iterator() const {
  return std::make_unique<IntArrayIterator>(*this);
}

namespace collections {
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/collections/ArrayUtil.kt:54-56
// Transliterated from: kotlin-native/runtime/src/main/cpp/Arrays.cpp:39-47
// Transliterated from: kotlin-native/runtime/src/main/cpp/Arrays.cpp:552-554
void array_fill(IntArray& array, std::int32_t from_index, std::int32_t to_index, std::int32_t value) {
  check_range_indexes(from_index, to_index, array.get_size());
  auto* address = array.storage_->slots_.get() + from_index;
  for (std::int32_t index = from_index; index < to_index; ++index) {
    *address++ = value;
  }
}
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/collections/ArrayUtil.kt:111-113
// Transliterated from: kotlin-native/runtime/src/main/cpp/Arrays.cpp:49-63
// Transliterated from: kotlin-native/runtime/src/main/cpp/Arrays.cpp:587-590
void array_copy(const IntArray& array, std::int32_t from_index, IntArray& destination,
                std::int32_t to_index, std::int32_t count) {
  if (count < 0 || from_index < 0 ||
      static_cast<std::uint32_t>(count) + static_cast<std::uint32_t>(from_index) >
          static_cast<std::uint32_t>(array.get_size()) ||
      to_index < 0 ||
      static_cast<std::uint32_t>(count) + static_cast<std::uint32_t>(to_index) >
          static_cast<std::uint32_t>(destination.get_size())) {
    throw std::out_of_range("");
  }
  std::memmove(destination.storage_->slots_.get() + to_index,
               array.storage_->slots_.get() + from_index,
               static_cast<std::size_t>(count) * sizeof(std::int32_t));
}
}  // namespace collections
}  // namespace kotlin
