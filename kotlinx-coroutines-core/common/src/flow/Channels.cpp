#include <string>
#include "kotlinx/coroutines/core_fwd.hpp"
// Transliterated from Kotlin to C++ (first pass - syntax/language translation only)
// Original: kotlinx-coroutines-core/common/src/flow/Channels.kt
//
// TODO: Implement suspend/coroutine semantics
// TODO: Translate JVM annotations
// TODO: Implement atomicfu atomic operations
// TODO: Handle nullability (T*)
// TODO: Implement Channel operations
// TODO: Map Kotlin's Unit type

// @file:JvmMultifileClass
// @file:JvmName("FlowKt")

namespace kotlinx { namespace coroutines { namespace flow {

// import kotlinx.atomicfu.*
// import kotlinx.coroutines.*
// import kotlinx.coroutines.channels.*
// import kotlinx.coroutines.flow.internal.*
// import kotlin.coroutines.*
// import kotlin.jvm.*
// import kotlinx.coroutines.flow.internal.unsafeFlow as flow

/**
 * Emits all elements from the given [channel] to this flow collector and [cancels][cancel] (consumes)
 * the channel afterwards. If you need to iterate over the channel without consuming it,
 * a regular `for` loop should be used instead.
 *
 * Note, that emitting values from a channel into a flow is not atomic. A value that was received from the
 * channel many not reach the flow collector if it was cancelled and will be lost.
 *
 * This function provides a more efficient shorthand for `channel.consumeEach { value -> emit(value) }`.
 * See [consumeEach][ReceiveChannel.consumeEach].
 */
template<typename T>
void emit_all(FlowCollector<T>* collector, ReceiveChannel<T>* channel) { // TODO: suspend, extension function
    emit_all_impl(collector, channel, true);
}

template<typename T>
void emit_all_impl(FlowCollector<T>* collector, ReceiveChannel<T>* channel, bool consume) { // TODO: suspend
    collector->ensure_active();
    Throwable* cause = nullptr;
    try {
        for (*channel element) { // TODO: channel iteration
            collector->emit(element);
        }
    } catch (Throwable& e) {
        cause = &e;
        throw;
    } // TODO: finally block
    if (consume) channel->cancel_consumed(cause);
}

/**
 * Represents the given receive channel as a hot flow and [receives][ReceiveChannel.receive] from the channel
 * in fan-out fashion every time this flow is collected. One element will be emitted to one collector only.
 *
 * See also [consumeAsFlow] which ensures that the resulting flow is collected just once.
 *
 * ### Cancellation semantics
 *
 * - Flow collectors are cancelled when the original channel is [closed][SendChannel.close] with an exception.
 * - Flow collectors complete normally when the original channel is [closed][SendChannel.close] normally.
 * - Failure or cancellation of the flow collector does not affect the channel.
 *   However, if a flow collector gets cancelled after receiving an element from the channel but before starting
 *   to process it, the element will be lost, and the `onUndeliveredElement` callback of the [Channel],
 *   if provided on channel construction, will be invoked.
 *   See [Channel.receive] for details of the effect of the prompt cancellation guarantee on element delivery.
 *
 * ### Operator fusion
 *
 * Adjacent applications of [flowOn], [buffer], [conflate], and [produceIn] to the result of `receiveAsFlow` are fused.
 * In particular, [produceIn] returns the original channel.
 * Calls to [flowOn] have generally no effect, unless [buffer] is used to explicitly request buffering.
 */
template<typename T>
Flow<T>* receive_as_flow(ReceiveChannel<T>* channel) { // TODO: extension function
    return new ChannelAsFlow<T>(channel, false);
}

/**
 * Represents the given receive channel as a hot flow and [consumes][ReceiveChannel.consume] the channel
 * on the first collection from this flow. The resulting flow can be collected just once and throws
 * [IllegalStateException] when trying to collect it more than once.
 *
 * See also [receiveAsFlow] which supports multiple collectors of the resulting flow.
 *
 * ### Cancellation semantics
 *
 * - Flow collector is cancelled when the original channel is [closed][SendChannel.close] with an exception.
 * - Flow collector completes normally when the original channel is [closed][SendChannel.close] normally.
 * - If the flow collector fails with an exception (for example, by getting cancelled),
 *   the source channel is [cancelled][ReceiveChannel.cancel].
 *
 * ### Operator fusion
 *
 * Adjacent applications of [flowOn], [buffer], [conflate], and [produceIn] to the result of `consumeAsFlow` are fused.
 * In particular, [produceIn] returns the original channel (but throws [IllegalStateException] on repeated calls).
 * Calls to [flowOn] have generally no effect, unless [buffer] is used to explicitly request buffering.
 */
template<typename T>
Flow<T>* consume_as_flow(ReceiveChannel<T>* channel) { // TODO: extension function
    return new ChannelAsFlow<T>(channel, true);
}

/**
 * Represents an existing [channel] as [ChannelFlow] implementation.
 * It fuses with subsequent [flowOn] operators, but for the most part ignores the specified context.
 * However, additional [buffer] calls cause a separate buffering channel to be created and that is where
 * the context might play a role, because it is used by the producing coroutine.
 */
template<typename T>
class ChannelAsFlow : ChannelFlow<T> {
private:
    ReceiveChannel<T>* channel;
    bool consume;
    std::atomic<bool> consumed; // TODO: atomic from kotlinx.atomicfu

public:
    ChannelAsFlow(
        ReceiveChannel<T>* channel,
        bool consume,
        CoroutineContext* context = &kEmptyCoroutineContext,
        int capacity = kOptionalChannel, // Channel.OPTIONAL_CHANNEL
        BufferOverflow on_buffer_overflow = BufferOverflow::kSuspend
    ) : ChannelFlow<T>(context, capacity, on_buffer_overflow),
        channel(channel), consume(consume), consumed(false) {}

    void mark_consumed() {
        if (consume) {
            bool expected = false;
            if (!consumed.compare_exchange_strong(expected, true)) {
                throw std::runtime_error("ReceiveChannel.consumeAsFlow can be collected just once");
            }
        }
    }

    ChannelFlow<T>* create(CoroutineContext* context, int capacity, BufferOverflow on_buffer_overflow) override {
        return new ChannelAsFlow<T>(channel, consume, context, capacity, on_buffer_overflow);
    }

    Flow<T>* drop_channel_operators() override {
        return new ChannelAsFlow<T>(channel, consume);
    }

    void collect_to(ProducerScope<T>* scope) override { // TODO: suspend
        // use efficient channel receiving code from emitAll
        SendingCollector<T> collector(scope);
        emit_all_impl(&collector, channel, consume);
    }

    ReceiveChannel<T>* produce_impl(CoroutineScope* scope) override {
        mark_consumed(); // fail fast on repeated attempt to collect it
        if (this->capacity == kOptionalChannel) {
            return channel; // direct
        } else {
            return ChannelFlow<T>::produce_impl(scope); // extra buffering channel
        }
    }

    void collect(FlowCollector<T>* collector) override { // TODO: suspend
        if (this->capacity == kOptionalChannel) {
            mark_consumed();
            emit_all_impl(collector, channel, consume); // direct
        } else {
            ChannelFlow<T>::collect(collector); // extra buffering channel, produceImpl will mark it as consumed
        }
    }

    std::string additional_to_string_props() override {
        return "channel=" + /* TODO: channel.tostd::string() */;
    }
};

/**
 * Creates a [produce] coroutine that collects the given flow.
 *
 * This transformation is **stateful**, it launches a [produce] coroutine
 * that collects the given flow, and has the same behavior:
 *
 * - if collecting the flow throws, the channel will be closed with that exception
 * - if the [ReceiveChannel] is cancelled, the collection of the flow will be cancelled
 * - if collecting the flow completes normally, the [ReceiveChannel] will be closed normally
 *
 * A channel with [default][Channel.Factory.BUFFERED] buffer size is created.
 * Use [buffer] operator on the flow before calling `produceIn` to specify a value other than
 * default and to control what happens when data is produced faster than it is consumed,
 * that is to control backpressure behavior.
 */
template<typename T>
ReceiveChannel<T>* produce_in(Flow<T>* flow, CoroutineScope* scope) { // TODO: extension function
    return flow->as_channel_flow()->produce_impl(scope);
}

}}} // namespace kotlinx::coroutines::flow
