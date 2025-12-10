/**
 * @file Yield.cpp
 * @brief Implementation of yield function
 *
 * Transliterated from: kotlinx-coroutines-core/common/src/Yield.kt
 *
 * See the header for full documentation.
 */

#include "kotlinx/coroutines/Yield.hpp"
#include <thread>

namespace kotlinx {
namespace coroutines {

void yield_coroutine() {
    // In a true coroutine implementation, this would:
    // 1. Check if there's a dispatcher in the context
    // 2. Schedule the current coroutine for later execution
    // 3. Suspend to allow other coroutines to run
    //
    // In this simplified C++ transliteration, we just yield the current thread's
    // time slice to give other threads a chance to run.
    std::this_thread::yield();
}

} // namespace coroutines
} // namespace kotlinx
