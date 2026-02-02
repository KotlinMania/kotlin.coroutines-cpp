#pragma once
// port-lint: source flow/terminal/Count.kt
/**
 * @file Count.hpp
 * @brief Terminal flow operators for counting: count, count with predicate
 *
 * Transliterated from: kotlinx-coroutines-core/common/src/flow/terminal/Count.kt
 */

#include "kotlinx/coroutines/flow/Flow.hpp"
#include "kotlinx/coroutines/Continuation.hpp"
#include <functional>
#include <memory>

namespace kotlinx {
namespace coroutines {
namespace flow {

// ============================================================================
// Line 11-18: count() operator
// ============================================================================

/**
 * Returns the number of elements in this flow.
 *
 * Transliterated from:
 * public suspend fun <T> Flow<T>.count(): Int
 *
 * @param flow The flow to count
 * @return The number of elements in the flow
 */
template <typename T>
int count(const std::shared_ptr<Flow<T>>& flow) {
    int i = 0;

    class CountCollector : public FlowCollector<T> {
    public:
        explicit CountCollector(int& counter) : counter_(counter) {}

        void* emit(T /*value*/, Continuation<void*>* /*cont*/) override {
            ++counter_;
            return nullptr;
        }

    private:
        int& counter_;
    };

    CountCollector collector(i);
    flow->collect(&collector, nullptr);

    return i;
}

// ============================================================================
// Line 23-32: count(predicate) operator
// ============================================================================

/**
 * Returns the number of elements matching the given predicate.
 *
 * Transliterated from:
 * public suspend fun <T> Flow<T>.count(predicate: suspend (T) -> Boolean): Int
 *
 * @param flow The flow to count
 * @param predicate Function to filter elements
 * @return The number of elements matching the predicate
 */
template <typename T>
int count(const std::shared_ptr<Flow<T>>& flow, std::function<bool(T)> predicate) {
    int i = 0;

    class CountPredicateCollector : public FlowCollector<T> {
    public:
        CountPredicateCollector(int& counter, std::function<bool(T)> pred)
            : counter_(counter), predicate_(std::move(pred)) {}

        void* emit(T value, Continuation<void*>* /*cont*/) override {
            if (predicate_(value)) {
                ++counter_;
            }
            return nullptr;
        }

    private:
        int& counter_;
        std::function<bool(T)> predicate_;
    };

    CountPredicateCollector collector(i, predicate);
    flow->collect(&collector, nullptr);

    return i;
}

} // namespace flow
} // namespace coroutines
} // namespace kotlinx
