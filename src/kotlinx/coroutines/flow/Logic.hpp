#pragma once
// port-lint: source flow/terminal/Logic.kt
/**
 * @file Logic.hpp
 * @brief Terminal flow operators for logic: any, all, none
 *
 * Transliterated from: kotlinx-coroutines-core/common/src/flow/terminal/Logic.kt
 */

#include "kotlinx/coroutines/flow/Flow.hpp"
#include "kotlinx/coroutines/Continuation.hpp"
#include <functional>
#include <memory>

namespace kotlinx {
namespace coroutines {
namespace flow {

// ============================================================================
// Helper: collectWhile (used by any/all)
// ============================================================================

/**
 * Collects elements from the flow while the predicate returns true.
 * Stops collection when predicate returns false.
 *
 * @param flow The flow to collect from
 * @param predicate Function that returns true to continue, false to stop
 */
template <typename T>
void collect_while(const std::shared_ptr<Flow<T>>& flow, std::function<bool(T)> predicate) {
    class CollectWhileCollector : public FlowCollector<T> {
    public:
        explicit CollectWhileCollector(std::function<bool(T)> pred)
            : predicate_(std::move(pred)), should_continue_(true) {}

        void* emit(T value, Continuation<void*>* /*cont*/) override {
            if (should_continue_) {
                should_continue_ = predicate_(value);
            }
            return nullptr;
        }

    private:
        std::function<bool(T)> predicate_;
        bool should_continue_;
    };

    CollectWhileCollector collector(std::move(predicate));
    flow->collect(&collector, nullptr);
}

// ============================================================================
// Line 34-42: any operator
// ============================================================================

/**
 * A terminal operator that returns `true` and immediately cancels the flow
 * if at least one element matches the given predicate.
 *
 * If the flow does not emit any elements or no element matches the predicate, the function returns `false`.
 *
 * Equivalent to `!all { !predicate(it) }` (see Flow.all) and `!none { predicate(it) }` (see Flow.none).
 *
 * Transliterated from:
 * public suspend fun <T> Flow<T>.any(predicate: suspend (T) -> Boolean): Boolean
 *
 * @param flow The flow to check
 * @param predicate Function to test each element
 * @return true if any element matches the predicate, false otherwise
 */
template <typename T>
bool any(const std::shared_ptr<Flow<T>>& flow, std::function<bool(T)> predicate) {
    bool found = false;

    collect_while(flow, [&found, &predicate](T value) -> bool {
        bool satisfies = predicate(value);
        if (satisfies) {
            found = true;
        }
        return !satisfies;  // Continue while predicate is NOT satisfied
    });

    return found;
}

// ============================================================================
// Line 71-79: all operator
// ============================================================================

/**
 * A terminal operator that returns `true` if all elements match the given predicate,
 * or returns `false` and cancels the flow as soon as the first element not matching the predicate is encountered.
 *
 * If the flow terminates without emitting any elements, the function returns `true` because there
 * are no elements in it that *do not* match the predicate.
 *
 * Equivalent to `!any { !predicate(it) }` (see Flow.any) and `none { !predicate(it) }` (see Flow.none).
 *
 * Transliterated from:
 * public suspend fun <T> Flow<T>.all(predicate: suspend (T) -> Boolean): Boolean
 *
 * @param flow The flow to check
 * @param predicate Function to test each element
 * @return true if all elements match the predicate, false otherwise
 */
template <typename T>
bool all(const std::shared_ptr<Flow<T>>& flow, std::function<bool(T)> predicate) {
    bool found_counter_example = false;

    collect_while(flow, [&found_counter_example, &predicate](T value) -> bool {
        bool satisfies = predicate(value);
        if (!satisfies) {
            found_counter_example = true;
        }
        return satisfies;  // Continue while predicate IS satisfied
    });

    return !found_counter_example;
}

// ============================================================================
// Line 107: none operator
// ============================================================================

/**
 * A terminal operator that returns `true` if no elements match the given predicate,
 * or returns `false` and cancels the flow as soon as the first element matching the predicate is encountered.
 *
 * If the flow terminates without emitting any elements, the function returns `true` because there
 * are no elements in it that match the predicate.
 *
 * Equivalent to `!any(predicate)` (see Flow.any) and `all { !predicate(it) }` (see Flow.all).
 *
 * Transliterated from:
 * public suspend fun <T> Flow<T>.none(predicate: suspend (T) -> Boolean): Boolean = !any(predicate)
 *
 * @param flow The flow to check
 * @param predicate Function to test each element
 * @return true if no elements match the predicate, false otherwise
 */
template <typename T>
bool none(const std::shared_ptr<Flow<T>>& flow, std::function<bool(T)> predicate) {
    return !any(flow, predicate);
}

} // namespace flow
} // namespace coroutines
} // namespace kotlinx
