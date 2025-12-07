// Transliterated from Kotlin to C++ (first-pass, syntax-only)
// Original: kotlinx-coroutines-core/common/src/flow/operators/Lint.kt
//
// TODO: Map deprecated/suppression annotations to C++ equivalents
// TODO: Implement SharedFlow and StateFlow operator fusion behavior
// TODO: Handle FlowCollector context access prevention
// TODO: Map cancellation and context to C++ coroutines

#pragma once

// @file:Suppress("unused", "INVISIBLE_REFERENCE", "INVISIBLE_MEMBER", "UNUSED_PARAMETER")

namespace kotlinx {
namespace coroutines {
namespace flow {

// TODO: import kotlinx.coroutines.*
// TODO: import kotlin.coroutines.*
// TODO: import kotlin.internal.InlineOnly

/**
 * Applying [cancellable][Flow.cancellable] to a [SharedFlow] has no effect.
 * See the [SharedFlow] documentation on Operator Fusion.
 * @suppress
 */
// @Deprecated(
//     level = DeprecationLevel.ERROR,
//     message = "Applying 'cancellable' to a SharedFlow has no effect. See the SharedFlow documentation on Operator Fusion.",
//     replaceWith = ReplaceWith("this")
// )
template<typename T>
[[deprecated("Applying 'cancellable' to a SharedFlow has no effect. See the SharedFlow documentation on Operator Fusion.")]]
Flow<T> cancellable(SharedFlow<T> flow) {
    // noImpl()
    return flow; // Return this
}

/**
 * Applying [flowOn][Flow.flowOn] to [SharedFlow] has no effect.
 * See the [SharedFlow] documentation on Operator Fusion.
 * @suppress
 */
// @Deprecated(
//     level = DeprecationLevel.ERROR,
//     message = "Applying 'flowOn' to SharedFlow has no effect. See the SharedFlow documentation on Operator Fusion.",
//     replaceWith = ReplaceWith("this")
// )
template<typename T>
[[deprecated("Applying 'flowOn' to SharedFlow has no effect. See the SharedFlow documentation on Operator Fusion.")]]
Flow<T> flow_on(SharedFlow<T> flow, CoroutineContext context) {
    // noImpl()
    return flow; // Return this
}

/**
 * Applying [conflate][Flow.conflate] to [StateFlow] has no effect.
 * See the [StateFlow] documentation on Operator Fusion.
 * @suppress
 */
// @Deprecated(
//     level = DeprecationLevel.ERROR,
//     message = "Applying 'conflate' to StateFlow has no effect. See the StateFlow documentation on Operator Fusion.",
//     replaceWith = ReplaceWith("this")
// )
template<typename T>
[[deprecated("Applying 'conflate' to StateFlow has no effect. See the StateFlow documentation on Operator Fusion.")]]
Flow<T> conflate(StateFlow<T> flow) {
    // noImpl()
    return flow; // Return this
}

/**
 * Applying [distinctUntilChanged][Flow.distinctUntilChanged] to [StateFlow] has no effect.
 * See the [StateFlow] documentation on Operator Fusion.
 * @suppress
 */
// @Deprecated(
//     level = DeprecationLevel.ERROR,
//     message = "Applying 'distinctUntilChanged' to StateFlow has no effect. See the StateFlow documentation on Operator Fusion.",
//     replaceWith = ReplaceWith("this")
// )
template<typename T>
[[deprecated("Applying 'distinctUntilChanged' to StateFlow has no effect. See the StateFlow documentation on Operator Fusion.")]]
Flow<T> distinct_until_changed(StateFlow<T> flow) {
    // noImpl()
    return flow; // Return this
}

/**
 * @suppress
 */
// @Deprecated(
//     message = "isActive is resolved into the extension of outer CoroutineScope which is likely to be an error. " +
//         "Use currentCoroutineContext().isActive or cancellable() operator instead " +
//         "or specify the receiver of isActive explicitly. " +
//         "Additionally, flow {} builder emissions are cancellable by default.",
//     level = DeprecationLevel.ERROR,
//     replaceWith = ReplaceWith("currentCoroutineContext().isActive")
// )
template<typename T>
struct [[deprecated("isActive is resolved into the extension of outer CoroutineScope. Use currentCoroutineContext().isActive instead")]]
FlowCollectorIsActive {
    // This property should not be accessed
    // bool is_active;
};

/**
 * @suppress
 */
// @Deprecated(
//     message = "cancel() is resolved into the extension of outer CoroutineScope which is likely to be an error. " +
//         "Use currentCoroutineContext().cancel() instead or specify the receiver of cancel() explicitly",
//     level = DeprecationLevel.ERROR,
//     replaceWith = ReplaceWith("currentCoroutineContext().cancel(cause)")
// )
template<typename T>
[[deprecated("cancel() is resolved into the extension of outer CoroutineScope. Use currentCoroutineContext().cancel(cause) instead")]]
void cancel(FlowCollector<T>*, CancellationException* cause = nullptr) {
    // noImpl()
}

/**
 * @suppress
 */
// @Deprecated(
//     message = "coroutineContext is resolved into the property of outer CoroutineScope which is likely to be an error. " +
//         "Use currentCoroutineContext() instead or specify the receiver of coroutineContext explicitly",
//     level = DeprecationLevel.ERROR,
//     replaceWith = ReplaceWith("currentCoroutineContext()")
// )
template<typename T>
struct [[deprecated("coroutineContext is resolved into the property of outer CoroutineScope. Use currentCoroutineContext() instead")]]
FlowCollectorCoroutineContext {
    // This property should not be accessed
    // CoroutineContext coroutine_context;
};

/**
 * @suppress
 */
// @Deprecated(
//     message = "SharedFlow never completes, so this operator typically has not effect, it can only " +
//         "catch exceptions from 'onSubscribe' operator",
//     level = DeprecationLevel.WARNING,
//     replaceWith = ReplaceWith("this")
// )
// @InlineOnly
template<typename T, typename Action>
[[deprecated("SharedFlow never completes, so this operator typically has no effect")]]
inline Flow<T> catch(SharedFlow<T> flow, Action action) {
    return catch((Flow<T>&)flow, action);
}

/**
 * @suppress
 */
// @Deprecated(
//     message = "SharedFlow never completes, so this operator has no effect.",
//     level = DeprecationLevel.WARNING,
//     replaceWith = ReplaceWith("this")
// )
// @InlineOnly
template<typename T, typename Predicate>
[[deprecated("SharedFlow never completes, so this operator has no effect")]]
inline Flow<T> retry(SharedFlow<T> flow, long retries = LONG_MAX, Predicate predicate = [](auto) { return true; }) {
    return retry((Flow<T>&)flow, retries, predicate);
}

/**
 * @suppress
 */
// @Deprecated(
//     message = "SharedFlow never completes, so this operator has no effect.",
//     level = DeprecationLevel.WARNING,
//     replaceWith = ReplaceWith("this")
// )
// @InlineOnly
template<typename T, typename Predicate>
[[deprecated("SharedFlow never completes, so this operator has no effect")]]
inline Flow<T> retry_when(SharedFlow<T> flow, Predicate predicate) {
    return retry_when((Flow<T>&)flow, predicate);
}

/**
 * @suppress
 */
// @Suppress("DeprecatedCallableAddReplaceWith")
// @Deprecated(
//     message = "SharedFlow never completes, so this terminal operation never completes.",
//     level = DeprecationLevel.WARNING
// )
// @InlineOnly
template<typename T>
[[deprecated("SharedFlow never completes, so this terminal operation never completes")]]
inline std::vector<T> to_list(SharedFlow<T> flow) {
    return to_list((Flow<T>&)flow);
}

/**
 * A specialized version of [Flow.toList] that returns [Nothing]
 * to indicate that [SharedFlow] collection never completes.
 */
// @InlineOnly
template<typename T>
inline void to_list(SharedFlow<T> flow, std::vector<T>& destination) {
    to_list((Flow<T>&)flow, destination);
    throw std::runtime_error("this code is supposed to be unreachable");
}

/**
 * @suppress
 */
// @Suppress("DeprecatedCallableAddReplaceWith")
// @Deprecated(
//     message = "SharedFlow never completes, so this terminal operation never completes.",
//     level = DeprecationLevel.WARNING
// )
// @InlineOnly
template<typename T>
[[deprecated("SharedFlow never completes, so this terminal operation never completes")]]
inline std::unordered_set<T> to_set(SharedFlow<T> flow) {
    return to_set((Flow<T>&)flow);
}

/**
 * A specialized version of [Flow.toSet] that returns [Nothing]
 * to indicate that [SharedFlow] collection never completes.
 */
// @InlineOnly
template<typename T>
inline void to_set(SharedFlow<T> flow, std::unordered_set<T>& destination) {
    to_set((Flow<T>&)flow, destination);
    throw std::runtime_error("this code is supposed to be unreachable");
}

/**
 * @suppress
 */
// @Suppress("DeprecatedCallableAddReplaceWith")
// @Deprecated(
//     message = "SharedFlow never completes, so this terminal operation never completes.",
//     level = DeprecationLevel.WARNING
// )
// @InlineOnly
template<typename T>
[[deprecated("SharedFlow never completes, so this terminal operation never completes")]]
inline int count(SharedFlow<T> flow) {
    return count((Flow<T>&)flow);
}

} // namespace flow
} // namespace coroutines
} // namespace kotlinx
