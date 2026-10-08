/**
 * Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/system/Timing.kt
 * Source runtime operations: kotlin-native/runtime/src/main/cpp/Porting.cpp:260-276
 */
#include "kotlin/system/Timing.hpp"
#include <chrono>
#include <type_traits>

namespace kotlin::system {
namespace {
// Transliterated from: kotlin-native/runtime/src/main/cpp/Porting.cpp:263-264
using SteadyTimeClock = std::conditional_t<std::chrono::high_resolution_clock::is_steady,
    std::chrono::high_resolution_clock, std::chrono::steady_clock>;
} // namespace

// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/system/Timing.kt:26
// Transliterated from: kotlin-native/runtime/src/main/cpp/Time.cpp:25-29
// Transliterated from: kotlin-native/runtime/src/main/cpp/Porting.cpp:266-268
// NOTE(port): Standalone C++ executes the original platform clock operation
// directly. It exports no Kotlin runtime symbol and supplies no Native calls
// checker/GC guard. Native interop requires the actual linked runtime boundary.
long long get_time_millis() {
    auto reading = std::chrono::duration_cast<std::chrono::milliseconds>(
        SteadyTimeClock::now().time_since_epoch()).count();
    // NOTE(port): Preserve the original uint64_t-to-KLong bit representation.
    return std::bit_cast<long long>(static_cast<std::uint64_t>(reading));
}
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/system/Timing.kt:43
// Transliterated from: kotlin-native/runtime/src/main/cpp/Time.cpp:31-35
// Transliterated from: kotlin-native/runtime/src/main/cpp/Porting.cpp:270-272
long long get_time_nanos() {
    auto reading = std::chrono::duration_cast<std::chrono::nanoseconds>(
        SteadyTimeClock::now().time_since_epoch()).count();
    return std::bit_cast<long long>(static_cast<std::uint64_t>(reading));
}
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/system/Timing.kt:60
// Transliterated from: kotlin-native/runtime/src/main/cpp/Time.cpp:37-41
// Transliterated from: kotlin-native/runtime/src/main/cpp/Porting.cpp:274-276
long long get_time_micros() {
    auto reading = std::chrono::duration_cast<std::chrono::microseconds>(
        SteadyTimeClock::now().time_since_epoch()).count();
    return std::bit_cast<long long>(static_cast<std::uint64_t>(reading));
}
} // namespace kotlin::system
