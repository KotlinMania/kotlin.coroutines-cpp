// port-lint: source kotlinx-coroutines-core/common/test/DurationToMillisTest.kt
// Transliterated from: kotlinx-coroutines-core/common/test/DurationToMillisTest.kt
#include "kotlinx/coroutines/Delay.hpp"
#include <iostream>
#include <limits>
#include <stdexcept>

namespace kotlinx::coroutines {
using namespace kotlin::time;
constexpr long long MAX_LONG = std::numeric_limits<long long>::max();
// NOTE(port): Test assertions remain active in Release builds.
void assert_equals(long long expected, long long actual) {
    if (expected != actual) throw std::runtime_error("Expected " + std::to_string(expected) + ", got " + std::to_string(actual));
}
// Transliterated from: kotlinx-coroutines-core/common/test/DurationToMillisTest.kt:9-65
class DurationToMillisTest {
public:
    // Transliterated from: kotlinx-coroutines-core/common/test/DurationToMillisTest.kt:12-14
    void test_negative_duration_coerced_to_zero_millis() {
        assert_equals(0, to_delay_millis(seconds(-1)));
    }
    // Transliterated from: kotlinx-coroutines-core/common/test/DurationToMillisTest.kt:17-19
    void test_zero_duration_coerced_to_zero_millis() {
        assert_equals(0, to_delay_millis(seconds(0)));
    }
    // Transliterated from: kotlinx-coroutines-core/common/test/DurationToMillisTest.kt:22-24
    void test_one_nanosecond_coerced_to_one_millisecond() {
        assert_equals(1, to_delay_millis(nanoseconds(1)));
    }
    // Transliterated from: kotlinx-coroutines-core/common/test/DurationToMillisTest.kt:27-29
    void test_one_second_coerced_to1000_milliseconds() {
        assert_equals(1000, to_delay_millis(seconds(1)));
    }
    // Transliterated from: kotlinx-coroutines-core/common/test/DurationToMillisTest.kt:32-34
    void test_mixed_component_duration_rounded_up_to_next_millisecond() {
        assert_equals(999, to_delay_millis(milliseconds(998) + nanoseconds(75909)));
    }
    // Transliterated from: kotlinx-coroutines-core/common/test/DurationToMillisTest.kt:37-39
    void test_one_extra_nanosecond_rounded_up_to_next_millisecond() {
        assert_equals(999, to_delay_millis(milliseconds(998) + nanoseconds(1)));
    }
    // Transliterated from: kotlinx-coroutines-core/common/test/DurationToMillisTest.kt:42-44
    void test_infinite_duration_coerced_to_long_max_value() {
        assert_equals(MAX_LONG, to_delay_millis(Duration::INFINITE));
    }
    // Transliterated from: kotlinx-coroutines-core/common/test/DurationToMillisTest.kt:47-49
    void test_negative_infinite_duration_coerced_to_zero() {
        assert_equals(0, to_delay_millis(-Duration::INFINITE));
    }
    // Transliterated from: kotlinx-coroutines-core/common/test/DurationToMillisTest.kt:52-54
    void test_nanosecond_off_by_one_infinity_does_not_overflow() {
        assert_equals(MAX_LONG / 1000000, to_delay_millis(nanoseconds(MAX_LONG - 1)));
    }
    // Transliterated from: kotlinx-coroutines-core/common/test/DurationToMillisTest.kt:57-59
    void test_millisecond_off_by_one_infinity_does_not_increment() {
        assert_equals(MAX_LONG / 2 - 1, to_delay_millis(milliseconds(MAX_LONG / 2 - 1)));
    }
    // Transliterated from: kotlinx-coroutines-core/common/test/DurationToMillisTest.kt:62-64
    void test_out_of_bounds_nanoseconds_but_finite_does_not_increment() {
        assert_equals(MAX_LONG / 10, to_delay_millis(milliseconds(MAX_LONG / 10)));
    }
};

// NOTE(port): Plain C++ runner replaces the Kotlin test annotations.
void run_duration_to_millis_tests() {
    DurationToMillisTest test;
    test.test_negative_duration_coerced_to_zero_millis();
    test.test_zero_duration_coerced_to_zero_millis();
    test.test_one_nanosecond_coerced_to_one_millisecond();
    test.test_one_second_coerced_to1000_milliseconds();
    test.test_mixed_component_duration_rounded_up_to_next_millisecond();
    test.test_one_extra_nanosecond_rounded_up_to_next_millisecond();
    test.test_infinite_duration_coerced_to_long_max_value();
    test.test_negative_infinite_duration_coerced_to_zero();
    test.test_nanosecond_off_by_one_infinity_does_not_overflow();
    test.test_millisecond_off_by_one_infinity_does_not_increment();
    test.test_out_of_bounds_nanoseconds_but_finite_does_not_increment();
    std::cout << "Executed 11 upstream duration conversion cases\n";
}
} // namespace kotlinx::coroutines
