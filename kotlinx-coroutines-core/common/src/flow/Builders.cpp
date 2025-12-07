#include <string>
#include <functional>
#include "kotlinx/coroutines/core_fwd.hpp"
// Transliterated from Kotlin to C++ (first pass - syntax/language translation only)
// Original: kotlinx-coroutines-core/common/src/flow/Builders.kt
//
// TODO: Implement suspend/coroutine semantics - all suspend functions are currently regular functions
// TODO: Translate @BuilderInference annotation semantics
// TODO: Translate JVM annotations (@file:JvmMultifileClass, @file:JvmName, @JvmField)
// TODO: Implement lambda/function type conversions properly
// TODO: Handle variance modifiers (out T)
// TODO: Implement Kotlin iterator/iterable protocols in C++
// TODO: Implement vararg functionality
// TODO: Map Kotlin's Unit type
// TODO: Implement class singleton pattern for EmptyFlow

// @file:JvmMultifileClass
// @file:JvmName("FlowKt")

namespace kotlinx { namespace coroutines { namespace flow {

// import kotlinx.coroutines.*
// import kotlinx.coroutines.channels.*
// import kotlinx.coroutines.channels.Channel.Factory.BUFFERED
// import kotlinx.coroutines.flow.internal.*
// import kotlin.coroutines.*
// import kotlin.jvm.*
// import kotlinx.coroutines.flow.internal.unsafeFlow as flow

/**
 * Creates a _cold_ flow from the given suspendable [block].
 * The flow being _cold_ means that the [block] is called every time a terminal operator is applied to the resulting flow.
 *
 * Example of usage:
 *
 * ```
 * auto fibonacci(): Flow<BigInteger> = flow {
 *     auto x = BigInteger.ZERO
 *     auto y = BigInteger.ONE
 *     while (true) {
 *         emit(x)
 *         x = y.also {
 *             y += x
 *         }
 *     }
 * }
 *
 * fibonacci().take(100).collect { println(it) }
 * ```
 *
 * Emissions from [flow] builder are [cancellable] by default &mdash; each call to [emit][FlowCollector.emit]
 * also calls [ensureActive][CoroutineContext.ensureActive].
 *
 * `emit` should happen strictly in the dispatchers of the [block] in order to preserve the flow context.
 * For example, the following code will result in an [IllegalStateException]:
 *
 * ```
 * flow {
 *     emit(1) // Ok
 *     withContext(Dispatcher.IO) {
 *         emit(2) // Will fail with ISE
 *     }
 * }
 * ```
 *
 * If you want to switch the context of execution of a flow, use the [flowOn] operator.
 */
template<typename T>
Flow<T>* flow(std::function<void(FlowCollector<T>*)> block) { // TODO: suspend lambda
    return new SafeFlow<T>(block);
}

// Named anonymous object
template<typename T>
class SafeFlow : public AbstractFlow<T> {
private:
    std::function<void(FlowCollector<T>*)> block; // TODO: suspend lambda

public:
    SafeFlow(std::function<void(FlowCollector<T>*)> block) : block(block) {}

    void collect_safely(FlowCollector<T>* collector) override { // TODO: suspend
        collector->block(); // TODO: extension function call semantics
    }
};

/**
 * Creates a _cold_ flow that produces a single value from the given functional type.
 */
template<typename T>
Flow<T>* as_flow(std::function<T()> func) {
    return flow<T>([func](FlowCollector<T>* collector) { // TODO: suspend
        collector->emit(func());
    });
}

/**
 * Creates a _cold_ flow that produces a single value from the given functional type.
 *
 * Example of usage:
 *
 * ```
 * auto remote_call(): R { return ...; }
 * auto remote_call_flow(): Flow<R> { return ::remoteCall.asFlow(); }
 * ```
 */
template<typename T>
Flow<T>* as_flow(std::function<T()> func) { // TODO: suspend lambda parameter
    return flow<T>([func](FlowCollector<T>* collector) { // TODO: suspend
        collector->emit(func());
    });
}

/**
 * Creates a _cold_ flow that produces values from the given iterable.
 */
template<typename T>
Flow<T>* as_flow(Iterable<T>* iterable) { // TODO: define Iterable
    return flow<T>([iterable](FlowCollector<T>* collector) { // TODO: suspend
        for (auto value : *iterable) { // TODO: proper iterator protocol
            collector->emit(value);
        }
    });
}

/**
 * Creates a _cold_ flow that produces values from the given iterator.
 */
template<typename T>
Flow<T>* as_flow(Iterator<T>* iterator) { // TODO: define Iterator
    return flow<T>([iterator](FlowCollector<T>* collector) { // TODO: suspend
        while (iterator->hasNext()) { // TODO: proper iterator protocol
            collector->emit(iterator->next());
        }
    });
}

/**
 * Creates a _cold_ flow that produces values from the given sequence.
 */
template<typename T>
Flow<T>* as_flow(Sequence<T>* sequence) { // TODO: define Sequence
    return flow<T>([sequence](FlowCollector<T>* collector) { // TODO: suspend
        for (auto value : *sequence) { // TODO: proper iterator protocol
            collector->emit(value);
        }
    });
}

/**
 * Creates a flow that produces values from the specified `vararg`-arguments.
 *
 * Example of usage:
 *
 * ```
 * flowOf(1, 2, 3)
 * ```
 */
template<typename T>
Flow<T>* flow_of(std::initializer_list<T> elements) { // TODO: vararg to initializer_list mapping
    return flow<T>([elements](FlowCollector<T>* collector) { // TODO: suspend
        for (auto element : elements) {
            collector->emit(element);
        }
    });
}

/**
 * Creates a flow that produces the given [value].
 */
template<typename T>
Flow<T>* flow_of(T value) {
    return flow<T>([value](FlowCollector<T>* collector) { // TODO: suspend
        /*
         * Implementation note: this is just an "optimized" overload of flowOf(vararg)
         * which significantly reduces the footprint of widespread single-value flows.
         */
        collector->emit(value);
    });
}

/**
 * Returns an empty flow.
 */
template<typename T>
Flow<T>* empty_flow() {
    return &kEmptyFlow; // TODO: class singleton
}

// class EmptyFlow : Flow<Nothing>
class EmptyFlowImpl : public Flow<void> { // TODO: Nothing type
    void collect(FlowCollector<void>* collector) override { // TODO: suspend
        // Unit - do nothing
    }
};

static EmptyFlowImpl kEmptyFlow; // TODO: class singleton pattern

/**
 * Creates a _cold_ flow that produces values from the given array.
 * The flow being _cold_ means that the array components are read every time a terminal operator is applied
 * to the resulting flow.
 */
template<typename T>
Flow<T>* as_flow(T* array, size_t size) { // TODO: Array type mapping
    return flow<T>([array, size](FlowCollector<T>* collector) { // TODO: suspend
        for (size_t i = 0; i < size; ++i) {
            collector->emit(array[i]);
        }
    });
}

/**
 * Creates a _cold_ flow that produces values from the array.
 * The flow being _cold_ means that the array components are read every time a terminal operator is applied
 * to the resulting flow.
 */
Flow<int>* as_flow(int* array, size_t size) { // IntArray
    return flow<int>([array, size](FlowCollector<int>* collector) { // TODO: suspend
        for (size_t i = 0; i < size; ++i) {
            collector->emit(array[i]);
        }
    });
}

/**
 * Creates a _cold_ flow that produces values from the given array.
 * The flow being _cold_ means that the array components are read every time a terminal operator is applied
 * to the resulting flow.
 */
Flow<long>* as_flow(long* array, size_t size) { // LongArray
    return flow<long>([array, size](FlowCollector<long>* collector) { // TODO: suspend
        for (size_t i = 0; i < size; ++i) {
            collector->emit(array[i]);
        }
    });
}

/**
 * Creates a flow that produces values from the range.
 */
Flow<int>* as_flow(IntRange range) { // TODO: define IntRange
    return flow<int>([range](FlowCollector<int>* collector) { // TODO: suspend
        for (int value = range.start; value <= range.end; value += range.step) { // TODO: range iteration
            collector->emit(value);
        }
    });
}

/**
 * Creates a flow that produces values from the range.
 */
Flow<long>* as_flow(LongRange range) { // TODO: define LongRange
    return flow<long>([range](FlowCollector<long>* collector) { // TODO: suspend
        for (long value = range.start; value <= range.end; value += range.step) { // TODO: range iteration
            collector->emit(value);
        }
    });
}

/**
 * Creates an instance of a _cold_ [Flow] with elements that are sent to a [SendChannel]
 * provided to the builder's [block] of code via [ProducerScope]. It allows elements to be
 * produced by code that is running in a different context or concurrently.
 * The resulting flow is _cold_, which means that [block] is called every time a terminal operator
 * is applied to the resulting flow.
 *
 * This builder ensures thread-safety and context preservation, thus the provided [ProducerScope] can be used
 * concurrently from different contexts.
 * The resulting flow completes as soon as the code in the [block] and all its children completes.
 * Use [awaitClose] as the last statement to keep it running.
 * A more detailed example is provided in the documentation of [callbackFlow].
 *
 * A channel with the [default][Channel.BUFFERED] buffer size is used. Use the [buffer] operator on the
 * resulting flow to specify a user-defined value and to control what happens when data is produced faster
 * than consumed, i.e. to control the back-pressure behavior.
 *
 * Adjacent applications of [channelFlow], [flowOn], [buffer], and [produceIn] are
 * always fused so that only one properly configured channel is used for execution.
 *
 * Examples of usage:
 *
 * ```
 * fun <T> Flow<T>.merge(other: Flow<T>): Flow<T> = channelFlow {
 *     // collect from one coroutine and send it
 *     launch {
 *         collect { send(it) }
 *     }
 *     // collect and send from this coroutine, too, concurrently
 *     other.collect { send(it) }
 * }
 *
 * fun <T> contextualFlow(): Flow<T> = channelFlow {
 *     // send from one coroutine
 *     launch(Dispatchers.IO) {
 *         send(computeIoValue())
 *     }
 *     // send from another coroutine, concurrently
 *     launch(Dispatchers.Default) {
 *         send(computeCpuValue())
 *     }
 * }
 * ```
 */
template<typename T>
Flow<T>* channel_flow(std::function<void(ProducerScope<T>*)> block) { // TODO: suspend lambda, @BuilderInference
    return new ChannelFlowBuilder<T>(block);
}

/**
 * Creates an instance of a _cold_ [Flow] with elements that are sent to a [SendChannel]
 * provided to the builder's [block] of code via [ProducerScope]. It allows elements to be
 * produced by code that is running in a different context or concurrently.
 *
 * The resulting flow is _cold_, which means that [block] is called every time a terminal operator
 * is applied to the resulting flow.
 *
 * This builder ensures thread-safety and context preservation, thus the provided [ProducerScope] can be used
 * from any context, e.g. from a callback-based API.
 * The resulting flow completes as soon as the code in the [block] completes.
 * [awaitClose] should be used to keep the flow running, otherwise the channel will be closed immediately
 * when block completes.
 * [awaitClose] argument is called either when a flow consumer cancels the flow collection
 * or when a callback-based API invokes [SendChannel.close] manually and is typically used
 * to cleanup the resources after the completion, e.g. unregister a callback.
 * Using [awaitClose] is mandatory in order to prevent memory leaks when the flow collection is cancelled,
 * otherwise the callback may keep running even when the flow collector is already completed.
 * To avoid such leaks, this method throws [IllegalStateException] if block returns, but the channel
 * is not closed yet.
 *
 * A channel with the [default][Channel.BUFFERED] buffer size is used. Use the [buffer] operator on the
 * resulting flow to specify a user-defined value and to control what happens when data is produced faster
 * than consumed, i.e. to control the back-pressure behavior.
 *
 * Adjacent applications of [callbackFlow], [flowOn], [buffer], and [produceIn] are
 * always fused so that only one properly configured channel is used for execution.
 *
 * Example of usage that converts a multi-shot callback API to a flow.
 * For single-shot callbacks use [suspendCancellableCoroutine].
 *
 * ```
 * auto flow_from(api: CallbackBasedApi): Flow<T> = callbackFlow {
 *     auto callback = class : Callback { // Implementation of some callback interface
 *         virtual auto on_next_value(value: T) {
 *             // To avoid blocking you can configure channel capacity using
 *             // either buffer(Channel.CONFLATED) or buffer(Channel.UNLIMITED) to avoid overfill
 *             trySendBlocking(value)
 *                 .onFailure { throwable ->
 *                     // Downstream has been cancelled or failed, can log here
 *                 }
 *         }
 *         virtual auto on_api_error(cause: Throwable) {
 *             cancel(CancellationException("API Error", cause))
 *         }
 *         virtual auto on_completed() { return channel.close(); }
 *     }
 *     api.register(callback)
 *     /*
 *      * Suspends until either 'onCompleted'/'onApiError' from the callback is invoked
 *      * or flow collector is cancelled (e.g. by 'take(1)' or because a collector's coroutine was cancelled).
 *      * In both cases, callback will be properly unregistered.
 *      */
 *     awaitClose { api.unregister(callback) }
 * }
 * ```
 *
 * > The callback `register`/`unregister` methods provided by an external API must be thread-safe, because
 * > `awaitClose` block can be called at any time due to asynchronous nature of cancellation, even
 * > concurrently with the call of the callback.
 */
template<typename T>
Flow<T>* callback_flow(std::function<void(ProducerScope<T>*)> block) { // TODO: suspend lambda, @BuilderInference
    return new CallbackFlowBuilder<T>(block);
}

// ChannelFlow implementation that is the first in the chain of flow operations and introduces (builds) a flow
template<typename T>
class ChannelFlowBuilder : public ChannelFlow<T> {
private:
    std::function<void(ProducerScope<T>*)> block; // TODO: suspend lambda

public:
    ChannelFlowBuilder(
        std::function<void(ProducerScope<T>*)> block,
        CoroutineContext* context = &kEmptyCoroutineContext, // TODO: default parameter
        int capacity = kBuffered, // BUFFERED constant
        BufferOverflow on_buffer_overflow = BufferOverflow::kSuspend
    ) : ChannelFlow<T>(context, capacity, on_buffer_overflow), block(block) {}

    ChannelFlow<T>* create(CoroutineContext* context, int capacity, BufferOverflow on_buffer_overflow) override {
        return new ChannelFlowBuilder<T>(block, context, capacity, on_buffer_overflow);
    }

    void collect_to(ProducerScope<T>* scope) override { // TODO: suspend
        block(scope);
    }

    std::string to_string() override {
        return "block[" + /* TODO: block.tostd::string() */ "] -> " + ChannelFlow<T>::to_string();
    }
};

template<typename T>
class CallbackFlowBuilder : public ChannelFlowBuilder<T> {
private:
    std::function<void(ProducerScope<T>*)> block; // TODO: suspend lambda

public:
    CallbackFlowBuilder(
        std::function<void(ProducerScope<T>*)> block,
        CoroutineContext* context = &kEmptyCoroutineContext,
        int capacity = kBuffered,
        BufferOverflow on_buffer_overflow = BufferOverflow::kSuspend
    ) : ChannelFlowBuilder<T>(block, context, capacity, on_buffer_overflow), block(block) {}

    void collect_to(ProducerScope<T>* scope) override { // TODO: suspend
        ChannelFlowBuilder<T>::collect_to(scope);
        /*
         * We expect user either call `awaitClose` from within a block (then the channel is closed at this moment)
         * or being closed/cancelled externally/manually. Otherwise "user forgot to call
         * awaitClose and receives unhelpful ClosedSendChannelException exceptions" situation is detected.
         */
        if (!scope->is_closed_for_send()) {
            throw std::runtime_error( // IllegalStateException
                "'awaitClose { yourCallbackOrListener.cancel() }' should be used in the end of callbackFlow block.\n"
                "Otherwise, a callback/listener may leak in case of external cancellation.\n"
                "See callbackFlow API documentation for the details."
            );
        }
    }

    ChannelFlow<T>* create(CoroutineContext* context, int capacity, BufferOverflow on_buffer_overflow) override {
        return new CallbackFlowBuilder<T>(block, context, capacity, on_buffer_overflow);
    }
};

}}} // namespace kotlinx::coroutines::flow
