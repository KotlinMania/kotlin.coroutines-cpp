// Transliterated from Kotlin to C++ (first-pass, syntax-only)
// Original: kotlinx-coroutines-core/common/src/flow/operators/Context.kt
//
// TODO: Implement coroutine semantics (suspend functions, coroutine context)
// TODO: Map Kotlin Flow types to C++ equivalents
// TODO: Handle Channel types and buffer overflow strategies
// TODO: Implement FusibleFlow interface
// TODO: Map Job type from coroutine context
// TODO: Implement context propagation

#pragma once

// @file:JvmMultifileClass
// @file:JvmName("FlowKt")

namespace kotlinx {
namespace coroutines {
namespace flow {

// TODO: import kotlinx.coroutines.*
// TODO: import kotlinx.coroutines.channels.*
// TODO: import kotlinx.coroutines.channels.Channel.Factory.BUFFERED
// TODO: import kotlinx.coroutines.channels.Channel.Factory.CONFLATED
// TODO: import kotlinx.coroutines.flow.internal.*
// TODO: import kotlin.coroutines.*
// TODO: import kotlin.jvm.*

/**
 * Buffers flow emissions via channel of a specified capacity and runs collector in a separate coroutine.
 *
 * Normally, [flows][Flow] are _sequential_. It means that the code of all operators is executed in the
 * same coroutine. For example, consider the following code using [onEach] and [collect] operators:
 *
 * ```
 * flowOf("A", "B", "C")
 *     .onEach  { println("1$it") }
 *     .collect { println("2$it") }
 * ```
 *
 * It is going to be executed in the following order by the coroutine `Q` that calls this code:
 *
 * ```
 * Q : -->-- [1A] -- [2A] -- [1B] -- [2B] -- [1C] -- [2C] -->--
 * ```
 *
 * So if the operator's code takes considerable time to execute, then the total execution time is going to be
 * the sum of execution times for all operators.
 *
 * The `buffer` operator creates a separate coroutine during execution for the flow it applies to.
 * Consider the following code:
 *
 * ```
 * flowOf("A", "B", "C")
 *     .onEach  { println("1$it") }
 *     .buffer()  // <--------------- buffer between onEach and collect
 *     .collect { println("2$it") }
 * ```
 *
 * It will use two coroutines for execution of the code. A coroutine `Q` that calls this code is
 * going to execute `collect`, and the code before `buffer` will be executed in a separate
 * new coroutine `P` concurrently with `Q`:
 *
 * ```
 * P : -->-- [1A] -- [1B] -- [1C] ---------->--  // flowOf(...).onEach { ... }
 *
 *                       |
 *                       | channel               // buffer()
 *                       V
 *
 * Q : -->---------- [2A] -- [2B] -- [2C] -->--  // collect
 * ```
 *
 * When the operator's code takes some time to execute, this decreases the total execution time of the flow.
 * A [channel][Channel] is used between the coroutines to send elements emitted by the coroutine `P` to
 * the coroutine `Q`. If the code before `buffer` operator (in the coroutine `P`) is faster than the code after
 * `buffer` operator (in the coroutine `Q`), then this channel will become full at some point and will suspend
 * the producer coroutine `P` until the consumer coroutine `Q` catches up.
 * The [capacity] parameter defines the size of this buffer.
 *
 * ### Buffer overflow
 *
 * By default, the emitter is suspended when the buffer overflows, to let collector catch up. This strategy can be
 * overridden with an optional [onBufferOverflow] parameter so that the emitter is never suspended. In this
 * case, on buffer overflow either the oldest value in the buffer is dropped with the [DROP_OLDEST][BufferOverflow.DROP_OLDEST]
 * strategy and the latest emitted value is added to the buffer,
 * or the latest value that is being emitted is dropped with the [DROP_LATEST][BufferOverflow.DROP_LATEST] strategy,
 * keeping the buffer intact.
 * To implement either of the custom strategies, a buffer of at least one element is used.
 *
 * ### Operator fusion
 *
 * Adjacent applications of [channelFlow], [flowOn], [buffer], and [produceIn] are
 * always fused so that only one properly configured channel is used for execution.
 *
 * Explicitly specified buffer capacity takes precedence over `buffer()` or `buffer(Channel.BUFFERED)` calls,
 * which effectively requests a buffer of any size. Multiple requests with a specified buffer
 * size produce a buffer with the sum of the requested buffer sizes.
 *
 * A `buffer` call with a non-[SUSPEND] value of the [onBufferOverflow] parameter overrides all immediately preceding
 * buffering operators, because it never suspends its upstream, and thus no upstream buffer would ever be used.
 *
 * ### Conceptual implementation
 *
 * The actual implementation of `buffer` is not trivial due to the fusing, but conceptually its basic
 * implementation is equivalent to the following code that can be written using [produce]
 * coroutine builder to produce a channel and [consumeEach][ReceiveChannel.consumeEach] extension to consume it:
 *
 * ```
 * fun <T> Flow<T>.buffer(capacity: Int = DEFAULT): Flow<T> = flow {
 *     coroutineScope { // limit the scope of concurrent producer coroutine
 *         val channel = produce(capacity = capacity) {
 *             collect { send(it) } // send all to channel
 *         }
 *         // emit all received values
 *         channel.consumeEach { emit(it) }
 *     }
 * }
 * ```
 *
 * ### Conflation
 *
 * Usage of this function with [capacity] of [Channel.CONFLATED][Channel.CONFLATED] is a shortcut to
 * `buffer(capacity = 0, onBufferOverflow = `[`BufferOverflow.DROP_OLDEST`][BufferOverflow.DROP_OLDEST]`)`,
 * and is available via a separate [conflate] operator.
 *
 * @param capacity type/capacity of the buffer between coroutines. Allowed values are the same as in `Channel(...)`
 *   factory function: [BUFFERED][Channel.BUFFERED] (by default), [CONFLATED][Channel.CONFLATED],
 *   [RENDEZVOUS][Channel.RENDEZVOUS], [UNLIMITED][Channel.UNLIMITED] or a non-negative value indicating
 *   an explicitly requested size.
 * @param onBufferOverflow configures an action on buffer overflow (optional, defaults to
 *   [SUSPEND][BufferOverflow.SUSPEND], supported only when `capacity >= 0` or `capacity == Channel.BUFFERED`,
 *   implicitly creates a channel with at least one buffered element).
 */
// @Suppress("NAME_SHADOWING")
template<typename T>
Flow<T> buffer(Flow<T> flow, int capacity = BUFFERED, BufferOverflow onBufferOverflow = BufferOverflow::SUSPEND) {
    // require(capacity >= 0 || capacity == BUFFERED || capacity == CONFLATED)
    if (!(capacity >= 0 || capacity == BUFFERED || capacity == CONFLATED)) {
        throw std::invalid_argument("Buffer size should be non-negative, BUFFERED, or CONFLATED, but was " + std::to_string(capacity));
    }
    // require(capacity != CONFLATED || onBufferOverflow == BufferOverflow.SUSPEND)
    if (!(capacity != CONFLATED || onBufferOverflow == BufferOverflow::SUSPEND)) {
        throw std::invalid_argument("CONFLATED capacity cannot be used with non-default onBufferOverflow");
    }
    // desugar CONFLATED capacity to (0, DROP_OLDEST)
    int capacity_local = capacity;
    BufferOverflow onBufferOverflow_local = onBufferOverflow;
    if (capacity_local == CONFLATED) {
        capacity_local = 0;
        onBufferOverflow_local = BufferOverflow::DROP_OLDEST;
    }
    // create a flow
    // return when (this)
    // TODO: implement dynamic dispatch/type checking
    if (auto* fusible = dynamic_cast<FusibleFlow<T>*>(&flow)) {
        return fusible->fuse(capacity_local, onBufferOverflow_local);
    } else {
        return ChannelFlowOperatorImpl<T>(flow, capacity_local, onBufferOverflow_local);
    }
}

// @Deprecated(level = DeprecationLevel.HIDDEN, message = "Since 1.4.0, binary compatibility with earlier versions")
template<typename T>
Flow<T> buffer(Flow<T> flow, int capacity = BUFFERED) {
    return buffer(flow, capacity);
}

/**
 * Conflates flow emissions via conflated channel and runs collector in a separate coroutine.
 * The effect of this is that emitter is never suspended due to a slow collector, but collector
 * always gets the most recent value emitted.
 *
 * This is a shortcut for `buffer(capacity = 0, onBufferOverflow = BufferOverflow.DROP_OLDEST)`.
 * See the [buffer] operator for other configuration options.
 *
 * For example, consider the flow that emits integers from 1 to 30 with 100 ms delay between them:
 *
 * ```
 * val flow = flow {
 *     for (i in 1..30) {
 *         delay(100)
 *         emit(i)
 *     }
 * }
 * ```
 *
 * Applying `conflate()` operator to it allows a collector that delays 1 second on each element to get
 * integers 1, 10, 20, 30:
 *
 * ```
 * val result = flow.conflate().onEach { delay(1000) }.toList()
 * assertEquals(listOf(1, 10, 20, 30), result)
 * ```
 *
 * Note that `conflate` operator is a shortcut for [buffer] with `capacity` of [Channel.CONFLATED][Channel.CONFLATED],
 * which is, in turn, a shortcut to a buffer that only keeps the latest element as
 * created by `buffer(onBufferOverflow = `[`BufferOverflow.DROP_OLDEST`][BufferOverflow.DROP_OLDEST]`)`.
 *
 * ### Operator fusion
 *
 * Adjacent applications of `conflate`/[buffer], [channelFlow], [flowOn] and [produceIn] are
 * always fused so that only one properly configured channel is used for execution.
 *
 * If there was no explicit buffer size specified, then the buffer size is `0`.
 * Otherwise, the buffer size is unchanged.
 * The strategy for buffer overflow becomes [BufferOverflow.DROP_OLDEST] after the application of this operator,
 * but can be overridden later.
 *
 * Note that any instance of [StateFlow] already behaves as if `conflate` operator is
 * applied to it, so applying `conflate` to a `StateFlow` has no effect.
 * See [StateFlow] documentation on Operator Fusion.
 */
template<typename T>
Flow<T> conflate(Flow<T> flow) {
    return buffer(flow, CONFLATED);
}

/**
 * Changes the context where this flow is executed to the given [context].
 * This operator is composable and affects only preceding operators that do not have its own context.
 * This operator is context preserving: [context] **does not** leak into the downstream flow.
 *
 * For example:
 *
 * ```
 * withContext(Dispatchers.Main) {
 *     val singleValue = intFlow // will be executed on IO if context wasn't specified before
 *         .map { ... } // Will be executed in IO
 *         .flowOn(Dispatchers.IO)
 *         .filter { ... } // Will be executed in Default
 *         .flowOn(Dispatchers.Default)
 *         .single() // Will be executed in the Main
 * }
 * ```
 *
 * For more explanation of context preservation please refer to [Flow] documentation.
 *
 * This operator retains a _sequential_ nature of flow if changing the context does not call for changing
 * the [dispatcher][CoroutineDispatcher]. Otherwise, if changing dispatcher is required, it collects
 * flow emissions in one coroutine that is run using a specified [context] and emits them from another coroutines
 * with the original collector's context using a channel with a [default][Channel.BUFFERED] buffer size
 * between two coroutines similarly to [buffer] operator, unless [buffer] operator is explicitly called
 * before or after `flowOn`, which requests buffering behavior and specifies channel size.
 *
 * Note, that flows operating across different dispatchers might lose some in-flight elements when cancelled.
 * In particular, this operator ensures that downstream flow does not resume on cancellation even if the element
 * was already emitted by the upstream flow.
 *
 * ### Operator fusion
 *
 * Adjacent applications of [channelFlow], [flowOn], [buffer], and [produceIn] are
 * always fused so that only one properly configured channel is used for execution.
 *
 * Multiple `flowOn` operators fuse to a single `flowOn` with a combined context. The elements of the context of
 * the first `flowOn` operator naturally take precedence over the elements of the second `flowOn` operator
 * when they have the same context keys, for example:
 *
 * ```
 * flow.map { ... } // Will be executed in IO
 *     .flowOn(Dispatchers.IO) // This one takes precedence
 *     .flowOn(Dispatchers.Default)
 * ```
 *
 * Note that an instance of [SharedFlow] does not have an execution context by itself,
 * so applying `flowOn` to a `SharedFlow` has not effect. See the [SharedFlow] documentation on Operator Fusion.
 *
 * @throws [IllegalArgumentException] if provided context contains [Job] instance.
 */
template<typename T>
Flow<T> flow_on(Flow<T> flow, CoroutineContext context) {
    check_flow_context(context);
    // return when
    if (context == EmptyCoroutineContext) {
        return flow;
    }
    if (auto* fusible = dynamic_cast<FusibleFlow<T>*>(&flow)) {
        return fusible->fuse(context);
    } else {
        return ChannelFlowOperatorImpl<T>(flow, context);
    }
}

/**
 * Returns a flow which checks cancellation status on each emission and throws
 * the corresponding cancellation cause if flow collector was cancelled.
 * Note that [flow] builder and all implementations of [SharedFlow] are [cancellable] by default.
 *
 * This operator provides a shortcut for `.onEach { currentCoroutineContext().ensureActive() }`.
 * See [ensureActive][CoroutineContext.ensureActive] for details.
 */
template<typename T>
Flow<T> cancellable(Flow<T> flow) {
    // return when (this)
    if (dynamic_cast<CancellableFlow<T>*>(&flow)) {
        return flow; // Fast-path, already cancellable
    } else {
        return CancellableFlowImpl<T>(flow);
    }
}

/**
 * Internal marker for flows that are [cancellable].
 */
template<typename T>
class CancellableFlow : public Flow<T> {
    // Marker interface
};

/**
 * Named implementation class for a flow that is defined by the [cancellable] function.
 */
template<typename T>
class CancellableFlowImpl : public CancellableFlow<T> {
private:
    Flow<T> flow;

public:
    CancellableFlowImpl(Flow<T> flow_) : flow(flow_) {}

    // TODO: suspend function
    void collect(FlowCollector<T> collector) /* override */ {
        flow.collect([&](T it) {
            current_coroutine_context().ensure_active();
            collector.emit(it);
        });
    }
};

void check_flow_context(CoroutineContext context) {
    // require(context[Job] == null)
    if (context[Job] != nullptr) {
        throw std::invalid_argument("Flow context cannot contain job in it. Had " + to_string(context));
    }
}

} // namespace flow
} // namespace coroutines
} // namespace kotlinx
