// Transliterated from Kotlin to C++ (first-pass, syntax-only)
// Original: kotlinx-coroutines-core/common/src/flow/terminal/Collection.kt
//
// TODO: Implement coroutine semantics (suspend functions)
// TODO: Map Kotlin Flow types to C++ equivalents
// TODO: Map Kotlin collections to C++ equivalents (List, Set, MutableCollection)

#pragma once

// @file:JvmMultifileClass
// @file:JvmName("FlowKt")

#include <vector>
#include <unordered_set>

namespace kotlinx {
namespace coroutines {
namespace flow {

// TODO: import kotlin.jvm.*

/**
 * Collects given flow into a [destination]
 */
template<typename T>
std::vector<T> to_list(Flow<T> flow) {
    std::vector<T> destination;
    return to_collection(flow, destination);
}

template<typename T>
std::vector<T> to_list(Flow<T> flow, std::vector<T> destination) {
    return to_collection(flow, destination);
}

/**
 * Collects given flow into a [destination]
 */
template<typename T>
std::unordered_set<T> to_set(Flow<T> flow) {
    std::unordered_set<T> destination; // LinkedHashSet equivalent
    return to_collection(flow, destination);
}

template<typename T>
std::unordered_set<T> to_set(Flow<T> flow, std::unordered_set<T> destination) {
    return to_collection(flow, destination);
}

/**
 * Collects given flow into a [destination]
 */
template<typename T, typename C>
C to_collection(Flow<T> flow, C& destination) {
    flow.collect([&](T value) {
        destination.insert(destination.end(), value); // or destination.add(value)
    });
    return destination;
}

} // namespace flow
} // namespace coroutines
} // namespace kotlinx
