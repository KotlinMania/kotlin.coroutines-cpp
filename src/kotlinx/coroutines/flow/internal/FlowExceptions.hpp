/**
 * Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/FlowExceptions.common.kt
 */
#pragma once
// port-lint: source kotlinx-coroutines-core/common/src/flow/internal/FlowExceptions.common.kt

namespace kotlinx::coroutines::flow::internal {
// Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/FlowExceptions.common.kt:13-15
class AbortFlowException;
// Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/FlowExceptions.common.kt:24-24
class ChildCancelledException;
// Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/FlowExceptions.common.kt:17-19
void check_ownership(AbortFlowException* receiver, void* owner);
// Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/FlowExceptions.common.kt:28-33
int check_index_overflow(int index);
} // namespace kotlinx::coroutines::flow::internal

// NOTE(port): The common expect classes are supplied by the matching Native actual.
#include "kotlinx/coroutines/native/flow/internal/FlowExceptions.hpp"
