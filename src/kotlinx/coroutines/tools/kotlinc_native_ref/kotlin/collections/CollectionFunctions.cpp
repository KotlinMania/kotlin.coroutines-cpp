/*
 * Copyright 2010-2025 JetBrains s.r.o. and Kotlin Programming Language contributors.
 * Use of this source code is governed by the Apache 2.0 license that can be found in the license/LICENSE.txt file.
 */
// port-lint: source libraries/stdlib/native-wasm/src/kotlin/collections/Collections.kt
// Transliterated from: libraries/stdlib/native-wasm/src/kotlin/collections/Collections.kt:108-113
// port-lint: source libraries/stdlib/src/kotlin/collections/Collections.kt
#include "CollectionFunctions.hpp"
#include <stdexcept>
namespace kotlin::collections {
// Transliterated from: libraries/stdlib/native-wasm/src/kotlin/collections/Collections.kt:108-113
std::int32_t check_index_overflow(std::int32_t index) {
  if (index < 0) {
    throw_index_overflow();
  }
  return index;
}
// NOTE(port): Source ArithmeticException maps to the C++ arithmetic error
// category here; this compiler helper does not construct a Native exception box.
// Transliterated from: libraries/stdlib/src/kotlin/collections/Collections.kt:507-507
[[noreturn]] void throw_index_overflow() {
  throw std::overflow_error("Index overflow has happened.");
}
}  // namespace kotlin::collections
