// Transliterated from: reactive/kotlinx-coroutines-jdk9/src/ReactiveFlow.cpp

// TODO: #include equivalent for kotlinx.coroutines.*
// TODO: #include equivalent for kotlinx.coroutines.flow.*
// TODO: #include equivalent for kotlinx.coroutines.reactive.asFlow
// TODO: #include equivalent for kotlinx.coroutines.reactive.asPublisher as asReactivePublisher
// TODO: #include equivalent for kotlinx.coroutines.reactive.collect
// TODO: #include equivalent for kotlinx.coroutines.channels.*
// TODO: #include equivalent for org.reactivestreams.*
// TODO: #include equivalent for kotlin.coroutines.*
// TODO: #include equivalent for java.util.concurrent.Flow as JFlow

namespace kotlinx {
namespace coroutines {
namespace jdk9 {

/**
 * Transforms the given reactive [Flow Publisher][JFlow.Publisher] into [Flow].
 * Use the [buffer] operator on the resulting flow to specify the size of the back-pressure.
 * In effect, it specifies the value of the subscription's [request][JFlow.Subscription.request].
 * The [default buffer capacity][Channel.BUFFERED] for a suspending channel is used by default.
 *
 * If any of the resulting flow transformations fails, the subscription is immediately cancelled and all the in-flight
 * elements are discarded.
 */
template<typename T>
/* Flow<T> */ auto as_flow(/* JFlow.Publisher<T>& publisher */) /* -> Flow<T> */ {
    // return FlowAdapters.toPublisher(this).asFlow()
}

/**
 * Transforms the given flow into a reactive specification compliant [Flow Publisher][JFlow.Publisher].
 *
 * An optional [context] can be specified to control the execution context of calls to the [Flow Subscriber][Subscriber]
 * methods.
 * A [CoroutineDispatcher] can be set to confine them to a specific thread; various [ThreadContextElement] can be set to
 * inject additional context into the caller thread. By default, the [Unconfined][Dispatchers.Unconfined] dispatcher
 * is used, so calls are performed from an arbitrary thread.
 */
// @JvmOverloads // binary compatibility
template<typename T>
/* JFlow.Publisher<T> */ auto as_publisher(/* Flow<T>& flow, */ CoroutineContext context /* = EmptyCoroutineContext */) /* -> JFlow.Publisher<T> */ {
    // return FlowAdapters.toFlowPublisher(asReactivePublisher(context))
}

/**
 * Subscribes to this [Flow Publisher][JFlow.Publisher] and performs the specified action for each received element.
 *
 * If [action] throws an exception at some point, the subscription is cancelled, and the exception is rethrown from
 * [collect]. Also, if the publisher signals an error, that error is rethrown from [collect].
 */
template<typename T>
void collect(/* JFlow.Publisher<T>& publisher, */ std::function<void(T)> action) {
    // TODO: implement coroutine suspension
    // FlowAdapters.toPublisher(this).collect(action)
}

} // namespace jdk9
} // namespace coroutines
} // namespace kotlinx

// TODO: Semantic implementation tasks:
// 1. Implement Flow<T> type
// 2. Implement JFlow.Publisher<T> and JFlow.Subscriber<T> interfaces
// 3. Implement JFlow.Subscription interface with request() method
// 4. Implement FlowAdapters.toPublisher and FlowAdapters.toFlowPublisher adapters
// 5. Implement asFlow() conversion from reactive Publisher to Flow
// 6. Implement asReactivePublisher() conversion from Flow to reactive Publisher
// 7. Implement buffer operator for Flow
// 8. Implement Channel.BUFFERED constant
// 9. Implement collect() as suspending function
// 10. Handle subscription cancellation on flow failure
// 11. Implement CoroutineContext and EmptyCoroutineContext
// 12. Implement CoroutineDispatcher and Dispatchers.Unconfined
// 13. Implement ThreadContextElement
// 14. Add JvmOverloads equivalent for default parameters
// 15. Implement inline function optimization
