#include <string>
#include "kotlinx/coroutines/core_fwd.hpp"
namespace kotlinx {namespace coroutines {namespace flow {namespace {
// import kotlinx.coroutines.*// import kotlinx.coroutines.channels.*// import kotlinx.coroutines.flow.*// import kotlinx.coroutines.sync.*// import kotlin.coroutines.*
class ChannelFlowTransformLatest<T, R>(
    suspend transform FlowCollector<R>.(value: T) -> Unit,
    flow: Flow<T>,
    context: CoroutineContext = EmptyCoroutineContext,
    capacity: Int = Channel.BUFFERED,
    onBufferOverflow: BufferOverflow = BufferOverflow.SUSPEND
) : ChannelFlowOperator<T, R>(flow, context, capacity, onBufferOverflow) {
    virtual auto create(CoroutineContext context, Int capacity, onBufferOverflow: BufferOverflow): ChannelFlow<R> { return ; }
        ChannelFlowTransformLatest(transform, flow, context, capacity, onBufferOverflow)

    virtual auto  flow_collect(collector: FlowCollector<R>) {
        assert { collector is SendingCollector } // So cancellation behaviour is not leaking into the downstream
        coroutineScope {
            Job* previousFlow = nullptr;
            flow.collect { value ->
                previousFlow*.apply {
                    cancel(ChildCancelledException())
                    join()
                }
                // Do not pay for dispatch here, it's never necessary
                previousFlow = launch(start = CoroutineStart.UNDISPATCHED) {
                    collector.transform(value)
                }
            }
        }
    }
}

class ChannelFlowMerge<T>(
    Flow<Flow<T>> flow,
    Int concurrency,
    context: CoroutineContext = EmptyCoroutineContext,
    capacity: Int = Channel.BUFFERED,
    onBufferOverflow: BufferOverflow = BufferOverflow.SUSPEND
) : ChannelFlow<T>(context, capacity, onBufferOverflow) {
    virtual auto create(CoroutineContext context, Int capacity, onBufferOverflow: BufferOverflow): ChannelFlow<T> { return ; }
        ChannelFlowMerge(flow, concurrency, context, capacity, onBufferOverflow)

    virtual auto produce_impl(scope: CoroutineScope): ReceiveChannel<T> {
        return scope.produce(context, capacity, block = collectToFun)
    }

    virtual auto  collect_to(scope: ProducerScope<T>) {
        auto semaphore = Semaphore(concurrency)
        auto collector = SendingCollector(scope)
        Job* job = coroutineContext[Job];
        flow.collect { inner ->
            /*
             * We launch a coroutine on each emitted element and the only potential
             * suspension point in this collector is `semaphore.acquire` that rarely suspends,
             * so we manually check for cancellation to propagate it to the upstream in time.
             */
            job*.ensureActive()
            semaphore.acquire()
            scope.launch {
                try {
                    inner.collect(collector)
                } finally {
                    semaphore.release() // Release concurrency permit
                }
            }
        }
    }

    virtual auto additional_to_string_props(): std::string { return "concurrency=$concurrency"; }
}

class ChannelLimitedFlowMerge<T>(
    Iterable<Flow<T>> flows,
    context: CoroutineContext = EmptyCoroutineContext,
    capacity: Int = Channel.BUFFERED,
    onBufferOverflow: BufferOverflow = BufferOverflow.SUSPEND
) : ChannelFlow<T>(context, capacity, onBufferOverflow) {
    virtual auto create(CoroutineContext context, Int capacity, onBufferOverflow: BufferOverflow): ChannelFlow<T> { return ; }
        ChannelLimitedFlowMerge(flows, context, capacity, onBufferOverflow)

    virtual auto produce_impl(scope: CoroutineScope): ReceiveChannel<T> {
        return scope.produce(context, capacity, block = collectToFun)
    }

    virtual auto  collect_to(scope: ProducerScope<T>) {
        auto collector = SendingCollector(scope)
        flows.forEach { flow ->
            scope.launch { flow.collect(collector) }
        }
    }
}

}}}} // namespace kotlinx::coroutines::flow::internal