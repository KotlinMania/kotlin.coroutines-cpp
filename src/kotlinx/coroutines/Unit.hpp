#pragma once
// port-lint: source kotlin-native/runtime/src/main/kotlin/kotlin/Unit.kt
/**
 * Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/Unit.kt
 */
#include <string>

namespace kotlin {
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/Unit.kt:11-16
// NOTE(port): The existing erased ABI uses equal value carriers for Kotlin's single Unit value.
struct Unit {
    bool operator==(const Unit&) const { return true; }
    bool operator!=(const Unit&) const { return false; }
    std::string to_string() const;
};
} // namespace kotlin

namespace kotlinx::coroutines {
using kotlin::Unit;
} // namespace kotlinx::coroutines
