#pragma once
#include "kotlinx/coroutines/core_fwd.hpp"
#include "kotlinx/coroutines/internal/Symbol.hpp" // For NO_THREAD_ELEMENTS
#include "kotlinx/coroutines/CoroutineContext.hpp"

namespace kotlinx {
namespace coroutines {
namespace internal {

// Symbol for no thread elements
extern Symbol NO_THREAD_ELEMENTS;

/**
 * Updates the current thread context with elements from the given [context].
 * Returns the old state that should be passed to [restoreThreadContext].
 */
void* update_thread_context(const CoroutineContext& context, void* count_or_element);

/**
 * Restores the thread context to the [old_state].
 */
void restore_thread_context(const CoroutineContext& context, void* old_state);

/**
 * Counts the number of ThreadContextElements in the context.
 */
void* thread_context_elements(const CoroutineContext& context);

} // namespace internal
} // namespace coroutines
} // namespace kotlinx
