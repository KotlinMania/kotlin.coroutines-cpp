/*
 * Copyright 2010-2024 JetBrains s.r.o. and Kotlin Programming Language contributors.
 * Use of this source code is governed by the Apache 2.0 license that can be found in the license/LICENSE.txt file.
 */

// port-lint: source libraries/stdlib/src/kotlin/collections/PrimitiveIterators.kt
// Transliterated from: libraries/stdlib/src/kotlin/collections/PrimitiveIterators.kt:12-347
#include "PrimitiveIterators.hpp"
namespace kotlin::collections {
// Transliterated from: libraries/stdlib/src/kotlin/collections/PrimitiveIterators.kt:44-44
std::int8_t ByteIterator::next() { return next_byte(); }
// Transliterated from: libraries/stdlib/src/kotlin/collections/PrimitiveIterators.kt:44-44
std::any ByteIterator::next_dispatch() {
  return detail::ElementCodec<std::int8_t>::box(next());
}

// Transliterated from: libraries/stdlib/src/kotlin/collections/PrimitiveIterators.kt:86-86
char16_t CharIterator::next() { return next_char(); }
// Transliterated from: libraries/stdlib/src/kotlin/collections/PrimitiveIterators.kt:86-86
std::any CharIterator::next_dispatch() {
  return detail::ElementCodec<char16_t>::box(next());
}

// Transliterated from: libraries/stdlib/src/kotlin/collections/PrimitiveIterators.kt:128-128
std::int16_t ShortIterator::next() { return next_short(); }
// Transliterated from: libraries/stdlib/src/kotlin/collections/PrimitiveIterators.kt:128-128
std::any ShortIterator::next_dispatch() {
  return detail::ElementCodec<std::int16_t>::box(next());
}

// Transliterated from: libraries/stdlib/src/kotlin/collections/PrimitiveIterators.kt:170-170
std::int32_t IntIterator::next() { return next_int(); }
// Transliterated from: libraries/stdlib/src/kotlin/collections/PrimitiveIterators.kt:170-170
std::any IntIterator::next_dispatch() {
  return detail::ElementCodec<std::int32_t>::box(next());
}

// Transliterated from: libraries/stdlib/src/kotlin/collections/PrimitiveIterators.kt:212-212
std::int64_t LongIterator::next() { return next_long(); }
// Transliterated from: libraries/stdlib/src/kotlin/collections/PrimitiveIterators.kt:212-212
std::any LongIterator::next_dispatch() {
  return detail::ElementCodec<std::int64_t>::box(next());
}

// Transliterated from: libraries/stdlib/src/kotlin/collections/PrimitiveIterators.kt:254-254
float FloatIterator::next() { return next_float(); }
// Transliterated from: libraries/stdlib/src/kotlin/collections/PrimitiveIterators.kt:254-254
std::any FloatIterator::next_dispatch() {
  return detail::ElementCodec<float>::box(next());
}

// Transliterated from: libraries/stdlib/src/kotlin/collections/PrimitiveIterators.kt:296-296
double DoubleIterator::next() { return next_double(); }
// Transliterated from: libraries/stdlib/src/kotlin/collections/PrimitiveIterators.kt:296-296
std::any DoubleIterator::next_dispatch() {
  return detail::ElementCodec<double>::box(next());
}

// Transliterated from: libraries/stdlib/src/kotlin/collections/PrimitiveIterators.kt:338-338
bool BooleanIterator::next() { return next_boolean(); }
// Transliterated from: libraries/stdlib/src/kotlin/collections/PrimitiveIterators.kt:338-338
std::any BooleanIterator::next_dispatch() {
  return detail::ElementCodec<bool>::box(next());
}

}  // namespace kotlin::collections
