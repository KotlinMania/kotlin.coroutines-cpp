// port-lint: source flow/operators/Context.kt
/**
 * @file Context.hpp
 * @brief Flow context operators: buffer, conflate, flowOn, cancellable
 *
 * Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Context.kt
 *
 * These operators control how flows are executed, including buffering behavior,
 * context switching, and cancellation checking.
 */
#pragma once
#include "kotlinx/coroutines/flow/Flow.hpp"
#include "kotlinx/coroutines/CoroutineContext.hpp"
#include "kotlinx/coroutines/channels/BufferOverflow.hpp"
#include "kotlinx/coroutines/Job.hpp"

namespace kotlinx {
namespace coroutines {
namespace flow {

using channels::BufferOverflow;

// CancellableFlow is defined in Flow.hpp

/**
 * Implementation of cancellable() operator.
 *
 * Wraps a flow to check cancellation status on each emission.
 *
 * Transliterated from:
 * private class CancellableFlowImpl<T>(private val flow: Flow<T>) : CancellableFlow<T>
 */
template<typename T>
class CancellableFlowImpl : public CancellableFlow<T> {
public:
    explicit CancellableFlowImpl(Flow<T>* upstream) : upstream_(upstream) {}

    void* collect(FlowCollector<T>* collector, Continuation<void*>* continuation) override {
        // TODO(suspend-plugin): Properly implement with ensureActive() check
        // For now, delegate to upstream - cancellation check not yet implemented
        return upstream_->collect(collector, continuation);
    }

private:
    Flow<T>* upstream_;
};

/**
 * Buffers flow emissions via channel of a specified capacity and runs collector in a separate coroutine.
 *
 * Normally, flows are _sequential_. It means that the code of all operators is executed in the
 * same coroutine. For example, consider the following code using on_each and collect operators:
 *
 * ```cpp
 * flow_of("A", "B", "C")
 *     | on_each([](auto s) { std::cout << "1" << s; })
 *     | collect([](auto s) { std::cout << "2" << s; });
 * ```
 *
 * It is going to be executed in the following order by the coroutine Q that calls this code:
 *
 * ```
 * Q : -->-- [1A] -- [2A] -- [1B] -- [2B] -- [1C] -- [2C] -->--
 * ```
 *
 * So if the operator's code takes considerable time to execute, then the total execution time is going to be
 * the sum of execution times for all operators.
 *
 * The buffer operator creates a separate coroutine during execution for the flow it applies to.
 * Consider the following code:
 *
 * ```cpp
 * flow_of("A", "B", "C")
 *     | on_each([](auto s) { std::cout << "1" << s; })
 *     | buffer()  // <--------------- buffer between on_each and collect
 *     | collect([](auto s) { std::cout << "2" << s; });
 * ```
 *
 * It will use two coroutines for execution of the code. A coroutine Q that calls this code is
 * going to execute collect, and the code before buffer will be executed in a separate
 * new coroutine P concurrently with Q:
 *
 * ```
 * P : -->-- [1A] -- [1B] -- [1C] ---------->--  // flow_of(...) | on_each(...)
 *
 *                       |
 *                       | channel               // buffer()
 *                       V
 *
 * Q : -->---------- [2A] -- [2B] -- [2C] -->--  // collect
 * ```
 *
 * When the operator's code takes some time to execute, this decreases the total execution time of the flow.
 * A channel is used between the coroutines to send elements emitted by the coroutine P to the coroutine Q.
 * If the code before buffer operator (in the coroutine P) is faster than the code after buffer operator
 * (in the coroutine Q), then this channel will become full at some point and will suspend the producer
 * coroutine P until the consumer coroutine Q catches up. The capacity parameter defines the size of this buffer.
 *
 * ### Buffer overflow
 *
 * By default, the emitter is suspended when the buffer overflows, to let collector catch up. This strategy can be
 * overridden with an optional on_buffer_overflow parameter so that the emitter is never suspended. In this case,
 * on buffer overflow either the oldest value in the buffer is dropped with the DROP_OLDEST strategy and the latest
 * emitted value is added to the buffer, or the latest value that is being emitted is dropped with the DROP_LATEST
 * strategy, keeping the buffer intact. To implement either of the custom strategies, a buffer of at least one
 * element is used.
 *
 * ### Operator fusion
 *
 * Adjacent applications of channel_flow, flow_on, buffer, and produce_in are always fused so that only one
 * properly configured channel is used for execution.
 *
 * @param capacity type/capacity of the buffer between coroutines. Allowed values are the same as in Channel(...)
 *   factory function: BUFFERED (default), CONFLATED, RENDEZVOUS, UNLIMITED or a non-negative value indicating
 *   an explicitly requested size.
 * @param on_buffer_overflow configures an action on buffer overflow (defaults to SUSPEND, supported only when
 *   capacity >= 0 or capacity == BUFFERED, implicitly creates a channel with at least one buffered element).
 *
 * Transliterated from:
 * public fun <T> Flow<T>.buffer(capacity: Int = BUFFERED, onBufferOverflow: BufferOverflow = BufferOverflow.SUSPEND): Flow<T>
 */
template<typename T>
Flow<T>* buffer(Flow<T>* flow, int capacity = -1, BufferOverflow onBufferOverflow = BufferOverflow::SUSPEND) {
    // TODO(port): Implement Fusion or ChannelFlow wrapper
    return flow;
}

/**
 * Conflates flow emissions via conflated channel and runs collector in a separate coroutine.
 * The effect of this is that emitter is never suspended due to a slow collector, but collector
 * always gets the most recent value emitted.
 *
 * This is a shortcut for `buffer(capacity = 0, on_buffer_overflow = BufferOverflow::DROP_OLDEST)`.
 * See the buffer operator for other configuration options.
 *
 * For example, consider the flow that emits integers from 1 to 30 with 100 ms delay between them:
 *
 * ```cpp
 * auto flow = flow_builder<int>([](FlowCollector<int>& emit) {
 *     for (int i = 1; i <= 30; ++i) {
 *         delay(100);
 *         emit(i);
 *     }
 * });
 * ```
 *
 * Applying conflate() operator to it allows a collector that delays 1 second on each element to get
 * integers 1, 10, 20, 30:
 *
 * ```cpp
 * auto result = flow | conflate() | on_each([](int) { delay(1000); }) | to_list();
 * // result == {1, 10, 20, 30}
 * ```
 *
 * Note that conflate operator is a shortcut for buffer with capacity of CONFLATED,
 * which is, in turn, a shortcut to a buffer that only keeps the latest element as
 * created by `buffer(on_buffer_overflow = BufferOverflow::DROP_OLDEST)`.
 *
 * ### Operator fusion
 *
 * Adjacent applications of conflate/buffer, channel_flow, flow_on and produce_in are
 * always fused so that only one properly configured channel is used for execution.
 *
 * Note that any instance of StateFlow already behaves as if conflate operator is
 * applied to it, so applying conflate to a StateFlow has no effect.
 * See StateFlow documentation on Operator Fusion.
 *
 * Transliterated from:
 * public fun <T> Flow<T>.conflate(): Flow<T> = buffer(CONFLATED)
 */
template<typename T>
Flow<T>* conflate(Flow<T>* flow) {
    // TODO(port): Implement conflate
    return buffer(flow, 0, BufferOverflow::DROP_OLDEST);
}

/**
 * Changes the context where this flow is executed to the given context.
 * This operator is composable and affects only preceding operators that do not have their own context.
 * This operator is context preserving: context **does not** leak into the downstream flow.
 *
 * For example:
 *
 * ```cpp
 * with_context(Dispatchers::Main, [&]() {
 *     auto single_value = int_flow // will be executed on IO if context wasn't specified before
 *         | map([](int v) { ... }) // Will be executed in IO
 *         | flow_on(Dispatchers::IO)
 *         | filter([](int v) { ... }) // Will be executed in Default
 *         | flow_on(Dispatchers::Default)
 *         | single(); // Will be executed in the Main
 * });
 * ```
 *
 * For more explanation of context preservation please refer to Flow documentation.
 *
 * This operator retains a _sequential_ nature of flow if changing the context does not call for changing
 * the dispatcher. Otherwise, if changing dispatcher is required, it collects flow emissions in one coroutine
 * that is run using a specified context and emits them from another coroutine with the original collector's
 * context using a channel with a default buffer size between two coroutines similarly to buffer operator,
 * unless buffer operator is explicitly called before or after flow_on, which requests buffering behavior
 * and specifies channel size.
 *
 * Note, that flows operating across different dispatchers might lose some in-flight elements when cancelled.
 * In particular, this operator ensures that downstream flow does not resume on cancellation even if the element
 * was already emitted by the upstream flow.
 *
 * ### Operator fusion
 *
 * Adjacent applications of channel_flow, flow_on, buffer, and produce_in are always fused so that only one
 * properly configured channel is used for execution.
 *
 * Multiple flow_on operators fuse to a single flow_on with a combined context. The elements of the context of
 * the first flow_on operator naturally take precedence over the elements of the second flow_on operator
 * when they have the same context keys, for example:
 *
 * ```cpp
 * flow | map([](int v) { ... }) // Will be executed in IO
 *      | flow_on(Dispatchers::IO) // This one takes precedence
 *      | flow_on(Dispatchers::Default);
 * ```
 *
 * Note that an instance of SharedFlow does not have an execution context by itself,
 * so applying flow_on to a SharedFlow has no effect. See the SharedFlow documentation on Operator Fusion.
 *
 * @throws std::invalid_argument if provided context contains Job instance.
 *
 * Transliterated from:
 * public fun <T> Flow<T>.flowOn(context: CoroutineContext): Flow<T>
 */
template<typename T>
Flow<T>* flow_on(Flow<T>* flow, std::shared_ptr<CoroutineContext> context) {
    // TODO(port): Implement flow_on (ChannelFlow operator)
    return flow;
}

/**
 * Returns a flow which checks cancellation status on each emission and throws
 * the corresponding cancellation cause if flow collector was cancelled.
 * Note that flow builder and all implementations of SharedFlow are cancellable by default.
 *
 * This operator provides a shortcut for `.on_each([](auto) { current_coroutine_context().ensure_active(); })`.
 * See ensure_active for details.
 *
 * If the flow already implements CancellableFlow (like AbstractFlow-based flows),
 * this is a no-op. Otherwise, wraps in CancellableFlowImpl.
 *
 * Transliterated from:
 * public fun <T> Flow<T>.cancellable(): Flow<T>
 */
template<typename T>
Flow<T>* cancellable(Flow<T>* flow) {
    // Fast-path: already cancellable
    if (dynamic_cast<CancellableFlow<T>*>(flow) != nullptr) {
        return flow;
    }
    // Wrap in CancellableFlowImpl
    return new CancellableFlowImpl<T>(flow);
}

} // namespace flow
} // namespace coroutines
} // namespace kotlinx
