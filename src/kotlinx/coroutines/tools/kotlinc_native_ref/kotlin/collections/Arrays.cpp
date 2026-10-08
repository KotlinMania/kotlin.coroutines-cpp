/*
 * Copyright 2010-2018 JetBrains s.r.o. Use of this source code is governed by the Apache 2.0 license
 * that can be found in the LICENSE file.
 */
// port-lint: source libraries/stdlib/native-wasm/src/kotlin/collections/Arrays.kt
// Transliterated from: libraries/stdlib/native-wasm/src/kotlin/collections/Arrays.kt:84-89
#include "Arrays.hpp"
#include "CollectionToArray.hpp"

namespace kotlin::collections {
// Transliterated from: libraries/stdlib/native-wasm/src/kotlin/collections/Arrays.kt:85-85
// NOTE(port): The Native body does not read its reference argument.
Array<std::any> array_of_nulls(const Array<std::any>&,
                               std::int32_t size) {
  return ::kotlin::array_of_nulls<std::any>(size);
}

// Transliterated from: libraries/stdlib/native-wasm/src/kotlin/collections/Arrays.kt:87-87
Array<std::any> collection_to_array(const Collection<std::any>& collection) {
  return collection_to_array_common_impl(collection);
}

// Transliterated from: libraries/stdlib/native-wasm/src/kotlin/collections/Arrays.kt:89-89
Array<std::any> collection_to_array(const Collection<std::any>& collection,
                                  Array<std::any> array) {
  return collection_to_array_common_impl(collection, std::move(array));
}
}  // namespace kotlin::collections
