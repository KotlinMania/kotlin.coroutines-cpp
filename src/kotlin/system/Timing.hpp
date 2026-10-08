/**
 * Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/system/Timing.kt
 */
#ifndef KOTLIN_SYSTEM_TIMING_HPP_
#define KOTLIN_SYSTEM_TIMING_HPP_
#include <bit>
#include <cstdint>

namespace kotlin::system {
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/system/Timing.kt:11-26
// Only differences between readings are meaningful. Upstream deprecates this API
// in favor of measureTime or TimeSource.Monotonic.markNow.
long long get_time_millis();
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/system/Timing.kt:28-43
long long get_time_nanos();
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/system/Timing.kt:45-60
long long get_time_micros();

// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/system/Timing.kt:62-80
// NOTE(port): The source inline callable is a public C++ template to preserve
// the actual caller's callable type without copying or imposing type erasure.
template <typename Block>
long long measure_time_millis(Block&& block) {
    auto start = get_time_millis();
    block();
    // NOTE(port): Kotlin Long subtraction wraps; unsigned bits preserve it.
    return std::bit_cast<long long>(static_cast<std::uint64_t>(get_time_millis()) -
                                   static_cast<std::uint64_t>(start));
}
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/system/Timing.kt:82-98
template <typename Block>
long long measure_time_micros(Block&& block) {
    auto start = get_time_micros();
    block();
    return std::bit_cast<long long>(static_cast<std::uint64_t>(get_time_micros()) -
                                   static_cast<std::uint64_t>(start));
}
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/system/Timing.kt:100-118
template <typename Block>
long long measure_nano_time(Block&& block) {
    auto start = get_time_nanos();
    block();
    return std::bit_cast<long long>(static_cast<std::uint64_t>(get_time_nanos()) -
                                   static_cast<std::uint64_t>(start));
}
} // namespace kotlin::system
#endif
