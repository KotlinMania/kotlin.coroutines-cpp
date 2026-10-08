/*
 * Copyright 2010-2025 JetBrains s.r.o. and Kotlin Programming Language contributors.
 * Use of this source code is governed by the Apache 2.0 license that can be found in the license/LICENSE.txt file.
 */
// port-lint: source kotlin-native/runtime/src/main/kotlin/kotlin/concurrent/atomics/Atomics.native.kt
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/concurrent/atomics/Atomics.native.kt:15-144
#pragma once

#include <atomic>
#include <cstdint>
#include <string>

namespace kotlin::concurrent::atomics {
/**
 * An [Int] value that may be updated atomically.
 *
 * Read operation [load] has the same memory effects as reading a [Volatile] property;
 * Write operation [store] has the same memory effects as writing a [Volatile] property;
 * Read-modify-write operations, like [exchange], [compareAndSet], [compareAndExchange], [fetchAndAdd], [addAndFetch],
 * have the same memory effects as reading and writing a [Volatile] property.
 *
 * For additional details about atomicity guarantees for reads and writes see [kotlin.concurrent.Volatile].
 *
 * @constructor Creates a new [AtomicInt] initialized with the specified value.
 */
// NOTE(port): Primitive compiler-owned fields map the Native sequentially
// consistent LLVM operations to C++ atomic instructions. This class does not
// implement Native ObjHeader layout or reference-atomic GC/runtime operations.
// NOTE(port): C++ deprecated attributes warn; the source ERROR-level and
// experimental opt-in diagnostics still require compiler metadata lowering.
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/concurrent/atomics/Atomics.native.kt:30-144
class AtomicInt final {
 public:
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/concurrent/atomics/Atomics.native.kt:30-34
  explicit AtomicInt(std::int32_t value);
/**
 * Atomically loads the value from this [AtomicInt].
 *
 * @sample samples.concurrent.atomics.AtomicInt.load
 */
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/concurrent/atomics/Atomics.native.kt:40-40
  std::int32_t load() const;
/**
 * Atomically stores the [new value][newValue] into this [AtomicInt].
 *
 * @sample samples.concurrent.atomics.AtomicInt.store
 */
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/concurrent/atomics/Atomics.native.kt:47-47
  void store(std::int32_t new_value);
/**
 * Atomically stores the [new value][newValue] into this [AtomicInt] and returns the old value.
 *
 * @sample samples.concurrent.atomics.AtomicInt.exchange
 */
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/concurrent/atomics/Atomics.native.kt:54-54
  std::int32_t exchange(std::int32_t new_value);
/**
 * Atomically stores the given [new value][newValue] into this [AtomicInt] if the current value equals the [expected value][expectedValue],
 * returns true if the operation was successful and false only if the current value was not equal to the expected value.
 *
 * This operation has so-called strong semantics,
 * meaning that it returns false if and only if current and expected values are not equal.
 *
 * Comparison of values is done by value.
 *
 * @sample samples.concurrent.atomics.AtomicInt.compareAndSet
 */
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/concurrent/atomics/Atomics.native.kt:67-68
  bool compare_and_set(std::int32_t expected_value, std::int32_t new_value);
/**
 * Atomically stores the given [new value][newValue] into this [AtomicInt] if the current value equals the [expected value][expectedValue]
 * and returns the old value in any case.
 *
 * Comparison of values is done by value.
 *
 * @sample samples.concurrent.atomics.AtomicInt.compareAndExchange
 */
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/concurrent/atomics/Atomics.native.kt:78-79
  std::int32_t compare_and_exchange(std::int32_t expected_value, std::int32_t new_value);
/**
 * Atomically adds the [given value][delta] to the current value of this [AtomicInt] and returns the old value.
 *
 * @sample samples.concurrent.atomics.AtomicInt.fetchAndAdd
 */
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/concurrent/atomics/Atomics.native.kt:86-86
  std::int32_t fetch_and_add(std::int32_t delta);
/**
 * Atomically adds the [given value][delta] to the current value of this [AtomicInt] and returns the new value.
 *
 * @sample samples.concurrent.atomics.AtomicInt.addAndFetch
 */
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/concurrent/atomics/Atomics.native.kt:93-93
  std::int32_t add_and_fetch(std::int32_t delta);
/**
 * Atomically sets the value to the given [new value][newValue] and returns the old value.
 */
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/concurrent/atomics/Atomics.native.kt:99-99
  [[deprecated("Use exchange(newValue: Int) instead.")]]
  std::int32_t get_and_set(std::int32_t new_value);
/**
 * Atomically adds the [given value][delta] to the current value and returns the old value.
 */
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/concurrent/atomics/Atomics.native.kt:105-105
  [[deprecated("Use fetchAndAdd(newValue: Int) instead.")]]
  std::int32_t get_and_add(std::int32_t delta);
/**
 * Atomically adds the [given value][delta] to the current value and returns the new value.
 */
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/concurrent/atomics/Atomics.native.kt:111-111
  [[deprecated("Use addAndFetch(newValue: Int) instead.")]]
  std::int32_t add_and_get(std::int32_t delta);
/**
 * Atomically increments the current value by one and returns the old value.
 */
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/concurrent/atomics/Atomics.native.kt:117-117
  [[deprecated("Use fetchAndIncrement() instead.")]]
  std::int32_t get_and_increment();
/**
 * Atomically increments the current value by one and returns the new value.
 */
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/concurrent/atomics/Atomics.native.kt:123-123
  [[deprecated("Use incrementAndFetch() instead.")]]
  std::int32_t increment_and_get();
/**
 * Atomically decrements the current value by one and returns the new value.
 */
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/concurrent/atomics/Atomics.native.kt:129-129
  [[deprecated("Use decrementAndFetch() instead.")]]
  std::int32_t decrement_and_get();
/**
 * Atomically decrements the current value by one and returns the old value.
 */
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/concurrent/atomics/Atomics.native.kt:135-135
  [[deprecated("Use fetchAndDecrement() instead.")]]
  std::int32_t get_and_decrement();
/**
 * Returns the string representation of the [Int] value stored in this [AtomicInt].
 *
 * This operation does not provide any atomicity guarantees.
 */
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/concurrent/atomics/Atomics.native.kt:143-143
  std::string to_string() const;
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/concurrent/atomics/Atomics.native.kt:30-34
  [[deprecated("To read the atomic value use load().")]] std::int32_t get_value() const;
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/concurrent/atomics/Atomics.native.kt:30-34
  [[deprecated("To atomically set the new value use store().")]] void set_value(std::int32_t new_value);
 private:
  std::atomic<std::int32_t> value_;
};

/**
 * Atomically adds the [given value][delta] to the current value of this [AtomicInt].
 *
 * @sample samples.concurrent.atomics.AtomicInt.plusAssign
 */
// Transliterated from: libraries/stdlib/src/kotlin/concurrent/atomics/Atomics.common.kt:107-107
void plus_assign(AtomicInt& receiver, std::int32_t delta);

/**
 * Atomically subtracts the [given value][delta] from the current value of this [AtomicInt].
 *
 * @sample samples.concurrent.atomics.AtomicInt.minusAssign
 */
// Transliterated from: libraries/stdlib/src/kotlin/concurrent/atomics/Atomics.common.kt:116-116
void minus_assign(AtomicInt& receiver, std::int32_t delta);

/**
 * Atomically increments the current value of this [AtomicInt] by one and returns the old value.
 *
 * @sample samples.concurrent.atomics.AtomicInt.fetchAndIncrement
 */
// Transliterated from: libraries/stdlib/src/kotlin/concurrent/atomics/Atomics.common.kt:125-125
std::int32_t fetch_and_increment(AtomicInt& receiver);

/**
 * Atomically increments the current value of this [AtomicInt] by one and returns the new value.
 *
 * @sample samples.concurrent.atomics.AtomicInt.incrementAndFetch
 */
// Transliterated from: libraries/stdlib/src/kotlin/concurrent/atomics/Atomics.common.kt:134-134
std::int32_t increment_and_fetch(AtomicInt& receiver);

/**
 * Atomically decrements the current value of this [AtomicInt] by one and returns the new value.
 *
 * @sample samples.concurrent.atomics.AtomicInt.decrementAndFetch
 */
// Transliterated from: libraries/stdlib/src/kotlin/concurrent/atomics/Atomics.common.kt:143-143
std::int32_t decrement_and_fetch(AtomicInt& receiver);

/**
 * Atomically decrements the current value of this [AtomicInt] by one and returns the old value.
 *
 * @sample samples.concurrent.atomics.AtomicInt.fetchAndDecrement
 */
// Transliterated from: libraries/stdlib/src/kotlin/concurrent/atomics/Atomics.common.kt:152-152
std::int32_t fetch_and_decrement(AtomicInt& receiver);

/**
 * Atomically updates the value of this [AtomicInt] with the value obtained by calling the [transform] function on the current value
 * and returns the value replaced by the updated one.
 *
 * [transform] may be invoked more than once to recompute a result.
 * That may happen, for example, when this atomic integer value was concurrently updated while [transform] was applied,
 * or due to a spurious compare-and-set failure.
 * The latter is implementation-specific, and it should not be relied upon.
 *
 * It's recommended to keep [transform] fast and free of side effects.
 *
 * @sample samples.concurrent.atomics.AtomicInt.fetchAndUpdate
 */
// NOTE(port): The public inline function parameter is a C++ callable template,
// retaining captured and move-only closures without an erased callable copy.
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/concurrent/atomics/Atomics.native.kt:542-551
template <typename Transform>
std::int32_t fetch_and_update(AtomicInt& receiver, Transform&& transform) {
  while (true) {
    const auto old = receiver.load();
    const auto new_value = transform(old);
    if (receiver.compare_and_set(old, new_value)) return old;
  }
}

/**
 * Atomically updates the value of this [AtomicInt] with the value obtained by calling the [transform] function on the current value
 * and returns the new value.
 *
 * [transform] may be invoked more than once to recompute a result.
 * That may happen, for example, when this atomic integer value was concurrently updated while [transform] was applied,
 * or due to a spurious compare-and-set failure.
 * The latter is implementation-specific, and it should not be relied upon.
 *
 * It's recommended to keep [transform] fast and free of side effects.
 *
 * @sample samples.concurrent.atomics.AtomicInt.updateAndFetch
 */
// NOTE(port): The public inline function parameter is a C++ callable template,
// retaining captured and move-only closures without an erased callable copy.
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/concurrent/atomics/Atomics.native.kt:569-578
template <typename Transform>
std::int32_t update_and_fetch(AtomicInt& receiver, Transform&& transform) {
  while (true) {
    const auto old = receiver.load();
    const auto new_value = transform(old);
    if (receiver.compare_and_set(old, new_value)) return new_value;
  }
}

/**
 *
 * Atomically updates the value of this [AtomicInt] with the value obtained by calling the [transform] function on the current value.
 *
 * [transform] may be invoked more than once to recompute a result.
 * That may happen, for example, when this atomic integer value was concurrently updated while [transform] was applied,
 * or due to a spurious compare-and-set failure.
 * The latter is implementation-specific, and it should not be relied upon.
 *
 * It's recommended to keep [transform] fast and free of side effects.
 *
 * @sample samples.concurrent.atomics.AtomicInt.update
 */
// NOTE(port): The public inline function parameter is a C++ callable template,
// retaining captured and move-only closures without an erased callable copy.
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/concurrent/atomics/Atomics.native.kt:519-524
template <typename Transform>
void update(AtomicInt& receiver, Transform&& transform) {
  static_cast<void>(fetch_and_update(receiver, transform));
}

}  // namespace kotlin::concurrent::atomics
