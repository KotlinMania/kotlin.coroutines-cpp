/**
 * Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/FlowExceptions.common.kt
 */
// port-lint: source kotlinx-coroutines-core/common/src/flow/internal/FlowExceptions.common.kt
#include "kotlinx/coroutines/flow/internal/FlowExceptions.hpp"
#include <exception>
#include <stdexcept>

namespace kotlinx::coroutines::flow::internal {
// Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/FlowExceptions.common.kt:17-19
void check_ownership(AbortFlowException* receiver, void* owner) {
    if (receiver->owner != owner) {
        // NOTE(port): Kotlin throws this same Throwable. Reuse the active C++
        // exception object when this is it; first throws of a value need a C++ box.
        auto active = std::current_exception();
        if (active) {
            bool is_this = false;
            try { std::rethrow_exception(active); }
            catch (const AbortFlowException& exception) { is_this = &exception == receiver; }
            catch (...) {}
            if (is_this) std::rethrow_exception(active);
        }
        throw *receiver;
    }
}

// Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/FlowExceptions.common.kt:17-19
// NOTE(port): Compatibility member binds the source extension receiver.
void AbortFlowException::check_ownership(void* owner) { internal::check_ownership(this, owner); }

// Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/FlowExceptions.common.kt:28-33
int check_index_overflow(int index) {
    if (index < 0) {
        // NOTE(port): Arithmetic overflow currently uses the existing std::overflow_error boundary.
        throw std::overflow_error("Index overflow has happened");
    }
    return index;
}
} // namespace kotlinx::coroutines::flow::internal
