#include <functional>
#include "kotlinx/coroutines/core_fwd.hpp"
// Transliterated from Kotlin to C++ (first-pass, syntax-only)
// Original: kotlinx-coroutines-core/common/src/flow/operators/Distinct.kt
//
// TODO: Implement coroutine semantics (suspend functions)
// TODO: Map Kotlin Flow types to C++ equivalents
// TODO: Implement StateFlow type
// TODO: Handle default lambda functions
// TODO: Implement NULL sentinel value

#pragma once

// @file:JvmMultifileClass
// @file:JvmName("FlowKt")

namespace kotlinx {
namespace coroutines {
namespace flow {

// TODO: import kotlinx.coroutines.flow.internal.*
// TODO: import kotlin.jvm.*

/**
 * Returns flow where all subsequent repetitions of the same value are filtered out.
 *
 * Note that any instance of [StateFlow] already behaves as if `distinctUntilChanged` operator is
 * applied to it, so applying `distinctUntilChanged` to a `StateFlow` has no effect.
 * See [StateFlow] documentation on Operator Fusion.
 * Also, repeated application of `distinctUntilChanged` operator on any flow has no effect.
 */
template<typename T>
Flow<T> distinct_until_changed(Flow<T> flow) {
    // return when (this)
    if (dynamic_cast<StateFlow<T>*>(&flow)) {
        return flow; // state flows are always distinct
    } else {
        return distinct_until_changed_by(flow, default_key_selector, default_are_equivalent);
    }
}

/**
 * Returns flow where all subsequent repetitions of the same value are filtered out, when compared
 * with each other via the provided [areEquivalent] function.
 *
 * Note that repeated application of `distinctUntilChanged` operator with the same parameter has no effect.
 */
// @Suppress("UNCHECKED_CAST")
template<typename T, typename Fn>
Flow<T> distinct_until_changed(Flow<T> flow, Fn are_equivalent) {
    return distinct_until_changed_by(flow, default_key_selector, are_equivalent);
}

/**
 * Returns flow where all subsequent repetitions of the same key are filtered out, where
 * key is extracted with [keySelector] function.
 *
 * Note that repeated application of `distinctUntilChanged` operator with the same parameter has no effect.
 */
template<typename T, typename K, typename Fn>
Flow<T> distinct_until_changed_by(Flow<T> flow, Fn key_selector) {
    return distinct_until_changed_by(flow, key_selector, default_are_equivalent);
}

// Default key selector
auto default_key_selector = [](auto it) { return it; };

// Default are equivalent
auto default_are_equivalent = [](auto old_val, auto new_val) { return old_val == new_val; };

/**
 * Returns flow where all subsequent repetitions of the same key are filtered out, where
 * keys are extracted with [keySelector] function and compared with each other via the
 * provided [areEquivalent] function.
 *
 * NOTE: It is non-inline to share a single implementing class.
 */
template<typename T, typename KeySelector, typename AreEquivalent>
Flow<T> distinct_until_changed_by(
    Flow<T> flow,
    KeySelector key_selector,
    AreEquivalent are_equivalent
) {
    // return when
    if (auto* distinct = dynamic_cast<DistinctFlowImpl<T>*>(&flow)) {
        if (distinct->key_selector == key_selector && distinct->are_equivalent == are_equivalent) {
            return flow; // same
        }
    }
    return DistinctFlowImpl<T>(flow, key_selector, are_equivalent);
}

template<typename T>
class DistinctFlowImpl : Flow<T> {
private:
    Flow<T> upstream;

public:
    // @JvmField
    std::function<void*(T)> key_selector;
    // @JvmField
    std::function<bool(void*, void*)> are_equivalent;

    DistinctFlowImpl(
        Flow<T> upstream_,
        std::function<void*(T)> key_selector_,
        std::function<bool(void*, void*)> are_equivalent_
    ) : upstream(upstream_), key_selector(key_selector_), are_equivalent(are_equivalent_) {}

    // TODO: suspend function
    void collect(FlowCollector<T> collector) /* override */ {
        void* previous_key = NULL;
        upstream.collect([&](T value) {
            void* key = key_selector(value);
            // @Suppress("UNCHECKED_CAST")
            if (previous_key == NULL || !are_equivalent(previous_key, key)) {
                previous_key = key;
                collector.emit(value);
            }
        });
    }
};

} // namespace flow
} // namespace coroutines
} // namespace kotlinx
