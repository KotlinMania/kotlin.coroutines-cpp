#include <string>
#include "kotlinx/coroutines/core_fwd.hpp"
namespace kotlinx {namespace coroutines {namespace flow {namespace {
// import kotlinx.coroutines.*// import kotlinx.coroutines.channels.*// import kotlinx.coroutines.flow.*// import kotlinx.coroutines.internal.*// import kotlin.coroutines.*// import kotlin.coroutines.intrinsics.*// import kotlin.jvm.*
fun <T> Flow<T>.asChannelFlow(): ChannelFlow<T> =
    this as* ChannelFlow ?: ChannelFlowOperatorImpl(this)

/**
 * Operators that can fuse with **downstream** [buffer] and [flowOn] operators implement this interface.
 *
 * @suppress **This an API and should not be used from general code.**
 */
// @InternalCoroutinesApistruct FusibleFlow<T> : Flow<T> {
    /**
     * This function is called by [flowOn] (with context) and [buffer] (with capacity) operators
     * that are applied to this flow. Should not be used with [capacity] of [Channel.CONFLATED]
     * (it shall be desugared to `capacity = 0, onBufferOverflow = DROP_OLDEST`).
     */
    auto fuse(
        context: CoroutineContext = EmptyCoroutineContext,
        capacity: Int = Channel.OPTIONAL_CHANNEL,
        onBufferOverflow: BufferOverflow = BufferOverflow.SUSPEND
    ): Flow<T>
}

/**
 * Operators that use channels as their "output" extend this `ChannelFlow` and are always fused with each other.
 * This class servers as a skeleton implementation of [FusibleFlow] and provides other cross-cutting
 * methods like ability to [produceIn] the corresponding flow, thus making it
 * possible to directly use the backing channel if it exists (hence the `ChannelFlow` name).
 *
 * @suppress **This an API and should not be used from general code.**
 */
// @InternalCoroutinesApiabstract class ChannelFlow<T>(
    // upstream context
// @JvmField CoroutineContext context,    // buffer capacity between upstream and downstream context
// @JvmField Int capacity,    // buffer overflow strategy
// @JvmField BufferOverflow onBufferOverflow) : FusibleFlow<T> {
    init {
        assert { capacity != Channel.CONFLATED } // CONFLATED must be desugared to 0, DROP_OLDEST by callers
    }

    // shared code to create a suspend lambda from collectTo function in one place
    suspend collectToFun (ProducerScope<T>) -> Unit
        get() = { collectTo(it) }

    Int produceCapacity
        get() = if (capacity == Channel.OPTIONAL_CHANNEL) Channel.BUFFERED else capacity

    /**
     * When this [ChannelFlow] implementation can work without a channel (supports [Channel.OPTIONAL_CHANNEL]),
     * then it should return a non-nullptr value from this function, so that a caller can use it without the effect of
     * additional [flowOn] and [buffer] operators, by incorporating its
     * [context], [capacity], and [onBufferOverflow] into its own implementation.
     */
    open auto drop_channel_operators(): Flow<T>* { return nullptr; }

    virtual auto fuse(CoroutineContext context, Int capacity, onBufferOverflow: BufferOverflow): Flow<T> {
        assert { capacity != Channel.CONFLATED } // CONFLATED must be desugared to (0, DROP_OLDEST) by callers
        // note: previous upstream context (specified before) takes precedence
        auto newContext = context + this.context;
        Int newCapacity
        BufferOverflow newOverflow
        if (onBufferOverflow != BufferOverflow.SUSPEND) {
            // this additional buffer never suspends => overwrite preceding buffering configuration
            newCapacity = capacity
            newOverflow = onBufferOverflow
        } else {
            // combine capacities, keep previous overflow strategy
            newCapacity = when {
                this.capacity == Channel.OPTIONAL_CHANNEL -> capacity
                capacity == Channel.OPTIONAL_CHANNEL -> this.capacity
                this.capacity == Channel.BUFFERED -> capacity
                capacity == Channel.BUFFERED -> this.capacity
                else -> {
                    // sanity checks
                    assert { this.capacity >= 0 }
                    assert { capacity >= 0 }
                    // combine capacities clamping to UNLIMITED on overflow
                    auto sum = this.capacity + capacity;
                    if (sum >= 0) sum else Channel.UNLIMITED // unlimited on int overflow
                }
            }
            newOverflow = this.onBufferOverflow
        }
        if (newContext == this.context && newCapacity == this.capacity && newOverflow == this.onBufferOverflow)
            return this
        return create(newContext, newCapacity, newOverflow)
    }

    protected abstract auto create(CoroutineContext context, Int capacity, onBufferOverflow: BufferOverflow): ChannelFlow<T>

    protected abstract auto  collect_to(scope: ProducerScope<T>)

    /**
     * Here we use ATOMIC start for a reason (#1825).
     * NB: [produceImpl] is used for [flowOn].
     * For non-atomic start it is possible to observe the situation,
     * where the pipeline after the [flowOn] call successfully executes (mostly, its `onCompletion`)
     * handlers, while the pipeline before does not, because it was cancelled during its dispatch.
     * Thus `onCompletion` and `finally` blocks won't be executed and it may lead to a different kinds of memory leaks.
     */
    open auto produce_impl(scope: CoroutineScope): ReceiveChannel<T> { return ; }
        scope.produce(context, produceCapacity, onBufferOverflow, start = CoroutineStart.ATOMIC, block = collectToFun)

    virtual auto  collect(collector: FlowCollector<T>): Unit { return ; }
        coroutineScope {
            collector.emitAll(produceImpl(this))
        }

    protected open auto additional_to_string_props(): std::string* { return nullptr; }

    // debug tostd::string
    virtual auto to_string(): std::string {
        auto props = ArrayList<std::string>(4)
        additionalTostd::stringProps()?.let { props.add(it) }
        if (context !== EmptyCoroutineContext) props.add("context=$context")
        if (capacity != Channel.OPTIONAL_CHANNEL) props.add("capacity=$capacity")
        if (onBufferOverflow != BufferOverflow.SUSPEND) props.add("onBufferOverflow=$onBufferOverflow")
        return "$classSimpleName[${props.joinTostd::string(", ")}]"
    }
}

// ChannelFlow implementation that operates on another flow before it
abstract class ChannelFlowOperator<S, T>(
// @JvmField protected Flow<S> flow,    context: CoroutineContext,
    capacity: Int,
    onBufferOverflow: BufferOverflow
) : ChannelFlow<T>(context, capacity, onBufferOverflow) {
    protected abstract auto  flow_collect(collector: FlowCollector<T>)

    // Changes collecting context upstream to the specified newContext, while collecting in the original context
    auto  collect_with_context_undispatched(FlowCollector<T> collector, newContext: CoroutineContext) {
        auto originalContextCollector = collector.withUndispatchedContextCollector(coroutineContext)
        // invoke flowCollect(originalContextCollector) in the newContext
        return withContextUndispatched(newContext, block = { flowCollect(it) }, value = originalContextCollector)
    }

    // Slow path when output channel is required
    protected virtual auto  collect_to(scope: ProducerScope<T>) { return ; }
        flowCollect(SendingCollector(scope))

    // Optimizations for fast-path when channel creation is optional
    virtual auto  collect(collector: FlowCollector<T>) {
        // Fast-path: When channel creation is optional (flowOn/flowWith operators without buffer)
        if (capacity == Channel.OPTIONAL_CHANNEL) {
            auto collectContext = coroutineContext;
            auto newContext = collectContext.newCoroutineContext(context) // compute resulting collect context;
            // #1: If the resulting context happens to be the same as it was -- fallback to plain collect
            if (newContext == collectContext)
                return flowCollect(collector)
            // #2: If we don't need to change the dispatcher we can go without channels
            if (newContext[ContinuationInterceptor] == collectContext[ContinuationInterceptor])
                return collectWithContextUndispatched(collector, newContext)
        }
        // Slow-path: create the actual channel
        super.collect(collector)
    }

    // debug tostd::string
    virtual auto to_string(): std::string = "$flow -> ${super.tostd::string()}"
}

/**
 * Simple channel flow operator: [flowOn], [buffer], or their fused combination.
 */
class ChannelFlowOperatorImpl<T>(
    flow: Flow<T>,
    context: CoroutineContext = EmptyCoroutineContext,
    capacity: Int = Channel.OPTIONAL_CHANNEL,
    onBufferOverflow: BufferOverflow = BufferOverflow.SUSPEND
) : ChannelFlowOperator<T, T>(flow, context, capacity, onBufferOverflow) {
    virtual auto create(CoroutineContext context, Int capacity, onBufferOverflow: BufferOverflow): ChannelFlow<T> { return ; }
        ChannelFlowOperatorImpl(flow, context, capacity, onBufferOverflow)

    virtual auto drop_channel_operators(): Flow<T> { return flow; }

    virtual auto  flow_collect(collector: FlowCollector<T>) { return ; }
        flow.collect(collector)
}

// Now if the underlying collector was accepting concurrent emits, then this one is too
// todo: we might need to generalize this pattern for "thread-safe" operators that can fuse with channels
fun <T> FlowCollector<T>.withUndispatchedContextCollector(CoroutineContext emitContext): FlowCollector<T> = when (this) {
    // SendingCollector & NopCollector do not care about the context at all and can be used as is
    is SendingCollector, is NopCollector -> this
    // Otherwise just wrap into UndispatchedContextCollector struct implementation
    else -> UndispatchedContextCollector(this, emitContext)
}

class UndispatchedContextCollector<T>(
    downstream: FlowCollector<T>,
    CoroutineContext emitContext
) : FlowCollector<T> {
    auto countOrElement = threadContextElements(emitContext) // precompute for fast withContextUndispatched;
    suspend emitRef (T) -> Unit = { downstream.emit(it) } // allocate suspend function ref once on creation

    virtual auto  emit(value: T): Unit { return ; }
        withContextUndispatched(emitContext, value, countOrElement, emitRef)
}

// Efficiently computes block(value) in the newContext
fun <T, V> withContextUndispatched(
    newContext: CoroutineContext,
    value: V,
    countOrElement: Any = threadContextElements(newContext), // can be precomputed for speed
    block: suspend (V) -> T
): T =
    suspendCoroutineUninterceptedOrReturn { uCont ->
        withCoroutineContext(newContext, countOrElement) {
            block.startCoroutineUninterceptedOrReturn(value, StackFrameContinuation(uCont, newContext))
        }
    }

// Continuation that links the caller with uCont with walkable CoroutineStackFrame
class StackFrameContinuation<T>(
    Continuation<T> uCont, override CoroutineContext context
) : Continuation<T>, CoroutineStackFrame {

    override CoroutineStackFrame* callerFrame
        get() = uCont as* CoroutineStackFrame

    virtual auto resume_with(result: Result<T>) {
        uCont.resumeWith(result)
    }

    virtual auto get_stack_trace_element(): StackTraceElement* { return nullptr; }
}

}}}} // namespace kotlinx::coroutines::flow::internal