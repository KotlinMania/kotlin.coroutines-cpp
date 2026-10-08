/**
 * Transliterated from: libraries/stdlib/src/kotlin/time/TimeSource.kt
 */
#include "kotlin/time/ComparableTimeMark.hpp"

namespace kotlin::time {
// Transliterated from: libraries/stdlib/src/kotlin/time/TimeSource.kt:204
ComparableTimeMark* ComparableTimeMark::minus(Duration duration) { return plus(-duration); }
// Transliterated from: libraries/stdlib/src/kotlin/time/TimeSource.kt:234-235
int ComparableTimeMark::compare_to(const ComparableTimeMark& other) const {
    return minus(other).compare_to(Duration::ZERO);
}
} // namespace kotlin::time
