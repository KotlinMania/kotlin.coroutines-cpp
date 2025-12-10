#pragma once
#include "kotlinx/coroutines/flow/FlowCollector.hpp"

namespace kotlinx {
namespace coroutines {
namespace flow {

/**
 * An asynchronous data stream that sequentially emits values and completes normally or with an exception.
 *
 * _Intermediate operators_ on the flow such as [map], [filter], [take], [zip], etc are functions that are
 * applied to the _upstream_ flow or flows and return a _downstream_ flow where further operators can be applied to.
 * Intermediate operations do not execute any code in the flow and are not suspending functions themselves.
 * They only set up a chain of operations for future execution and quickly return.
 * This is known as a _cold flow_ property.
 *
 * _Terminal operators_ on the flow are either suspending functions such as [collect], [single], [reduce], [toList], etc.
 * or [launchIn] operator that starts collection of the flow in the given scope.
 * They are applied to the upstream flow and trigger execution of all operations.
 * Execution of the flow is also called _collecting the flow_  and is always performed in a suspending manner
 * without actual blocking. Terminal operators complete normally or exceptionally depending on successful or failed
 * execution of all the flow operations in the upstream. The most basic terminal operator is [collect], for example:
 *
 * ```cpp
 * try {
 *     flow->collect(value, [](auto value) {
 *         std::cout << "Received " << value << std::endl;
 *     });
 * } catch (const std::exception& e) {
 *     std::cout << "The flow has thrown an exception: " << e.what() << std::endl;
 * }
 * ```
 *
 * By default, flows are _sequential_ and all flow operations are executed sequentially in the same coroutine,
 * with an exception for a few operations specifically designed to introduce concurrency into flow
 * execution such as [buffer] and [flatMapMerge]. See their documentation for details.
 *
 * The `Flow` interface does not carry information whether a flow is a _cold_ stream that can be collected repeatedly and
 * triggers execution of the same code every time it is collected, or if it is a _hot_ stream that emits different
 * values from the same running source on each collection. Usually flows represent _cold_ streams, but
 * there is a [SharedFlow] subtype that represents _hot_ streams. In addition to that, any flow can be turned
 * into a _hot_ one by the [stateIn] and [shareIn] operators, or by converting the flow into a hot channel
 * via the [produceIn] operator.
 *
 * ### Flow constraints
 *
 * All implementations of the `Flow` interface must adhere to two key properties described in detail below:
 *
 * - Context preservation.
 * - Exception transparency.
 *
 * These properties ensure the ability to perform local reasoning about the code with flows and modularize the code
 * in such a way that upstream flow emitters can be developed separately from downstream flow collectors.
 * A user of a flow does not need to be aware of implementation details of the upstream flows it uses.
 *
 * ### Context preservation
 *
 * The flow has a context preservation property: it encapsulates its own execution context and never propagates or leaks
 * it downstream, thus making reasoning about the execution context of particular transformations or terminal
 * operations trivial.
 *
 * There is only one way to change the context of a flow: the [flowOn] operator
 * that changes the upstream context ("everything above the `flowOn` operator").
 * For additional information refer to its documentation.
 *
 * ### Exception transparency
 *
 * When `emit` or `emitAll` throws, the Flow implementations must immediately stop emitting new values and finish with an exception.
 * For diagnostics or application-specific purposes, the exception may be different from the one thrown by the emit operation,
 * suppressing the original exception as discussed below.
 * If there is a need to emit values after the downstream failed, please use the [catch] operator.
 *
 * The [catch] operator only catches upstream exceptions, but passes
 * all downstream exceptions. Similarly, terminal operators like [collect]
 * throw any unhandled exceptions that occur in their code or in upstream flows.
 *
 * Flow machinery enforces exception transparency at runtime and throws [IllegalStateException] on any attempt to emit a value,
 * if an exception has been thrown on previous attempt.
 *
 * ### Not stable for inheritance
 *
 * **The `Flow` interface is not stable for inheritance in 3rd party libraries**, as new methods
 * might be added to this interface in the future, but is stable for use.
 *
 * Use the `flow { ... }` builder function to create an implementation, or extend [AbstractFlow].
 * These implementations ensure that the context preservation property is not violated, and prevent most
 * of the developer mistakes related to concurrency, inconsistent flow dispatchers, and cancellation.
 */
template <typename T>
struct Flow {
    virtual ~Flow() = default;

    /**
     * Accepts the given [collector] and [emits][FlowCollector.emit] values into it.
     *
     * This method can be used along with SAM-conversion of [FlowCollector]:
     * ```cpp
     * myFlow.collect([](auto value) { std::cout << "Collected " << value << std::endl; });
     * ```
     *
     * ### Method inheritance
     *
     * To ensure the context preservation property, it is not recommended implementing this method directly.
     * Instead, [AbstractFlow] can be used as the base type to properly ensure flow's properties.
     *
     * All default flow implementations ensure context preservation and exception transparency properties on a best-effort basis
     * and throw [IllegalStateException] if a violation was detected.
     */
    virtual void collect(FlowCollector<T>* collector) = 0;
};

/**
 * Base class for stateful implementations of `Flow`.
 * It tracks all the properties required for context preservation and throws an [IllegalStateException]
 * if any of the properties are violated.
 * 
 * Example of the implementation (C++):
 *
 * ```cpp
 * // list.asFlow() + collect counter
 * class CountingListFlow : public AbstractFlow<int> {
 *     std::vector<int> values;
 *     std::atomic<int> collectedCounter;
 * public:
 *     CountingListFlow(std::vector<int> v) : values(v), collectedCounter(0) {}
 *
 *     void collectSafely(FlowCollector<int>* collector) override {
 *         collectedCounter++; // Increment collected counter
 *         for (auto& it : values) { // Emit all the values
 *             collector->emit(it);
 *         }
 *     }
 *
 *     std::string toDiagnosticString() { 
 *         return "Flow with values " + std::to_string(values.size()) + 
 *                " was collected " + std::to_string(collectedCounter.load()) + " times";
 *     }
 * };
 * ```
 */
template<typename T>
class AbstractFlow : public Flow<T> {
public:
    void collect(FlowCollector<T>* collector) override {
        // TODO: SafeCollector implementation to ensure context preservation
        // SafeCollector<T> safeCollector(collector, coroutineContext);
        // try {
        //     collectSafely(&safeCollector);
        // } catch (...) {
        //     safeCollector.releaseIntercepted();
        //     throw;
        // }
        // safeCollector.releaseIntercepted();
        collect_safely(collector);
    }

    /**
     * Accepts the given [collector] and [emits][FlowCollector.emit] values into it.
     *
     * A valid implementation of this method has the following constraints:
     * 1) It should not change the coroutine context when emitting values.
     *    The emission should happen in the context of the [collect] call.
     *    Please refer to the top-level [Flow] documentation for more details.
     * 2) It should serialize calls to [emit][FlowCollector.emit] as [FlowCollector] implementations are not
     *    thread-safe by default.
     *    To automatically serialize emissions [channelFlow] builder can be used instead of [flow]
     *
     * @throws IllegalStateException if any of the invariants are violated.
     */
    virtual void collect_safely(FlowCollector<T>* collector) = 0;
};

} // namespace flow
} // namespace coroutines
} // namespace kotlinx
