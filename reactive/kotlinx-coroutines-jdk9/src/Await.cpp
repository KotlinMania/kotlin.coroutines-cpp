// Transliterated from: reactive/kotlinx-coroutines-jdk9/src/Await.cpp

// TODO: #include equivalent for kotlinx.coroutines.*
// TODO: #include equivalent for java.util.concurrent.*
// TODO: #include equivalent for org.reactivestreams.FlowAdapters
// TODO: #include equivalent for kotlinx.coroutines.reactive.*

namespace kotlinx {
namespace coroutines {
namespace jdk9 {

/**
 * Awaits the first value from the given publisher without blocking the thread and returns the resulting value, or, if
 * the publisher has produced an error, throws the corresponding exception.
 *
 * This suspending function is cancellable.
 * If the [Job] of the current coroutine is cancelled while the suspending function is waiting, this
 * function immediately cancels its [Flow.Subscription] and resumes with [CancellationException].
 *
 * @throws NoSuchElementException if the publisher does not emit any value
 */
template<typename T>
T await_first(/* Flow.Publisher<T>& publisher */) {
    // TODO: implement coroutine suspension
    // return FlowAdapters.toPublisher(this).awaitFirst()
}

/**
 * Awaits the first value from the given publisher, or returns the [default] value if none is emitted, without blocking
 * the thread, and returns the resulting value, or, if this publisher has produced an error, throws the corresponding
 * exception.
 *
 * This suspending function is cancellable.
 * If the [Job] of the current coroutine is cancelled while the suspending function is waiting, this
 * function immediately cancels its [Flow.Subscription] and resumes with [CancellationException].
 */
template<typename T>
T await_first_or_default(/* Flow.Publisher<T>& publisher, */ T default_value) {
    // TODO: implement coroutine suspension
    // return FlowAdapters.toPublisher(this).awaitFirstOrDefault(default)
}

/**
 * Awaits the first value from the given publisher, or returns `null` if none is emitted, without blocking the thread,
 * and returns the resulting value, or, if this publisher has produced an error, throws the corresponding exception.
 *
 * This suspending function is cancellable.
 * If the [Job] of the current coroutine is cancelled while the suspending function is waiting, this
 * function immediately cancels its [Flow.Subscription] and resumes with [CancellationException].
 */
template<typename T>
T* await_first_or_null(/* Flow.Publisher<T>& publisher */) {
    // TODO: implement coroutine suspension
    // return FlowAdapters.toPublisher(this).awaitFirstOrNull()
}

/**
 * Awaits the first value from the given publisher, or calls [defaultValue] to get a value if none is emitted, without
 * blocking the thread, and returns the resulting value, or, if this publisher has produced an error, throws the
 * corresponding exception.
 *
 * This suspending function is cancellable.
 * If the [Job] of the current coroutine is cancelled while the suspending function is waiting, this
 * function immediately cancels its [Flow.Subscription] and resumes with [CancellationException].
 */
template<typename T>
T await_first_or_else(/* Flow.Publisher<T>& publisher, */ std::function<T()> default_value) {
    // TODO: implement coroutine suspension
    // return FlowAdapters.toPublisher(this).awaitFirstOrElse(defaultValue)
}

/**
 * Awaits the last value from the given publisher without blocking the thread and
 * returns the resulting value, or, if this publisher has produced an error, throws the corresponding exception.
 *
 * This suspending function is cancellable.
 * If the [Job] of the current coroutine is cancelled while the suspending function is waiting, this
 * function immediately cancels its [Flow.Subscription] and resumes with [CancellationException].
 *
 * @throws NoSuchElementException if the publisher does not emit any value
 */
template<typename T>
T await_last(/* Flow.Publisher<T>& publisher */) {
    // TODO: implement coroutine suspension
    // return FlowAdapters.toPublisher(this).awaitLast()
}

/**
 * Awaits the single value from the given publisher without blocking the thread and returns the resulting value, or,
 * if this publisher has produced an error, throws the corresponding exception.
 *
 * This suspending function is cancellable.
 * If the [Job] of the current coroutine is cancelled while the suspending function is waiting, this
 * function immediately cancels its [Flow.Subscription] and resumes with [CancellationException].
 *
 * @throws NoSuchElementException if the publisher does not emit any value
 * @throws IllegalArgumentException if the publisher emits more than one value
 */
template<typename T>
T await_single(/* Flow.Publisher<T>& publisher */) {
    // TODO: implement coroutine suspension
    // return FlowAdapters.toPublisher(this).awaitSingle()
}

} // namespace jdk9
} // namespace coroutines
} // namespace kotlinx

// TODO: Semantic implementation tasks:
// 1. Implement Flow.Publisher extension methods as free functions or class methods
// 2. Implement coroutine suspension mechanism for all await_* functions
// 3. Implement FlowAdapters.toPublisher adapter
// 4. Handle CancellationException properly
// 5. Handle NoSuchElementException for empty publishers
// 6. Handle IllegalArgumentException for awaitSingle with multiple values
// 7. Implement proper subscription cancellation on coroutine cancellation
// 8. Add proper template parameter handling for nullable types
