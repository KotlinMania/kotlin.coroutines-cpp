/**
 * Transliterated from: kotlinx-coroutines-core/native/src/flow/internal/FlowExceptions.kt
 */
// port-lint: source kotlinx-coroutines-core/native/src/flow/internal/FlowExceptions.kt
#include "kotlinx/coroutines/native/flow/internal/FlowExceptions.hpp"
#include <exception>
#include <stdexcept>

namespace kotlinx::coroutines::flow::internal {

// Transliterated from: kotlinx-coroutines-core/native/src/flow/internal/FlowExceptions.kt:6-8
AbortFlowException::AbortFlowException(void* owner)
    : CancellationException("Flow was aborted, no more elements needed"), owner(owner) {}

// Transliterated from: kotlinx-coroutines-core/native/src/flow/internal/FlowExceptions.kt:9-9
ChildCancelledException::ChildCancelledException()
    : CancellationException("Child of the scoped flow was cancelled") {}

} // namespace kotlinx::coroutines::flow::internal
