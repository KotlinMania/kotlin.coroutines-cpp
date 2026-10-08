/**
 * Transliterated from: kotlinx-coroutines-core/common/src/CancellableContinuationImpl.kt:624-630
 */
#pragma once
/**
 * @file ContinuationState.hpp
 * @brief State hierarchy for CancellableContinuationImpl.
 *
 * Extracted from CancellableContinuationImpl.hpp to allow SegmentBase
 * and other types to inherit from NotCompleted without circular includes.
 */

#include <string>

namespace kotlinx {
namespace coroutines {

// ------------------------------------------------------------------
// State Hierarchy (Faithful to Kotlin "Any" state logic)
// ------------------------------------------------------------------

/**
 * Base class for all states in CancellableContinuationImpl state machine.
 * Corresponds to `Any?` in `_state = atomic<Any?>(Active)`.
 */
// NOTE(port): Type-erased storage for Kotlin AtomicRef<Any?>, with C++ destruction.
struct State {
    virtual ~State() = default;
    virtual std::string to_string() const = 0;
};

// Internal interface NotCompleted
// Transliterated from: kotlinx-coroutines-core/common/src/CancellableContinuationImpl.kt:626-626
struct NotCompleted : public virtual State {};

// Transliterated from: kotlinx-coroutines-core/common/src/CancellableContinuationImpl.kt:628-630
struct Active : public NotCompleted {
    static Active instance;
    // Transliterated from: kotlinx-coroutines-core/common/src/CancellableContinuationImpl.kt:629-629
    std::string to_string() const override { return "Active"; }
};
inline Active Active::instance;

} // namespace coroutines
} // namespace kotlinx
