/**
 * Transliterated from: libraries/stdlib/src/kotlin/time/TimeSource.kt
 */
#ifndef KOTLIN_TIME_COMPARABLE_TIME_MARK_HPP_
#define KOTLIN_TIME_COMPARABLE_TIME_MARK_HPP_
#include "kotlin/Comparable.hpp"
#include "kotlin/time/TimeMark.hpp"
#include <cstdint>

namespace kotlin { class Any; }
namespace kotlin::time {
// Transliterated from: libraries/stdlib/src/kotlin/time/TimeSource.kt:197-243
// Comparable marks must originate from the same time source.
class ComparableTimeMark : public TimeMark, public kotlin::Comparable<ComparableTimeMark> {
public:
    // Transliterated from: libraries/stdlib/src/kotlin/time/TimeSource.kt:203
    // The caller owns the returned mark, as with TimeMark::plus.
    ComparableTimeMark* plus(Duration duration) override = 0;
    // Transliterated from: libraries/stdlib/src/kotlin/time/TimeSource.kt:204
    ComparableTimeMark* minus(Duration duration) override;
    // Transliterated from: libraries/stdlib/src/kotlin/time/TimeSource.kt:206-221
    // Returns the duration from other to this mark; different sources must throw.
    virtual Duration minus(const ComparableTimeMark& other) const = 0;
    // Transliterated from: libraries/stdlib/src/kotlin/time/TimeSource.kt:223-235
    int compare_to(const ComparableTimeMark& other) const override;
    // Transliterated from: libraries/stdlib/src/kotlin/time/TimeSource.kt:237-241
    // NOTE(port): Nullable Any stays the actual Kotlin object boundary. Ordinary
    // C++ time marks are not made compiler-owned Any objects by this declaration.
    virtual bool equals(const kotlin::Any* other) const = 0;
    // Transliterated from: libraries/stdlib/src/kotlin/time/TimeSource.kt:242
    virtual std::int32_t hash_code() const = 0;
};
} // namespace kotlin::time
#endif
