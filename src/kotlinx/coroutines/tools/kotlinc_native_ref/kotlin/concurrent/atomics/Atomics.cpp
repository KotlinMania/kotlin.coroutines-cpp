/*
 * Copyright 2010-2025 JetBrains s.r.o. and Kotlin Programming Language contributors.
 * Use of this source code is governed by the Apache 2.0 license that can be found in the license/LICENSE.txt file.
 */
// port-lint: source kotlin-native/runtime/src/main/kotlin/kotlin/concurrent/atomics/Atomics.native.kt
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/concurrent/atomics/Atomics.native.kt:30-144
#include "Atomics.hpp"
#include <bit>

namespace kotlin::concurrent::atomics {
namespace {
// NOTE(port): Kotlin Int addition wraps modulo 2^32. Unsigned arithmetic and
// bit_cast preserve that result without C++ signed-overflow undefined behavior.
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/concurrent/atomics/Atomics.native.kt:93-93
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/concurrent/atomics/Atomics.native.kt:111-111
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/concurrent/atomics/Atomics.native.kt:123-123
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/concurrent/atomics/Atomics.native.kt:129-129
std::int32_t wrap_add(std::int32_t left, std::int32_t right) {
  return std::bit_cast<std::int32_t>(static_cast<std::uint32_t>(left) + static_cast<std::uint32_t>(right));
}
}  // namespace
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/concurrent/atomics/Atomics.native.kt:30-34
AtomicInt::AtomicInt(std::int32_t value) : value_(value) {}
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/concurrent/atomics/Atomics.native.kt:40-40
std::int32_t AtomicInt::load() const {
  return value_.load(std::memory_order_seq_cst);
}
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/concurrent/atomics/Atomics.native.kt:47-47
void AtomicInt::store(std::int32_t new_value) {
  value_.store(new_value, std::memory_order_seq_cst);
}
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/concurrent/atomics/Atomics.native.kt:54-54
std::int32_t AtomicInt::exchange(std::int32_t new_value) {
  return value_.exchange(new_value, std::memory_order_seq_cst);
}
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/concurrent/atomics/Atomics.native.kt:67-68
bool AtomicInt::compare_and_set(std::int32_t expected_value, std::int32_t new_value) {
  return value_.compare_exchange_strong(expected_value, new_value, std::memory_order_seq_cst, std::memory_order_seq_cst);
}
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/concurrent/atomics/Atomics.native.kt:78-79
std::int32_t AtomicInt::compare_and_exchange(std::int32_t expected_value, std::int32_t new_value) {
  value_.compare_exchange_strong(expected_value, new_value, std::memory_order_seq_cst, std::memory_order_seq_cst);
  return expected_value;
}
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/concurrent/atomics/Atomics.native.kt:86-86
std::int32_t AtomicInt::fetch_and_add(std::int32_t delta) {
  return value_.fetch_add(delta, std::memory_order_seq_cst);
}
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/concurrent/atomics/Atomics.native.kt:93-93
std::int32_t AtomicInt::add_and_fetch(std::int32_t delta) {
  return wrap_add(value_.fetch_add(delta, std::memory_order_seq_cst), delta);
}
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/concurrent/atomics/Atomics.native.kt:99-99
std::int32_t AtomicInt::get_and_set(std::int32_t new_value) {
  return value_.exchange(new_value, std::memory_order_seq_cst);
}
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/concurrent/atomics/Atomics.native.kt:105-105
std::int32_t AtomicInt::get_and_add(std::int32_t delta) {
  return value_.fetch_add(delta, std::memory_order_seq_cst);
}
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/concurrent/atomics/Atomics.native.kt:111-111
std::int32_t AtomicInt::add_and_get(std::int32_t delta) {
  return wrap_add(value_.fetch_add(delta, std::memory_order_seq_cst), delta);
}
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/concurrent/atomics/Atomics.native.kt:117-117
std::int32_t AtomicInt::get_and_increment() {
  return value_.fetch_add(1, std::memory_order_seq_cst);
}
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/concurrent/atomics/Atomics.native.kt:123-123
std::int32_t AtomicInt::increment_and_get() {
  return wrap_add(value_.fetch_add(1, std::memory_order_seq_cst), 1);
}
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/concurrent/atomics/Atomics.native.kt:129-129
std::int32_t AtomicInt::decrement_and_get() {
  return wrap_add(value_.fetch_add(-1, std::memory_order_seq_cst), -1);
}
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/concurrent/atomics/Atomics.native.kt:135-135
std::int32_t AtomicInt::get_and_decrement() {
  return value_.fetch_add(-1, std::memory_order_seq_cst);
}
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/concurrent/atomics/Atomics.native.kt:143-143
std::string AtomicInt::to_string() const {
  return std::to_string(value_.load(std::memory_order_seq_cst));
}
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/concurrent/atomics/Atomics.native.kt:30-34
std::int32_t AtomicInt::get_value() const { return value_.load(std::memory_order_seq_cst); }
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/concurrent/atomics/Atomics.native.kt:30-34
void AtomicInt::set_value(std::int32_t new_value) { value_.store(new_value, std::memory_order_seq_cst); }
// Transliterated from: libraries/stdlib/src/kotlin/concurrent/atomics/Atomics.common.kt:107-107
void plus_assign(AtomicInt& receiver, std::int32_t delta) {
  static_cast<void>(receiver.add_and_fetch(delta));
}
// Transliterated from: libraries/stdlib/src/kotlin/concurrent/atomics/Atomics.common.kt:116-116
void minus_assign(AtomicInt& receiver, std::int32_t delta) {
  static_cast<void>(receiver.add_and_fetch(std::bit_cast<std::int32_t>(std::uint32_t{0} - static_cast<std::uint32_t>(delta))));
}
// Transliterated from: libraries/stdlib/src/kotlin/concurrent/atomics/Atomics.common.kt:125-125
std::int32_t fetch_and_increment(AtomicInt& receiver) {
  return receiver.fetch_and_add(1);
}
// Transliterated from: libraries/stdlib/src/kotlin/concurrent/atomics/Atomics.common.kt:134-134
std::int32_t increment_and_fetch(AtomicInt& receiver) {
  return receiver.add_and_fetch(1);
}
// Transliterated from: libraries/stdlib/src/kotlin/concurrent/atomics/Atomics.common.kt:143-143
std::int32_t decrement_and_fetch(AtomicInt& receiver) {
  return receiver.add_and_fetch(-1);
}
// Transliterated from: libraries/stdlib/src/kotlin/concurrent/atomics/Atomics.common.kt:152-152
std::int32_t fetch_and_decrement(AtomicInt& receiver) {
  return receiver.fetch_and_add(-1);
}
}  // namespace kotlin::concurrent::atomics
