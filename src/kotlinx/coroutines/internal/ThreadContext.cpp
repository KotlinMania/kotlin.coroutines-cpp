/**
 * Transliterated from: kotlinx-coroutines-core/native/src/internal/ThreadContext.kt
 */
#include "kotlinx/coroutines/internal/ThreadContext.hpp"

namespace kotlinx::coroutines::internal {

// Transliterated from: kotlinx-coroutines-core/native/src/internal/ThreadContext.kt:5-5
void* thread_context_elements(const CoroutineContext&) {
    // NOTE(port): Native's constant zero is represented by the zero value of the erased ABI.
    return nullptr;
}

} // namespace kotlinx::coroutines::internal
