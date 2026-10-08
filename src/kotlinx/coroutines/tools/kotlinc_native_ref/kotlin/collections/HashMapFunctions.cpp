// port-lint: source libraries/stdlib/native-wasm/src/kotlin/collections/HashMap.kt
// Transliterated from: libraries/stdlib/native-wasm/src/kotlin/collections/HashMap.kt:599-601
#include "HashMapFunctions.hpp"
#include "../Numbers.hpp"
#include <algorithm>
#include <bit>
namespace kotlin::collections::hash_map::detail {
// Transliterated from: libraries/stdlib/native-wasm/src/kotlin/collections/HashMap.kt:599-599
std::int32_t compute_hash_size(std::int32_t capacity) {
    // NOTE(port): Kotlin Int multiplication wraps before takeHighestOneBit.
    const auto product = std::bit_cast<std::int32_t>(
        static_cast<std::uint32_t>(std::max(capacity, std::int32_t{1})) * 3U);
    return kotlin::take_highest_one_bit(product);
}
// Transliterated from: libraries/stdlib/native-wasm/src/kotlin/collections/HashMap.kt:601-601
std::int32_t compute_shift(std::int32_t hash_size) {
    return kotlin::count_leading_zero_bits(hash_size) + 1;
}
}
