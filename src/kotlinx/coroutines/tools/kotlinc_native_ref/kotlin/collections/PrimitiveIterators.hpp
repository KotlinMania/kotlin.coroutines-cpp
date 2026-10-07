/*
 * Copyright 2010-2024 JetBrains s.r.o. and Kotlin Programming Language contributors.
 * Use of this source code is governed by the Apache 2.0 license that can be found in the license/LICENSE.txt file.
 */

// port-lint: source libraries/stdlib/src/kotlin/collections/PrimitiveIterators.kt
// Transliterated from: libraries/stdlib/src/kotlin/collections/PrimitiveIterators.kt:12-347
#pragma once
#include "Iterator.hpp"
namespace kotlin::collections {
/**
 * An iterator over a sequence of values of type `Byte`.
 *
 * This is a substitute for `Iterator<Byte>` that provides a specialized version of `next(): T` method: `nextByte(): Byte`
 * and has a special handling by the compiler to avoid platform-specific boxing conversions as a performance optimization.
 *
 * In the following example:
 *
 * ```kotlin
 * class ByteContainer(private val data: ByteArray) {
 *
 *     // ByteIterator instead of Iterator<Byte> in the signature
 *     operator fun iterator(): ByteIterator = object : ByteIterator() {
 *         private var idx = 0
 *
 *         override fun nextByte(): Byte {
 *             if (!hasNext()) throw NoSuchElementException()
 *             return data[idx++]
 *         }
 *
 *         override fun hasNext(): Boolean = idx < data.size
 *     }
 * }
 *
 * for (element in ByteContainer(byteArrayOf(1, 2, 3))) {
 *     ... handle element ...
 * }
 * ```
 * No boxing conversion is performed during the for-loop iteration.
 * Note that the iterator itself will still be allocated.
 */
// Transliterated from: libraries/stdlib/src/kotlin/collections/PrimitiveIterators.kt:43-52
class ByteIterator : public Iterator<std::int8_t> {
 public:
  // Transliterated from: libraries/stdlib/src/kotlin/collections/PrimitiveIterators.kt:44-44
  std::int8_t next();
/**
     * Returns the next element in the iteration without boxing conversion.
     * @throws NoSuchElementException if the iteration has no next element.
     */
  // Transliterated from: libraries/stdlib/src/kotlin/collections/PrimitiveIterators.kt:51-51
  virtual std::int8_t next_byte() = 0;
 private:
  // NOTE(port): The typed source next remains unboxed. Only the existing C++
  // generic/covariant virtual boundary transports a boxed value.
  // Transliterated from: libraries/stdlib/src/kotlin/collections/PrimitiveIterators.kt:44-44
  std::any next_dispatch() final;
};

/**
 * An iterator over a sequence of values of type `Char`.
 *
 * This is a substitute for `Iterator<Char>` that provides a specialized version of `next(): T` method: `nextChar(): Char`
 * and has a special handling by the compiler to avoid platform-specific boxing conversions as a performance optimization.
 *
 * In the following example:
 *
 * ```kotlin
 * class CharContainer(private val data: CharArray) {
 *
 *     // CharIterator instead of Iterator<Char> in the signature
 *     operator fun iterator(): CharIterator = object : CharIterator() {
 *         private var idx = 0
 *
 *         override fun nextChar(): Char {
 *             if (!hasNext()) throw NoSuchElementException()
 *             return data[idx++]
 *         }
 *
 *         override fun hasNext(): Boolean = idx < data.size
 *     }
 * }
 *
 * for (element in CharContainer(charArrayOf('1', '2', '3'))) {
 *     ... handle element ...
 * }
 * ```
 * No boxing conversion is performed during the for-loop iteration.
 * Note that the iterator itself will still be allocated.
 */
// Transliterated from: libraries/stdlib/src/kotlin/collections/PrimitiveIterators.kt:85-94
class CharIterator : public Iterator<char16_t> {
 public:
  // Transliterated from: libraries/stdlib/src/kotlin/collections/PrimitiveIterators.kt:86-86
  char16_t next();
/**
     * Returns the next element in the iteration without boxing conversion.
     * @throws NoSuchElementException if the iteration has no next element.
     */
  // Transliterated from: libraries/stdlib/src/kotlin/collections/PrimitiveIterators.kt:93-93
  virtual char16_t next_char() = 0;
 private:
  // NOTE(port): The typed source next remains unboxed. Only the existing C++
  // generic/covariant virtual boundary transports a boxed value.
  // Transliterated from: libraries/stdlib/src/kotlin/collections/PrimitiveIterators.kt:86-86
  std::any next_dispatch() final;
};

/**
 * An iterator over a sequence of values of type `Short`.
 *
 * This is a substitute for `Iterator<Short>` that provides a specialized version of `next(): T` method: `nextShort(): Short`
 * and has a special handling by the compiler to avoid platform-specific boxing conversions as a performance optimization.
 *
 * In the following example:
 *
 * ```kotlin
 * class ShortContainer(private val data: ShortArray) {
 *
 *     // ShortIterator instead of Iterator<Short> in the signature
 *     operator fun iterator(): ShortIterator = object : ShortIterator() {
 *         private var idx = 0
 *
 *         override fun nextShort(): Short {
 *             if (!hasNext()) throw NoSuchElementException()
 *             return data[idx++]
 *         }
 *
 *         override fun hasNext(): Boolean = idx < data.size
 *     }
 * }
 *
 * for (element in ShortContainer(shortArrayOf(1, 2, 3))) {
 *     ... handle element ...
 * }
 * ```
 * No boxing conversion is performed during the for-loop iteration.
 * Note that the iterator itself will still be allocated.
 */
// Transliterated from: libraries/stdlib/src/kotlin/collections/PrimitiveIterators.kt:127-136
class ShortIterator : public Iterator<std::int16_t> {
 public:
  // Transliterated from: libraries/stdlib/src/kotlin/collections/PrimitiveIterators.kt:128-128
  std::int16_t next();
/**
     * Returns the next element in the iteration without boxing conversion.
     * @throws NoSuchElementException if the iteration has no next element.
     */
  // Transliterated from: libraries/stdlib/src/kotlin/collections/PrimitiveIterators.kt:135-135
  virtual std::int16_t next_short() = 0;
 private:
  // NOTE(port): The typed source next remains unboxed. Only the existing C++
  // generic/covariant virtual boundary transports a boxed value.
  // Transliterated from: libraries/stdlib/src/kotlin/collections/PrimitiveIterators.kt:128-128
  std::any next_dispatch() final;
};

/**
 * An iterator over a sequence of values of type `Int`.
 *
 * This is a substitute for `Iterator<Int>` that provides a specialized version of `next(): T` method: `nextInt(): Int`
 * and has a special handling by the compiler to avoid platform-specific boxing conversions as a performance optimization.
 *
 * In the following example:
 *
 * ```kotlin
 * class IntContainer(private val data: IntArray) {
 *
 *     // IntIterator instead of Iterator<Int> in the signature
 *     operator fun iterator(): IntIterator = object : IntIterator() {
 *         private var idx = 0
 *
 *         override fun nextInt(): Int {
 *             if (!hasNext()) throw NoSuchElementException()
 *             return data[idx++]
 *         }
 *
 *         override fun hasNext(): Boolean = idx < data.size
 *     }
 * }
 *
 * for (element in IntContainer(intArrayOf(1, 2, 3))) {
 *     ... handle element ...
 * }
 * ```
 * No boxing conversion is performed during the for-loop iteration.
 * Note that the iterator itself will still be allocated.
 */
// Transliterated from: libraries/stdlib/src/kotlin/collections/PrimitiveIterators.kt:169-178
class IntIterator : public Iterator<std::int32_t> {
 public:
  // Transliterated from: libraries/stdlib/src/kotlin/collections/PrimitiveIterators.kt:170-170
  std::int32_t next();
/**
     * Returns the next element in the iteration without boxing conversion.
     * @throws NoSuchElementException if the iteration has no next element.
     */
  // Transliterated from: libraries/stdlib/src/kotlin/collections/PrimitiveIterators.kt:177-177
  virtual std::int32_t next_int() = 0;
 private:
  // NOTE(port): The typed source next remains unboxed. Only the existing C++
  // generic/covariant virtual boundary transports a boxed value.
  // Transliterated from: libraries/stdlib/src/kotlin/collections/PrimitiveIterators.kt:170-170
  std::any next_dispatch() final;
};

/**
 * An iterator over a sequence of values of type `Long`.
 *
 * This is a substitute for `Iterator<Long>` that provides a specialized version of `next(): T` method: `nextLong(): Long`
 * and has a special handling by the compiler to avoid platform-specific boxing conversions as a performance optimization.
 *
 * In the following example:
 *
 * ```kotlin
 * class LongContainer(private val data: LongArray) {
 *
 *     // LongIterator instead of Iterator<Long> in the signature
 *     operator fun iterator(): LongIterator = object : LongIterator() {
 *         private var idx = 0
 *
 *         override fun nextLong(): Long {
 *             if (!hasNext()) throw NoSuchElementException()
 *             return data[idx++]
 *         }
 *
 *         override fun hasNext(): Boolean = idx < data.size
 *     }
 * }
 *
 * for (element in LongContainer(longArrayOf(1, 2, 3))) {
 *     ... handle element ...
 * }
 * ```
 * No boxing conversion is performed during the for-loop iteration.
 * Note that the iterator itself will still be allocated.
 */
// Transliterated from: libraries/stdlib/src/kotlin/collections/PrimitiveIterators.kt:211-220
class LongIterator : public Iterator<std::int64_t> {
 public:
  // Transliterated from: libraries/stdlib/src/kotlin/collections/PrimitiveIterators.kt:212-212
  std::int64_t next();
/**
     * Returns the next element in the iteration without boxing conversion.
     * @throws NoSuchElementException if the iteration has no next element.
     */
  // Transliterated from: libraries/stdlib/src/kotlin/collections/PrimitiveIterators.kt:219-219
  virtual std::int64_t next_long() = 0;
 private:
  // NOTE(port): The typed source next remains unboxed. Only the existing C++
  // generic/covariant virtual boundary transports a boxed value.
  // Transliterated from: libraries/stdlib/src/kotlin/collections/PrimitiveIterators.kt:212-212
  std::any next_dispatch() final;
};

/**
 * An iterator over a sequence of values of type `Float`.
 *
 * This is a substitute for `Iterator<Float>` that provides a specialized version of `next(): T` method: `nextFloat(): Float`
 * and has a special handling by the compiler to avoid platform-specific boxing conversions as a performance optimization.
 *
 * In the following example:
 *
 * ```kotlin
 * class FloatContainer(private val data: FloatArray) {
 *
 *     // FloatIterator instead of Iterator<Float> in the signature
 *     operator fun iterator(): FloatIterator = object : FloatIterator() {
 *         private var idx = 0
 *
 *         override fun nextFloat(): Float {
 *             if (!hasNext()) throw NoSuchElementException()
 *             return data[idx++]
 *         }
 *
 *         override fun hasNext(): Boolean = idx < data.size
 *     }
 * }
 *
 * for (element in FloatContainer(floatArrayOf(1f, 2f, 3f))) {
 *     ... handle element ...
 * }
 * ```
 * No boxing conversion is performed during the for-loop iteration.
 * Note that the iterator itself will still be allocated.
 */
// Transliterated from: libraries/stdlib/src/kotlin/collections/PrimitiveIterators.kt:253-262
class FloatIterator : public Iterator<float> {
 public:
  // Transliterated from: libraries/stdlib/src/kotlin/collections/PrimitiveIterators.kt:254-254
  float next();
/**
     * Returns the next element in the iteration without boxing conversion.
     * @throws NoSuchElementException if the iteration has no next element.
     */
  // Transliterated from: libraries/stdlib/src/kotlin/collections/PrimitiveIterators.kt:261-261
  virtual float next_float() = 0;
 private:
  // NOTE(port): The typed source next remains unboxed. Only the existing C++
  // generic/covariant virtual boundary transports a boxed value.
  // Transliterated from: libraries/stdlib/src/kotlin/collections/PrimitiveIterators.kt:254-254
  std::any next_dispatch() final;
};

/**
 * An iterator over a sequence of values of type `Double`.
 *
 * This is a substitute for `Iterator<Double>` that provides a specialized version of `next(): T` method: `nextDouble(): Double`
 * and has a special handling by the compiler to avoid platform-specific boxing conversions as a performance optimization.
 *
 * In the following example:
 *
 * ```kotlin
 * class DoubleContainer(private val data: DoubleArray) {
 *
 *     // DoubleIterator instead of Iterator<Double> in the signature
 *     operator fun iterator(): DoubleIterator = object : DoubleIterator() {
 *         private var idx = 0
 *
 *         override fun nextDouble(): Double {
 *             if (!hasNext()) throw NoSuchElementException()
 *             return data[idx++]
 *         }
 *
 *         override fun hasNext(): Boolean = idx < data.size
 *     }
 * }
 *
 * for (element in DoubleContainer(doubleArrayOf(1.0, 2.0, 3.0))) {
 *     ... handle element ...
 * }
 * ```
 * No boxing conversion is performed during the for-loop iteration.
 * Note that the iterator itself will still be allocated.
 */
// Transliterated from: libraries/stdlib/src/kotlin/collections/PrimitiveIterators.kt:295-304
class DoubleIterator : public Iterator<double> {
 public:
  // Transliterated from: libraries/stdlib/src/kotlin/collections/PrimitiveIterators.kt:296-296
  double next();
/**
     * Returns the next element in the iteration without boxing conversion.
     * @throws NoSuchElementException if the iteration has no next element.
     */
  // Transliterated from: libraries/stdlib/src/kotlin/collections/PrimitiveIterators.kt:303-303
  virtual double next_double() = 0;
 private:
  // NOTE(port): The typed source next remains unboxed. Only the existing C++
  // generic/covariant virtual boundary transports a boxed value.
  // Transliterated from: libraries/stdlib/src/kotlin/collections/PrimitiveIterators.kt:296-296
  std::any next_dispatch() final;
};

/**
 * An iterator over a sequence of values of type `Boolean`.
 *
 * This is a substitute for `Iterator<Boolean>` that provides a specialized version of `next(): T` method: `nextBoolean(): Boolean`
 * and has a special handling by the compiler to avoid platform-specific boxing conversions as a performance optimization.
 *
 * In the following example:
 *
 * ```kotlin
 * class BooleanContainer(private val data: BooleanArray) {
 *
 *     // BooleanIterator instead of Iterator<Boolean> in the signature
 *     operator fun iterator(): BooleanIterator = object : BooleanIterator() {
 *         private var idx = 0
 *
 *         override fun nextBoolean(): Boolean {
 *             if (!hasNext()) throw NoSuchElementException()
 *             return data[idx++]
 *         }
 *
 *         override fun hasNext(): Boolean = idx < data.size
 *     }
 * }
 *
 * for (element in BooleanContainer(booleanArrayOf(true, false, true))) {
 *     ... handle element ...
 * }
 * ```
 * No boxing conversion is performed during the for-loop iteration.
 * Note that the iterator itself will still be allocated.
 */
// Transliterated from: libraries/stdlib/src/kotlin/collections/PrimitiveIterators.kt:337-346
class BooleanIterator : public Iterator<bool> {
 public:
  // Transliterated from: libraries/stdlib/src/kotlin/collections/PrimitiveIterators.kt:338-338
  bool next();
/**
     * Returns the next element in the iteration without boxing conversion.
     * @throws NoSuchElementException if the iteration has no next element.
     */
  // Transliterated from: libraries/stdlib/src/kotlin/collections/PrimitiveIterators.kt:345-345
  virtual bool next_boolean() = 0;
 private:
  // NOTE(port): The typed source next remains unboxed. Only the existing C++
  // generic/covariant virtual boundary transports a boxed value.
  // Transliterated from: libraries/stdlib/src/kotlin/collections/PrimitiveIterators.kt:338-338
  std::any next_dispatch() final;
};

}  // namespace kotlin::collections
