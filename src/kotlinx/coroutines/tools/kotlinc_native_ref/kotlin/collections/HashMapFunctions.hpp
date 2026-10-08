// port-lint: source libraries/stdlib/native-wasm/src/kotlin/collections/HashMap.kt
// Transliterated from: libraries/stdlib/native-wasm/src/kotlin/collections/HashMap.kt:599-601
#pragma once
#include <cstdint>

// NOTE(port): These private companion operations use the same namespace
// mapping as AbstractListFunctions. No replacement HashMap class is supplied.
// Declarations serve the compiler's internal collection implementation only.
namespace kotlin::collections::hash_map::detail {
// Transliterated from: libraries/stdlib/native-wasm/src/kotlin/collections/HashMap.kt:599-599
std::int32_t compute_hash_size(std::int32_t capacity);
// Transliterated from: libraries/stdlib/native-wasm/src/kotlin/collections/HashMap.kt:601-601
std::int32_t compute_shift(std::int32_t hash_size);
}
