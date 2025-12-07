// Transliterated from Kotlin to C++ (first-pass, syntax-only)
// Original: kotlinx-coroutines-core/common/src/flow/terminal/Logic.kt
//
// TODO: Implement coroutine semantics (suspend functions)
// TODO: Map Kotlin Flow types to C++ equivalents
// TODO: Implement collectWhile utility

#pragma once

// @file:JvmMultifileClass
// @file:JvmName("FlowKt")

namespace kotlinx {
namespace coroutines {
namespace flow {

// TODO: import kotlinx.coroutines.*
// TODO: import kotlin.jvm.*

/**
 * A terminal operator that returns `true` and immediately cancels the flow
 * if at least one element matches the given [predicate].
 *
 * If the flow does not emit any elements or no element matches the predicate, the function returns `false`.
 *
 * Equivalent to `!all { !predicate(it) }` (see [Flow.all]) and `!none { predicate(it) }` (see [Flow.none]).
 *
 * Example:
 *
 * ```
 * val myFlow = flow {
 *   repeat(10) {
 *     emit(it)
 *   }
 *   throw RuntimeException("You still didn't find the required number? I gave you ten!")
 * }
 * println(myFlow.any { it > 5 }) // true
 * println(flowOf(1, 2, 3).any { it > 5 }) // false
 * ```
 *
 * @see Iterable.any
 * @see Sequence.any
 */
template<typename T, typename Predicate>
bool any(Flow<T> flow, Predicate predicate) {
    bool found = false;
    collect_while(flow, [&](T it) {
        bool satisfies = predicate(it);
        if (satisfies) found = true;
        return !satisfies;
    });
    return found;
}

/**
 * A terminal operator that returns `true` if all elements match the given [predicate],
 * or returns `false` and cancels the flow as soon as the first element not matching the predicate is encountered.
 *
 * If the flow terminates without emitting any elements, the function returns `true` because there
 * are no elements in it that *do not* match the predicate.
 * See a more detailed explanation of this logic concept in the
 * ["Vacuous truth"](https://en.wikipedia.org/wiki/Vacuous_truth) article.
 *
 * Equivalent to `!any { !predicate(it) }` (see [Flow.any]) and `none { !predicate(it) }` (see [Flow.none]).
 *
 * Example:
 *
 * ```
 * val myFlow = flow {
 *   repeat(10) {
 *     emit(it)
 *   }
 *   throw RuntimeException("You still didn't find the required number? I gave you ten!")
 * }
 * println(myFlow.all { it <= 5 }) // false
 * println(flowOf(1, 2, 3).all { it <= 5 }) // true
 * ```
 *
 * @see Iterable.all
 * @see Sequence.all
 */
template<typename T, typename Predicate>
bool all(Flow<T> flow, Predicate predicate) {
    bool found_counter_example = false;
    collect_while(flow, [&](T it) {
        bool satisfies = predicate(it);
        if (!satisfies) found_counter_example = true;
        return satisfies;
    });
    return !found_counter_example;
}

/**
 * A terminal operator that returns `true` if no elements match the given [predicate],
 * or returns `false` and cancels the flow as soon as the first element matching the predicate is encountered.
 *
 * If the flow terminates without emitting any elements, the function returns `true` because there
 * are no elements in it that match the predicate.
 * See a more detailed explanation of this logic concept in the
 * ["Vacuous truth"](https://en.wikipedia.org/wiki/Vacuous_truth) article.
 *
 * Equivalent to `!any(predicate)` (see [Flow.any]) and `all { !predicate(it) }` (see [Flow.all]).
 *
 * Example:
 * ```
 * val myFlow = flow {
 *   repeat(10) {
 *     emit(it)
 *   }
 *   throw RuntimeException("You still didn't find the required number? I gave you ten!")
 * }
 * println(myFlow.none { it > 5 }) // false
 * println(flowOf(1, 2, 3).none { it > 5 }) // true
 * ```
 *
 * @see Iterable.none
 * @see Sequence.none
 */
template<typename T, typename Predicate>
bool none(Flow<T> flow, Predicate predicate) {
    return !any(flow, predicate);
}

} // namespace flow
} // namespace coroutines
} // namespace kotlinx
