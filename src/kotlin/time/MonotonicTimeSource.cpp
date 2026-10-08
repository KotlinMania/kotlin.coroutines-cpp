/**
 * Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/time/MonotonicTimeSource.kt
 * Transliterated from: libraries/stdlib/src/kotlin/time/TimeSource.kt
 */
#include "kotlin/time/TimeSource.hpp"
#include "kotlin/time/LongSaturatedMath.hpp"
#include "kotlin/system/Timing.hpp"
#include <bit>
#include <stdexcept>

namespace kotlin::time {
namespace {
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/time/MonotonicTimeSource.kt:12-28
// NOTE(port): The source internal object has concrete private C++ storage.
class MonotonicTimeSource final {
public:
    static MonotonicTimeSource& instance() {
        static MonotonicTimeSource source;
        return source;
    }
    // Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/time/MonotonicTimeSource.kt:19
    ValueTimeMark* mark_now() { return new ValueTimeMark(read()); }
    // Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/time/MonotonicTimeSource.kt:20-21
    Duration elapsed_from(const ValueTimeMark& mark) {
        return saturating_diff(read(), mark.reading(), DurationUnit::NANOSECONDS);
    }
    // Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/time/MonotonicTimeSource.kt:23-24
    Duration difference_between(const ValueTimeMark& one, const ValueTimeMark& another) {
        return saturating_origins_diff(one.reading(), another.reading(), DurationUnit::NANOSECONDS);
    }
    // Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/time/MonotonicTimeSource.kt:26-27
    ValueTimeMark* adjust_reading(const ValueTimeMark& mark, Duration duration) {
        return new ValueTimeMark(saturating_add(mark.reading(), DurationUnit::NANOSECONDS, duration));
    }
    // Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/time/MonotonicTimeSource.kt:17
    std::string to_string() const { return "TimeSource(System.nanoTime())"; }
private:
    // Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/time/MonotonicTimeSource.kt:14
    MonotonicTimeSource() : zero_(kotlin::system::get_time_nanos()) {}
    // Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/time/MonotonicTimeSource.kt:16
    long long read() const {
        // NOTE(port): Preserve Kotlin's wrapping subtraction without signed UB.
        return std::bit_cast<long long>(static_cast<unsigned long long>(kotlin::system::get_time_nanos()) -
                                       static_cast<unsigned long long>(zero_));
    }
    const long long zero_;
};
} // namespace

// NOTE(port): Public source object access keeps one C++ instance.
TimeSource::Monotonic& TimeSource::Monotonic::instance() {
    static Monotonic source;
    return source;
}
// Transliterated from: libraries/stdlib/src/kotlin/time/TimeSource.kt:51
ValueTimeMark* TimeSource::Monotonic::mark_now() { return MonotonicTimeSource::instance().mark_now(); }
// Transliterated from: libraries/stdlib/src/kotlin/time/TimeSource.kt:52
std::string TimeSource::Monotonic::to_string() const { return MonotonicTimeSource::instance().to_string(); }
// Transliterated from: libraries/stdlib/src/kotlin/time/TimeSource.kt:70
ValueTimeMark::ValueTimeMark(ValueTimeMarkReading reading) : reading_(reading) {}
// Transliterated from: libraries/stdlib/src/kotlin/time/TimeSource.kt:70
ValueTimeMarkReading ValueTimeMark::reading() const { return reading_; }
// Transliterated from: libraries/stdlib/src/kotlin/time/TimeSource.kt:82
Duration ValueTimeMark::elapsed_now() const { return MonotonicTimeSource::instance().elapsed_from(*this); }
// Transliterated from: libraries/stdlib/src/kotlin/time/TimeSource.kt:83
ValueTimeMark* ValueTimeMark::plus(Duration duration) {
    return MonotonicTimeSource::instance().adjust_reading(*this, duration);
}
// Transliterated from: libraries/stdlib/src/kotlin/time/TimeSource.kt:84
ValueTimeMark* ValueTimeMark::minus(Duration duration) {
    return MonotonicTimeSource::instance().adjust_reading(*this, -duration);
}
// Transliterated from: libraries/stdlib/src/kotlin/time/TimeSource.kt:85
bool ValueTimeMark::has_passed_now() const { return !elapsed_now().is_negative(); }
// Transliterated from: libraries/stdlib/src/kotlin/time/TimeSource.kt:86
bool ValueTimeMark::has_not_passed_now() const { return elapsed_now().is_negative(); }
// Transliterated from: libraries/stdlib/src/kotlin/time/TimeSource.kt:88-92
Duration ValueTimeMark::minus(const ComparableTimeMark& other) const {
    auto* mark = dynamic_cast<const ValueTimeMark*>(&other);
    if (!mark) throw std::invalid_argument(
        "Subtracting or comparing time marks from different time sources is not possible: " +
        to_string() + " and " + other.to_string());
    return minus(*mark);
}
// Transliterated from: libraries/stdlib/src/kotlin/time/TimeSource.kt:105
Duration ValueTimeMark::minus(const ValueTimeMark& other) const {
    return MonotonicTimeSource::instance().difference_between(*this, other);
}
// Transliterated from: libraries/stdlib/src/kotlin/time/TimeSource.kt:113-114
int ValueTimeMark::compare_to(const ValueTimeMark& other) const { return minus(other).compare_to(Duration::ZERO); }
// Transliterated from: compiler/ir/ir.tree/src/org/jetbrains/kotlin/ir/util/DataClassMembersGenerator.kt:178-185
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/Primitives.kt:1851-1852
std::int32_t ValueTimeMark::hash_code() const {
    auto bits = static_cast<std::uint64_t>(reading_);
    return std::bit_cast<std::int32_t>(static_cast<std::uint32_t>((bits >> 32) ^ bits));
}
// Transliterated from: compiler/ir/ir.tree/src/org/jetbrains/kotlin/ir/util/DataClassMembersGenerator.kt:237-265
// NOTE(port): Decimal Long text uses the actual C++ integer conversion; no
// substitute object identity or runtime text is supplied.
std::string ValueTimeMark::to_string() const { return "ValueTimeMark(reading=" + std::to_string(reading_) + ")"; }
} // namespace kotlin::time
