#include "kotlinx/coroutines/core_fwd.hpp"
// Transliterated from Kotlin to C++ (first-pass, syntax-only)
// Original: kotlinx-coroutines-core/common/src/flow/terminal/Count.kt
//
// TODO: Implement coroutine semantics (suspend functions)
// TODO: Map Kotlin Flow types to C++ equivalents

#pragma once

// @file:JvmMultifileClass
// @file:JvmName("FlowKt")

namespace kotlinx {
namespace coroutines {
namespace flow {

// TODO: import kotlin.jvm.*

/**
 * Returns the number of elements in this flow.
 */
template<typename T>
int count(Flow<T> flow) {
    int i = 0;
    flow.collect([&](T) {
        ++i;
    });

    return i;
}

/**
 * Returns the number of elements matching the given predicate.
 */
template<typename T, typename Predicate>
int count(Flow<T> flow, Predicate predicate) {
    int i = 0;
    flow.collect([&](T value) {
        if (predicate(value)) {
            ++i;
        }
    });

    return i;
}

} // namespace flow
} // namespace coroutines
} // namespace kotlinx
