#pragma once
/**
 * @file Yield.hpp
 * @brief Yield function declaration
 *
 * Transliterated from: kotlinx-coroutines-core/common/src/Yield.kt
 *
 * Provides the yield_coroutine() function (named yield_coroutine to avoid conflict
 * with C++ keyword 'yield' in C++20).
 */
#include <memory>
#include "kotlinx/coroutines/Continuation.hpp"
namespace kotlinx {
namespace coroutines {

/*
 * TODO: STUB - yield_coroutine() yields thread instead of suspending coroutine
 *
 * Kotlin source: yield() in Yield.kt
 *
 * What's missing:
 * - Should be a suspend function: suspend fun yield()
 * - Should check cancellation, then suspend and re-dispatch coroutine
 * - Allows other coroutines on same dispatcher to run
 * - See Yield.cpp for full TODO details
 *
 * Current behavior: Calls std::this_thread::yield() - OS level thread yield
 * Correct behavior: Suspend coroutine, re-enqueue to dispatcher for cooperative scheduling
 *
 * NOTE: Named yield_coroutine to avoid conflict with C++20's yield keyword.
 */
[[deprecated("Use yield(completion) instead")]]
void yield_coroutine();

/**
 * Suspend function form - requires continuation.
 * This is the proper suspend function implementation.
 */
void* yield(std::shared_ptr<Continuation<void*>> completion);

/**
 * No-arg yield for test compatibility.
 * In Kotlin, yield() is a suspend function called within coroutines.
 * In C++ tests not using full coroutine machinery, this provides
 * a simple OS-level thread yield as a stand-in.
 */
inline void yield() {
    // Suppress deprecation warning - this is the test compatibility wrapper
    #pragma clang diagnostic push
    #pragma clang diagnostic ignored "-Wdeprecated-declarations"
    yield_coroutine();
    #pragma clang diagnostic pop
}

} // namespace coroutines
} // namespace kotlinx
