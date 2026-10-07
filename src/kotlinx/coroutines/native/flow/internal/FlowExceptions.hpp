/**
 * Transliterated from: kotlinx-coroutines-core/native/src/flow/internal/FlowExceptions.kt
 */
#pragma once
// port-lint: source kotlinx-coroutines-core/native/src/flow/internal/FlowExceptions.kt
#include "kotlinx/coroutines/Exceptions.hpp"

namespace kotlinx::coroutines::flow::internal {

// This exception is thrown when an operator needs no more elements from the flow.
// It must not escape its owner's boundary. Each invocation needs a unique owner.
// Transliterated from: kotlinx-coroutines-core/native/src/flow/internal/FlowExceptions.kt:6-8
class AbortFlowException : public CancellationException {
public:
    void* const owner;
    // Transliterated from: kotlinx-coroutines-core/native/src/flow/internal/FlowExceptions.kt:6-8
    explicit AbortFlowException(void* owner);
    // Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/FlowExceptions.common.kt:17-19
    void check_ownership(void* other);
};

// Exception used to cancel a scopedFlow child without cancelling the whole scope.
// Transliterated from: kotlinx-coroutines-core/native/src/flow/internal/FlowExceptions.kt:9-9
class ChildCancelledException : public CancellationException {
public:
    // Transliterated from: kotlinx-coroutines-core/native/src/flow/internal/FlowExceptions.kt:9-9
    ChildCancelledException();
};

} // namespace kotlinx::coroutines::flow::internal
