// Transliterated from Kotlin to C++ (first-pass, syntax-only)
// Original: kotlinx-coroutines-core/common/src/flow/terminal/Reduce.kt
//
// TODO: Implement coroutine semantics (suspend functions)
// TODO: Map Kotlin Flow types to C++ equivalents
// TODO: Implement NULL sentinel and Symbol types
// TODO: Implement collectWhile utility
// TODO: Handle inline/crossinline functions

#pragma once

// @file:JvmMultifileClass
// @file:JvmName("FlowKt")
// @file:Suppress("UNCHECKED_CAST")

namespace kotlinx {
namespace coroutines {
namespace flow {

// TODO: import kotlinx.coroutines.flow.internal.*
// TODO: import kotlinx.coroutines.internal.Symbol
// TODO: import kotlin.jvm.*

/**
 * Accumulates value starting with the first element and applying [operation] to current accumulator value and each element.
 * Throws [NoSuchElementException] if flow was empty.
 */
template<typename S, typename T, typename Operation>
S reduce(Flow<T> flow, Operation operation) {
    void* accumulator = NULL;

    flow.collect([&](T value) {
        if (accumulator != NULL) {
            // @Suppress("UNCHECKED_CAST")
            accumulator = operation((S)accumulator, value);
        } else {
            accumulator = value;
        }
    });

    if (accumulator == NULL) {
        throw std::runtime_error("Empty flow can't be reduced"); // NoSuchElementException
    }
    // @Suppress("UNCHECKED_CAST")
    return (S)accumulator;
}

/**
 * Accumulates value starting with [initial] value and applying [operation] current accumulator value and each element
 */
template<typename T, typename R, typename Operation>
R fold(Flow<T> flow, R initial, Operation operation) {
    R accumulator = initial;
    flow.collect([&](T value) {
        accumulator = operation(accumulator, value);
    });
    return accumulator;
}

/**
 * The terminal operator that awaits for one and only one value to be emitted.
 * Throws [NoSuchElementException] for empty flow and [IllegalArgumentException] for flow
 * that contains more than one element.
 */
template<typename T>
T single(Flow<T> flow) {
    void* result = NULL;
    flow.collect([&](T value) {
        // require(result === NULL)
        if (!(result == NULL)) {
            throw std::invalid_argument("Flow has more than one element");
        }
        result = (void*)new T(value);
    });

    if (result == NULL) {
        throw std::runtime_error("Flow is empty"); // NoSuchElementException
    }
    return *(T*)result;
}

/**
 * The terminal operator that awaits for one and only one value to be emitted.
 * Returns the single value or `null`, if the flow was empty or emitted more than one value.
 */
template<typename T>
T* single_or_null(Flow<T> flow) {
    void* result = NULL;
    collect_while(flow, [&](T it) {
        // No values yet, update result
        if (result == NULL) {
            result = (void*)new T(it);
            return true;
        } else {
            // Second value, reset result and bail out
            delete (T*)result;
            result = NULL;
            return false;
        }
    });
    return (result == NULL) ? nullptr : (T*)result;
}

/**
 * The terminal operator that returns the first element emitted by the flow and then cancels flow's collection.
 * Throws [NoSuchElementException] if the flow was empty.
 */
template<typename T>
T first(Flow<T> flow) {
    void* result = NULL;
    collect_while(flow, [&](T it) {
        result = (void*)new T(it);
        return false;
    });
    if (result == NULL) {
        throw std::runtime_error("Expected at least one element"); // NoSuchElementException
    }
    return *(T*)result;
}

/**
 * The terminal operator that returns the first element emitted by the flow matching the given [predicate] and then cancels flow's collection.
 * Throws [NoSuchElementException] if the flow has not contained elements matching the [predicate].
 */
template<typename T, typename Predicate>
T first(Flow<T> flow, Predicate predicate) {
    void* result = NULL;
    collect_while(flow, [&](T it) {
        if (predicate(it)) {
            result = (void*)new T(it);
            return false;
        } else {
            return true;
        }
    });
    if (result == NULL) {
        throw std::runtime_error("Expected at least one element matching the predicate"); // NoSuchElementException
    }
    return *(T*)result;
}

/**
 * The terminal operator that returns the first element emitted by the flow and then cancels flow's collection.
 * Returns `null` if the flow was empty.
 */
template<typename T>
T* first_or_null(Flow<T> flow) {
    T* result = nullptr;
    collect_while(flow, [&](T it) {
        result = new T(it);
        return false;
    });
    return result;
}

/**
 * The terminal operator that returns the first element emitted by the flow matching the given [predicate] and then cancels flow's collection.
 * Returns `null` if the flow did not contain an element matching the [predicate].
 */
template<typename T, typename Predicate>
T* first_or_null(Flow<T> flow, Predicate predicate) {
    T* result = nullptr;
    collect_while(flow, [&](T it) {
        if (predicate(it)) {
            result = new T(it);
            return false;
        } else {
            return true;
        }
    });
    return result;
}

/**
 * The terminal operator that returns the last element emitted by the flow.
 *
 * Throws [NoSuchElementException] if the flow was empty.
 */
template<typename T>
T last(Flow<T> flow) {
    void* result = NULL;
    flow.collect([&](T it) {
        result = (void*)new T(it);
    });
    if (result == NULL) {
        throw std::runtime_error("Expected at least one element"); // NoSuchElementException
    }
    return *(T*)result;
}

/**
 * The terminal operator that returns the last element emitted by the flow or `null` if the flow was empty.
 */
template<typename T>
T* last_or_null(Flow<T> flow) {
    T* result = nullptr;
    flow.collect([&](T it) {
        if (result) delete result;
        result = new T(it);
    });
    return result;
}

} // namespace flow
} // namespace coroutines
} // namespace kotlinx
