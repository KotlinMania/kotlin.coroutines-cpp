/**
 * Transliterated from: libraries/stdlib/src/kotlin/time/TimeSource.kt
 */
#ifndef KOTLIN_TIME_TIME_MARK_HPP_
#define KOTLIN_TIME_TIME_MARK_HPP_
#include "kotlin/time/Duration.hpp"
#include <memory>

namespace kotlin::time {
// Transliterated from: libraries/stdlib/src/kotlin/time/TimeSource.kt:128-195
// Represents a time point bound to the time source it was taken from.
// NOTE(port): Shared ownership, when present, retains the original mark in an
// adjusted mark. Raw/stack receivers remain borrowed and must outlive adjustments.
class TimeMark : public std::enable_shared_from_this<TimeMark> {
public:
    virtual ~TimeMark();
    // Transliterated from: libraries/stdlib/src/kotlin/time/TimeSource.kt:135-151
    // Returns the amount of time passed from this mark; it may be negative or infinite.
    virtual Duration elapsed_now() const = 0;
    // Transliterated from: libraries/stdlib/src/kotlin/time/TimeSource.kt:152-164
    // Returns a mark ahead by duration (behind when duration is negative).
    // NOTE(port): The caller owns the returned mark and must delete it or place
    // it in an owning smart pointer. Pointer returns permit covariant overrides.
    virtual TimeMark* plus(Duration duration);
    // Transliterated from: libraries/stdlib/src/kotlin/time/TimeSource.kt:166-177
    virtual TimeMark* minus(Duration duration);
    // Transliterated from: libraries/stdlib/src/kotlin/time/TimeSource.kt:180-186
    virtual bool has_passed_now() const;
    // Transliterated from: libraries/stdlib/src/kotlin/time/TimeSource.kt:188-194
    virtual bool has_not_passed_now() const;
};
} // namespace kotlin::time
#endif
