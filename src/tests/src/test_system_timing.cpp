// NOTE(port): Standalone C++ contract regression for the translated timing API.
#include "kotlin/system/Timing.hpp"
#include <cassert>
#include <exception>
#include <memory>
#include <stdexcept>
using namespace kotlin::system;

int main() {
    // Ordered readings bracket the same source reading in different units.
    for (int i = 0; i < 100; ++i) {
        auto before_ms = get_time_millis();
        auto nanos = get_time_nanos();
        auto after_ms = get_time_millis();
        assert(before_ms <= nanos / 1000000 && nanos / 1000000 <= after_ms);
        auto before_us = get_time_micros();
        nanos = get_time_nanos();
        auto after_us = get_time_micros();
        assert(before_us <= nanos / 1000 && nanos / 1000 <= after_us);
    }
    int calls = 0;
    assert(measure_time_millis([&] { ++calls; }) >= 0);
    assert(measure_time_micros([&] { ++calls; }) >= 0);
    assert(measure_nano_time([owned = std::make_unique<int>(3), &calls] {
        calls += *owned;
    }) >= 0);
    assert(calls == 5);

    auto failure = std::make_exception_ptr(std::runtime_error("original timing failure"));
    bool same_failure = false;
    try { (void)measure_nano_time([&] { std::rethrow_exception(failure); }); }
    catch (...) { same_failure = std::current_exception() == failure; }
    assert(same_failure);
}
