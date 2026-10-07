#pragma once
// port-lint: source kotlinx-coroutines-core/common/src/flow/operators/Emitters.kt
/**
 * Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Emitters.kt:193-205
 */

#include "kotlinx/coroutines/flow/FlowCollector.hpp"
#include <exception>
#include <utility>

namespace kotlinx::coroutines::flow {

// NOTE(port): Kotlin's contravariant FlowCollector<Any?> accepts every element
// type. C++ requires the matching FlowCollector<T> specialization; this internal
// template stays in a header for each caller's element type.
// Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Emitters.kt:201-205
template <typename T>
class ThrowingCollector final : public FlowCollector<T> {
public:
    // Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Emitters.kt:201
    explicit ThrowingCollector(std::exception_ptr e) : e_(std::move(e)) {}

    // Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Emitters.kt:202-204
    void* emit(T, Continuation<void*>*) override {
        std::rethrow_exception(e_);
    }

    // Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Emitters.kt:201
    std::exception_ptr exception() const { return e_; }

private:
    std::exception_ptr e_;
};

/*
 * emit_all methods call this to fail immediately before starting to collect
 * their sources (that may not have any elements for a long time).
 */
// Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Emitters.kt:193-199
template <typename T>
inline void ensure_active(FlowCollector<T>* collector) {
    if (auto* throwing = dynamic_cast<ThrowingCollector<T>*>(collector)) {
        std::rethrow_exception(throwing->exception());
    }
}

} // namespace kotlinx::coroutines::flow
