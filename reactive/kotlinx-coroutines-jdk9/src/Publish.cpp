// Transliterated from: reactive/kotlinx-coroutines-jdk9/src/Publish.cpp

// TODO: #include equivalent for kotlinx.coroutines.*
// TODO: #include equivalent for kotlinx.coroutines.channels.*
// TODO: #include equivalent for kotlinx.coroutines.reactive.*
// TODO: #include equivalent for java.util.concurrent.*
// TODO: #include equivalent for kotlin.coroutines.*
// TODO: #include equivalent for org.reactivestreams.FlowAdapters

namespace kotlinx {
namespace coroutines {
namespace jdk9 {

/**
 * Creates a cold reactive [Flow.Publisher] that runs a given [block] in a coroutine.
 *
 * Every time the returned flux is subscribed, it starts a new coroutine in the specified [context].
 * The coroutine emits (via [Flow.Subscriber.onNext]) values with [send][ProducerScope.send],
 * completes (via [Flow.Subscriber.onComplete]) when the coroutine completes or channel is explicitly closed, and emits
 * errors (via [Flow.Subscriber.onError]) if the coroutine throws an exception or closes channel with a cause.
 * Unsubscribing cancels the running coroutine.
 *
 * Invocations of [send][ProducerScope.send] are suspended appropriately when subscribers apply back-pressure and to
 * ensure that [onNext][Flow.Subscriber.onNext] is not invoked concurrently.
 *
 * Coroutine context can be specified with [context] argument.
 * If the context does not have any dispatcher nor any other [ContinuationInterceptor], then [Dispatchers.Default] is
 * used.
 *
 * **Note: This is an experimental api.** Behaviour of publishers that work as children in a parent scope with respect
 *        to cancellation and error handling may change in the future.
 *
 * @throws IllegalArgumentException if the provided [context] contains a [Job] instance.
 */
// @BuilderInference
template<typename T>
/* Flow.Publisher<T> */ auto flow_publish(
    CoroutineContext context /* = EmptyCoroutineContext */,
    std::function<void(ProducerScope<T>&)> block
) /* -> Flow.Publisher<T> */ {
    // TODO: implement coroutine suspension
    // return FlowAdapters.toFlowPublisher(publish(context, block))
}

} // namespace jdk9
} // namespace coroutines
} // namespace kotlinx

// TODO: Semantic implementation tasks:
// 1. Implement CoroutineContext and EmptyCoroutineContext
// 2. Implement ProducerScope<T> with send() method
// 3. Implement publish() function from kotlinx.coroutines.reactive
// 4. Implement FlowAdapters.toFlowPublisher adapter
// 5. Implement Flow.Publisher and Flow.Subscriber interfaces
// 6. Handle IllegalArgumentException when context contains Job
// 7. Implement proper back-pressure handling
// 8. Implement cancellation propagation from subscription to coroutine
// 9. Implement default dispatcher (Dispatchers.Default)
// 10. Implement ContinuationInterceptor detection
// 11. Add builder inference equivalent for template deduction
