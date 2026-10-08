#pragma once
/**
 * Transliterated from: kotlinx-coroutines-core/common/src/internal/ThreadContext.common.kt
 * and kotlinx-coroutines-core/native/src/internal/ThreadContext.kt
 */
#include "kotlinx/coroutines/CoroutineContext.hpp"

namespace kotlinx::coroutines::internal {

// Transliterated from: kotlinx-coroutines-core/common/src/internal/ThreadContext.common.kt:5-5
// and kotlinx-coroutines-core/native/src/internal/ThreadContext.kt:5-5
void* thread_context_elements(const CoroutineContext& context);

} // namespace kotlinx::coroutines::internal
