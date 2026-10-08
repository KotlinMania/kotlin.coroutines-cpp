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
    class Monotonic;
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
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/time/MonotonicTimeSource.kt:31
using ValueTimeMarkReading = long long;
// Transliterated from: libraries/stdlib/src/kotlin/time/TimeSource.kt:54-115
// NOTE(port): Define the mark before its source so C++ can validate covariant
// returns; the source's nested name below aliases this same actual type.
class ValueTimeMark final : public ComparableTimeMark {
public:
    // Transliterated from: libraries/stdlib/src/kotlin/time/TimeSource.kt:70
    explicit ValueTimeMark(ValueTimeMarkReading reading);
    // Transliterated from: libraries/stdlib/src/kotlin/time/TimeSource.kt:82-86
    Duration elapsed_now() const override;
    ValueTimeMark* plus(Duration duration) override;
    ValueTimeMark* minus(Duration duration) override;
    bool has_passed_now() const override;
    bool has_not_passed_now() const override;
    // Transliterated from: libraries/stdlib/src/kotlin/time/TimeSource.kt:88-92
    Duration minus(const ComparableTimeMark& other) const override;
    // Transliterated from: libraries/stdlib/src/kotlin/time/TimeSource.kt:94-105
    Duration minus(const ValueTimeMark& other) const;
    // Transliterated from: libraries/stdlib/src/kotlin/time/TimeSource.kt:107-114
    int compare_to(const ValueTimeMark& other) const;
    using ComparableTimeMark::compare_to;
    // Transliterated from: compiler/ir/ir.tree/src/org/jetbrains/kotlin/ir/util/DataClassMembersGenerator.kt:136-176
    bool equals(const kotlin::Any* other) const override;
    // Transliterated from: compiler/ir/ir.tree/src/org/jetbrains/kotlin/ir/util/DataClassMembersGenerator.kt:178-185
    std::int32_t hash_code() const override;
    // Transliterated from: compiler/ir/ir.tree/src/org/jetbrains/kotlin/ir/util/DataClassMembersGenerator.kt:237-265
    std::string to_string() const override;
    // Transliterated from: libraries/stdlib/src/kotlin/time/TimeSource.kt:70
    ValueTimeMarkReading reading() const;
private:
    const ValueTimeMarkReading reading_;
};
// Transliterated from: libraries/stdlib/src/kotlin/time/TimeSource.kt:43-117
class TimeSource::Monotonic final : public TimeSource::WithComparableMarks {
public:
    using ValueTimeMark = kotlin::time::ValueTimeMark;
    // NOTE(port): A source object is one function-local C++ singleton.
    static Monotonic& instance();
    // Transliterated from: libraries/stdlib/src/kotlin/time/TimeSource.kt:51
    ValueTimeMark* mark_now() override;
    // Transliterated from: libraries/stdlib/src/kotlin/time/TimeSource.kt:52
    std::string to_string() const;
private:
    // NOTE(port): Construction is private because the source is a singleton object.
    Monotonic() = default;
};
} // namespace kotlin::time
#endif
