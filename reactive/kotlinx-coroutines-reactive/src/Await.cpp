// Transliterated from: reactive/kotlinx-coroutines-reactive/src/Await.kt
// TODO: Implement semantic correctness for reactive streams await operations

namespace kotlinx {
namespace coroutines {
namespace reactive {

// TODO: #include <kotlinx/coroutines/coroutines.hpp>
// TODO: #include <org/reactivestreams/Publisher.hpp>
// TODO: #include <org/reactivestreams/Subscriber.hpp>
// TODO: #include <org/reactivestreams/Subscription.hpp>
// TODO: #include <exception>
// TODO: #include <functional>
// TODO: #include <optional>

/**
 * Awaits the first value from the given publisher without blocking the thread and returns the resulting value, or, if
 * the publisher has produced an error, throws the corresponding exception.
 *
 * This suspending function is cancellable.
 * If the [Job] of the current coroutine is cancelled while the suspending function is waiting, this
 * function immediately cancels its [Subscription] and resumes with [CancellationException].
 *
 * @throws NoSuchElementException if the publisher does not emit any value
 */
// TODO: implement coroutine suspension
template<typename T>
T await_first(Publisher<T>& publisher) {
    return await_one(publisher, Mode::kFirst);
}

/**
 * Awaits the first value from the given publisher, or returns the [default] value if none is emitted, without blocking
 * the thread, and returns the resulting value, or, if this publisher has produced an error, throws the corresponding
 * exception.
 *
 * This suspending function is cancellable.
 * If the [Job] of the current coroutine is cancelled while the suspending function is waiting, this
 * function immediately cancels its [Subscription] and resumes with [CancellationException].
 */
// TODO: implement coroutine suspension
template<typename T>
T await_first_or_default(Publisher<T>& publisher, T default_value) {
    return await_one(publisher, Mode::kFirstOrDefault, default_value);
}

/**
 * Awaits the first value from the given publisher, or returns `null` if none is emitted, without blocking the thread,
 * and returns the resulting value, or, if this publisher has produced an error, throws the corresponding exception.
 *
 * This suspending function is cancellable.
 * If the [Job] of the current coroutine is cancelled while the suspending function is waiting, this
 * function immediately cancels its [Subscription] and resumes with [CancellationException].
 */
// TODO: implement coroutine suspension
template<typename T>
T* await_first_or_null(Publisher<T>& publisher) {
    return await_one(publisher, Mode::kFirstOrDefault);
}

/**
 * Awaits the first value from the given publisher, or calls [defaultValue] to get a value if none is emitted, without
 * blocking the thread, and returns the resulting value, or, if this publisher has produced an error, throws the
 * corresponding exception.
 *
 * This suspending function is cancellable.
 * If the [Job] of the current coroutine is cancelled while the suspending function is waiting, this
 * function immediately cancels its [Subscription] and resumes with [CancellationException].
 */
// TODO: implement coroutine suspension
template<typename T>
T await_first_or_else(Publisher<T>& publisher, std::function<T()> default_value) {
    auto result = await_one(publisher, Mode::kFirstOrDefault);
    return result ? *result : default_value();
}

/**
 * Awaits the last value from the given publisher without blocking the thread and
 * returns the resulting value, or, if this publisher has produced an error, throws the corresponding exception.
 *
 * This suspending function is cancellable.
 * If the [Job] of the current coroutine is cancelled while the suspending function is waiting, this
 * function immediately cancels its [Subscription] and resumes with [CancellationException].
 *
 * @throws NoSuchElementException if the publisher does not emit any value
 */
// TODO: implement coroutine suspension
template<typename T>
T await_last(Publisher<T>& publisher) {
    return await_one(publisher, Mode::kLast);
}

/**
 * Awaits the single value from the given publisher without blocking the thread and returns the resulting value, or,
 * if this publisher has produced an error, throws the corresponding exception.
 *
 * This suspending function is cancellable.
 * If the [Job] of the current coroutine is cancelled while the suspending function is waiting, this
 * function immediately cancels its [Subscription] and resumes with [CancellationException].
 *
 * @throws NoSuchElementException if the publisher does not emit any value
 * @throws IllegalArgumentException if the publisher emits more than one value
 */
// TODO: implement coroutine suspension
template<typename T>
T await_single(Publisher<T>& publisher) {
    return await_one(publisher, Mode::kSingle);
}

/**
 * Awaits the single value from the given publisher, or returns the [default] value if none is emitted, without
 * blocking the thread, and returns the resulting value, or, if this publisher has produced an error, throws the
 * corresponding exception.
 *
 * This suspending function is cancellable.
 * If the [Job] of the current coroutine is cancelled while the suspending function is waiting, this
 * function immediately cancels its [Subscription] and resumes with [CancellationException].
 *
 * ### Deprecation
 *
 * This method is deprecated because the conventions established in Kotlin mandate that an operation with the name
 * `awaitSingleOrDefault` returns the default value instead of throwing in case there is an error; however, this would
 * also mean that this method would return the default value if there are *too many* values. This could be confusing to
 * those who expect this function to validate that there is a single element or none at all emitted, and cases where
 * there are no elements are indistinguishable from those where there are too many, though these cases have different
 * meaning.
 *
 * @throws NoSuchElementException if the publisher does not emit any value
 * @throws IllegalArgumentException if the publisher emits more than one value
 *
 * @suppress
 */
// @Deprecated(
//     message = "Deprecated without a replacement due to its name incorrectly conveying the behavior. " +
//         "Please consider using awaitFirstOrDefault().",
//     level = DeprecationLevel.HIDDEN
// ) // Warning since 1.5, error in 1.6, hidden in 1.7
// TODO: implement coroutine suspension
template<typename T>
[[deprecated("Deprecated without a replacement due to its name incorrectly conveying the behavior. Please consider using await_first_or_default().")]]
T await_single_or_default(Publisher<T>& publisher, T default_value) {
    return await_one(publisher, Mode::kSingleOrDefault, default_value);
}

/**
 * Awaits the single value from the given publisher without blocking the thread and returns the resulting value, or, if
 * this publisher has produced an error, throws the corresponding exception. If more than one value or none were
 * produced by the publisher, `null` is returned.
 *
 * This suspending function is cancellable.
 * If the [Job] of the current coroutine is cancelled while the suspending function is waiting, this
 * function immediately cancels its [Subscription] and resumes with [CancellationException].
 *
 * ### Deprecation
 *
 * This method is deprecated because the conventions established in Kotlin mandate that an operation with the name
 * `awaitSingleOrNull` returns `null` instead of throwing in case there is an error; however, this would
 * also mean that this method would return `null` if there are *too many* values. This could be confusing to
 * those who expect this function to validate that there is a single element or none at all emitted, and cases where
 * there are no elements are indistinguishable from those where there are too many, though these cases have different
 * meaning.
 *
 * @throws IllegalArgumentException if the publisher emits more than one value
 * @suppress
 */
// @Deprecated(
//     message = "Deprecated without a replacement due to its name incorrectly conveying the behavior. " +
//         "There is a specialized version for Reactor's Mono, please use that where applicable. " +
//         "Alternatively, please consider using awaitFirstOrNull().",
//     level = DeprecationLevel.HIDDEN,
//     replaceWith = ReplaceWith("this.awaitSingleOrNull()", "kotlinx.coroutines.reactor")
// ) // Warning since 1.5, error in 1.6, hidden in 1.7
// TODO: implement coroutine suspension
template<typename T>
[[deprecated("Deprecated without a replacement due to its name incorrectly conveying the behavior. There is a specialized version for Reactor's Mono, please use that where applicable. Alternatively, please consider using await_first_or_null().")]]
T* await_single_or_null(Publisher<T>& publisher) {
    return await_one(publisher, Mode::kSingleOrDefault);
}

/**
 * Awaits the single value from the given publisher, or calls [defaultValue] to get a value if none is emitted, without
 * blocking the thread, and returns the resulting value, or, if this publisher has produced an error, throws the
 * corresponding exception.
 *
 * This suspending function is cancellable.
 * If the [Job] of the current coroutine is cancelled while the suspending function is waiting, this
 * function immediately cancels its [Subscription] and resumes with [CancellationException].
 *
 * ### Deprecation
 *
 * This method is deprecated because the conventions established in Kotlin mandate that an operation with the name
 * `awaitSingleOrElse` returns the calculated value instead of throwing in case there is an error; however, this would
 * also mean that this method would return the calculated value if there are *too many* values. This could be confusing
 * to those who expect this function to validate that there is a single element or none at all emitted, and cases where
 * there are no elements are indistinguishable from those where there are too many, though these cases have different
 * meaning.
 *
 * @throws IllegalArgumentException if the publisher emits more than one value
 * @suppress
 */
// @Deprecated(
//     message = "Deprecated without a replacement due to its name incorrectly conveying the behavior. " +
//         "Please consider using awaitFirstOrElse().",
//     level = DeprecationLevel.HIDDEN
// ) // Warning since 1.5, error in 1.6, hidden in 1.7
// TODO: implement coroutine suspension
template<typename T>
[[deprecated("Deprecated without a replacement due to its name incorrectly conveying the behavior. Please consider using await_first_or_else().")]]
T await_single_or_else(Publisher<T>& publisher, std::function<T()> default_value) {
    auto result = await_one(publisher, Mode::kSingleOrDefault);
    return result ? *result : default_value();
}

// ------------------------ private ------------------------

enum class Mode {
    kFirst,
    kFirstOrDefault,
    kLast,
    kSingle,
    kSingleOrDefault
};

// TODO: implement coroutine suspension
template<typename T>
T await_one(Publisher<T>& publisher, Mode mode, std::optional<T> default_value = std::nullopt) {
    // TODO: Implement suspendCancellableCoroutine equivalent
    /* This implementation must obey
    https://github.com/reactive-streams/reactive-streams-jvm/blob/v1.0.3/README.md#2-subscriber-code
    The numbers of rules are taken from there. */
    // TODO: Implement full subscriber logic with:
    // - Subscription management
    // - Value tracking (seenValue, value, inTerminalState)
    // - onSubscribe, onNext, onComplete, onError handlers
    // - Context injection via injectCoroutineContext
    // - Serial execution enforcement via withSubscriptionLock
    // - Cancellation handling
    throw std::runtime_error("Not implemented");
}

/**
 * Enforce rule 2.4 (detect publishers that don't respect rule 1.7): don't process anything after a terminal
 * state was reached.
 */
void got_signal_in_terminal_state_exception(const CoroutineContext& context, const std::string& signal_name) {
    handle_coroutine_exception(context,
        std::runtime_error("'" + signal_name + "' was called after the publisher already signalled being in a terminal state"));
}

/**
 * Enforce rule 1.1: it is invalid for a publisher to provide more values than requested.
 */
void more_than_one_value_provided_exception(const CoroutineContext& context, Mode mode) {
    std::string mode_str;
    switch (mode) {
        case Mode::kFirst: mode_str = "awaitFirst"; break;
        case Mode::kFirstOrDefault: mode_str = "awaitFirstOrDefault"; break;
        case Mode::kLast: mode_str = "awaitLast"; break;
        case Mode::kSingle: mode_str = "awaitSingle"; break;
        case Mode::kSingleOrDefault: mode_str = "awaitSingleOrDefault"; break;
    }
    handle_coroutine_exception(context,
        std::runtime_error("Only a single value was requested in '" + mode_str + "', but the publisher provided more"));
}

} // namespace reactive
} // namespace coroutines
} // namespace kotlinx

/*
 * TODO List for semantic implementation:
 *
 * 1. Implement C++ coroutine support (co_await, co_return)
 * 2. Implement CoroutineContext and Job management
 * 3. Implement Subscription lifecycle management
 * 4. Implement reactive streams Subscriber interface
 * 5. Implement thread-safe subscription operations (@Synchronized equivalent)
 * 6. Implement suspendCancellableCoroutine primitive
 * 7. Implement context injection (injectCoroutineContext)
 * 8. Implement exception handling (handleCoroutineException)
 * 9. Implement cancellation support (CancellationException)
 * 10. Add proper error handling for reactive streams rules
 * 11. Implement continuation resume/resumeWithException
 * 12. Add mutex/lock primitives for serial execution
 * 13. Implement std::optional<T> for nullable types
 * 14. Add NoSuchElementException type
 * 15. Add IllegalArgumentException type
 * 16. Test compliance with reactive streams specification
 */
