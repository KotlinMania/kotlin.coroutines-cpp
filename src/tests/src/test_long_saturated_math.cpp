// NOTE(port): Boundary regression for the actual longSaturatedMath source
// algorithms. This is C++ test infrastructure, not a translated Kotlin test class.
#include "kotlin/time/LongSaturatedMath.hpp"
#include <cassert>
#include <initializer_list>
#include <limits>
#include <stdexcept>
#include <string>
using namespace kotlin::time;

int main() {
    constexpr auto minimum = std::numeric_limits<long long>::min();
    constexpr auto maximum = std::numeric_limits<long long>::max();
    for (auto value : {minimum, minimum + 1, -1LL, 0LL, 1LL, maximum - 1, maximum}) {
        assert(is_saturated(value) == (value == minimum || value == maximum));
        for (auto unit : {DurationUnit::NANOSECONDS, DurationUnit::MICROSECONDS,
                          DurationUnit::MILLISECONDS, DurationUnit::SECONDS, DurationUnit::DAYS})
            assert(saturating_origins_diff(value, value, unit).compare_to(Duration::ZERO) == 0);
    }
    assert(saturating_add(42, DurationUnit::NANOSECONDS, nanoseconds(-2)) == 40);
    assert(saturating_add(maximum - 5, DurationUnit::NANOSECONDS, nanoseconds(100)) == maximum);
    assert(saturating_add(minimum + 5, DurationUnit::NANOSECONDS, nanoseconds(-100)) == minimum);
    assert(saturating_add(maximum, DurationUnit::NANOSECONDS, nanoseconds(-1)) == maximum);
    assert(saturating_add(minimum, DurationUnit::NANOSECONDS, nanoseconds(1)) == minimum);
    assert(saturating_add(0, DurationUnit::NANOSECONDS, Duration::INFINITE) == maximum);
    assert(saturating_add(0, DurationUnit::NANOSECONDS, -Duration::INFINITE) == minimum);
    // Finite durations can exceed Long nanoseconds and still produce a finite
    // reading after a negative offset; the half-duration path must preserve it.
    assert(saturating_add(-1000000, DurationUnit::NANOSECONDS, milliseconds(9223372036855LL)) == 9223372036854000000LL);
    assert(saturating_add(1000000, DurationUnit::NANOSECONDS, milliseconds(-9223372036855LL)) == -9223372036854000000LL);
    for (auto value : {minimum, maximum}) {
        bool rejected = false;
        try { (void)saturating_add(value, DurationUnit::NANOSECONDS,
                                  value < 0 ? Duration::INFINITE : -Duration::INFINITE); }
        catch (const std::invalid_argument& error) {
            rejected = std::string(error.what()) == "Summing infinities of different signs";
        }
        assert(rejected);
    }
    assert(saturating_diff(0, maximum, DurationUnit::NANOSECONDS).compare_to(-Duration::INFINITE) == 0);
    assert(saturating_diff(0, minimum, DurationUnit::NANOSECONDS).compare_to(Duration::INFINITE) == 0);
    assert(saturating_diff(maximum, maximum, DurationUnit::NANOSECONDS).compare_to(-Duration::INFINITE) == 0);
    assert(saturating_origins_diff(maximum, minimum, DurationUnit::NANOSECONDS).compare_to(Duration::INFINITE) == 0);
    assert(saturating_origins_diff(minimum, maximum, DurationUnit::NANOSECONDS).compare_to(-Duration::INFINITE) == 0);
    assert(saturating_origins_diff(20, 30, DurationUnit::NANOSECONDS).compare_to(nanoseconds(-10)) == 0);
    // Mathematical difference is 18446744073709551612. Nanosecond/microsecond
    // readings retain a finite Duration even though the integer subtraction wraps.
    for (auto unit : {DurationUnit::NANOSECONDS, DurationUnit::MICROSECONDS}) {
        auto positive = saturating_origins_diff(maximum - 1, minimum + 1, unit);
        auto negative = saturating_origins_diff(minimum + 1, maximum - 1, unit);
        assert(positive.is_finite() && negative.is_finite());
        auto expected_ms = unit == DurationUnit::NANOSECONDS ? 18446744073709LL : 18446744073709551LL;
        assert(positive.in_whole_milliseconds() == expected_ms);
        assert(negative.in_whole_milliseconds() == -expected_ms);
        assert(positive.compare_to(-negative) == 0);
    }
    for (auto unit : {DurationUnit::MILLISECONDS, DurationUnit::SECONDS, DurationUnit::DAYS}) {
        assert(saturating_origins_diff(maximum - 1, minimum + 1, unit).compare_to(Duration::INFINITE) == 0);
        assert(saturating_origins_diff(minimum + 1, maximum - 1, unit).compare_to(-Duration::INFINITE) == 0);
    }
}
