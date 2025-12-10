/**
 * @file Delay.cpp
 * @brief Implementation of delay functions
 *
 * Transliterated from: kotlinx-coroutines-core/common/src/Delay.kt
 *
 * Provides delay functionality for coroutines.
 *
 * NOTE: In a full C++20 coroutine implementation, delay would be a suspend function.
 * This implementation provides a simplified blocking version.
 */

#include "kotlinx/coroutines/Delay.hpp"
#include "kotlinx/coroutines/Exceptions.hpp"
#include <thread>
#include <chrono>

namespace kotlinx {
namespace coroutines {

/**
 * Delays the current thread for at least the given time.
 *
 * In true Kotlin coroutines, this is a suspending function that doesn't block a thread.
 * In this C++ transliteration, this is implemented as a thread sleep.
 *
 * TODO: Implement proper coroutine suspension when C++20 coroutines are used.
 *
 * @param time_millis time in milliseconds. If non-positive, returns immediately.
 */
void delay(long time_millis) {
    if (time_millis <= 0) return;
    std::this_thread::sleep_for(std::chrono::milliseconds(time_millis));
}

/**
 * Delays the current thread for at least the given duration.
 *
 * @param duration the duration to delay
 */
void delay(std::chrono::nanoseconds duration) {
    if (duration.count() <= 0) return;
    std::this_thread::sleep_for(duration);
}

/**
 * Overload for milliseconds duration.
 */
void delay(std::chrono::milliseconds duration) {
    if (duration.count() <= 0) return;
    std::this_thread::sleep_for(duration);
}

/**
 * Suspends until cancellation.
 *
 * In true Kotlin coroutines, this suspends indefinitely until cancelled.
 * In this C++ transliteration, this blocks indefinitely (until the program terminates).
 *
 * TODO: Implement proper cancellation support.
 */
[[noreturn]] void await_cancellation() {
    // Block indefinitely - in a real implementation this would wait for cancellation
    while (true) {
        std::this_thread::sleep_for(std::chrono::hours(24));
    }
}

} // namespace coroutines
} // namespace kotlinx
