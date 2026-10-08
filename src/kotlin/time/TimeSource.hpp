/**
 * Transliterated from: libraries/stdlib/src/kotlin/time/TimeSource.kt
 */
#ifndef KOTLIN_TIME_TIME_SOURCE_HPP_
#define KOTLIN_TIME_TIME_SOURCE_HPP_
#include "kotlin/time/ComparableTimeMark.hpp"

namespace kotlin::time {
// Transliterated from: libraries/stdlib/src/kotlin/time/TimeSource.kt:10-41
// A source of time for measuring intervals. Only Monotonic guarantees monotonicity.
class TimeSource {
public:
    class WithComparableMarks;
    // NOTE(port): Virtual destruction preserves ordinary C++ source cleanup.
    virtual ~TimeSource() = default;
    // Transliterated from: libraries/stdlib/src/kotlin/time/TimeSource.kt:25-31
    // NOTE(port): Caller owns the returned mark and must delete/adopt it.
    virtual TimeMark* mark_now() = 0;
};
// Transliterated from: libraries/stdlib/src/kotlin/time/TimeSource.kt:33-41
class TimeSource::WithComparableMarks : public TimeSource {
public:
    // Transliterated from: libraries/stdlib/src/kotlin/time/TimeSource.kt:40
    ComparableTimeMark* mark_now() override = 0;
};
} // namespace kotlin::time
#endif
