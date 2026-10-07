/*
 * Copyright 2010-2020 JetBrains s.r.o. and Kotlin Programming Language contributors.
 * Use of this source code is governed by the Apache 2.0 license that can be found in the license/LICENSE.txt file.
 */
// port-lint: source libraries/stdlib/src/kotlin/collections/AbstractList.kt
// Transliterated from: libraries/stdlib/src/kotlin/collections/AbstractList.kt:115-157
#pragma once
#include <cstdint>

// NOTE(port): These actual companion functions use the permitted namespace
// mapping. The full AbstractList class and its collection algorithms remain
// separate source dependencies; no partial replacement class is introduced.
namespace kotlin::collections::abstract_list {
// Transliterated from: libraries/stdlib/src/kotlin/collections/AbstractList.kt:116-120
void check_element_index(std::int32_t index, std::int32_t size);
// Transliterated from: libraries/stdlib/src/kotlin/collections/AbstractList.kt:122-126
void check_position_index(std::int32_t index, std::int32_t size);
// Transliterated from: libraries/stdlib/src/kotlin/collections/AbstractList.kt:128-135
void check_range_indexes(std::int32_t from_index, std::int32_t to_index, std::int32_t size);
// Transliterated from: libraries/stdlib/src/kotlin/collections/AbstractList.kt:137-144
void check_bounds_indexes(std::int32_t start_index, std::int32_t end_index, std::int32_t size);
/** [oldCapacity] and [minCapacity] must be non-negative. */
// Transliterated from: libraries/stdlib/src/kotlin/collections/AbstractList.kt:149-157
std::int32_t new_capacity(std::int32_t old_capacity, std::int32_t min_capacity);
}  // namespace kotlin::collections::abstract_list
