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
struct State {
    virtual ~State() = default;
    virtual std::string to_string() const = 0;
};

// Internal interface NotCompleted
struct NotCompleted : public virtual State {};

struct Active : public NotCompleted {
    static Active instance;
    std::string to_string() const override { return "Active"; }
};
inline Active Active::instance;

} // namespace coroutines
} // namespace kotlinx
