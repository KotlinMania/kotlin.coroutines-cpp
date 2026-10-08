/**
 * Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/NopCollector.kt
 */
#pragma once
// port-lint: source kotlinx-coroutines-core/common/src/flow/internal/NopCollector.kt
#include "kotlinx/coroutines/flow/FlowCollector.hpp"

namespace kotlinx {
namespace coroutines {
namespace flow {
namespace internal {

// Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/NopCollector.kt:5-9
// NOTE(port): FlowCollector<T> is invariant in C++; typed instances project the
// source contravariant FlowCollector<Any?> singleton without converting values.
template <typename T>
struct NopCollector final : public FlowCollector<T> {
    // Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/NopCollector.kt:6-8
    void* emit(T /*value*/, Continuation<void*>* /*continuation*/) override {
        // does nothing
        return nullptr;
    }
};

} // namespace internal
} // namespace flow
} // namespace coroutines
} // namespace kotlinx
