#pragma once
/**
 * Transliterated from: kotlinx-coroutines-core/common/src/flow/Channels.kt
 *
 * Kotlin file header (translated):
 *   @file:JvmMultifileClass
 *   @file:JvmName("FlowKt")
 *   package kotlinx.coroutines.flow
 */

#include "kotlinx/coroutines/CoroutineContext.hpp"
#include "kotlinx/coroutines/Continuation.hpp"
#include "kotlinx/coroutines/CoroutineScope.hpp"
#include "kotlinx/coroutines/Result.hpp"
#include "kotlinx/coroutines/channels/BufferOverflow.hpp"
#include "kotlinx/coroutines/channels/Channel.hpp"
#include "kotlinx/coroutines/channels/Channels.hpp"
#include "kotlinx/coroutines/dsl/Suspend.hpp"
#include "kotlinx/coroutines/flow/Flow.hpp"
#include "kotlinx/coroutines/flow/FlowCollector.hpp"
#include "kotlinx/coroutines/flow/internal/ChannelFlow.hpp"
#include "kotlinx/coroutines/flow/internal/SendingCollector.hpp"
#include "kotlinx/coroutines/intrinsics/Intrinsics.hpp"

#include <atomic>
#include <exception>
#include <memory>
#include <string>

namespace kotlinx::coroutines::flow {

// kotlinx.coroutines.flow.internal.unsafeFlow is imported in upstream and
// aliased to `flow`. The C++ port models cold flows as direct ChannelFlow
// constructions, so the alias is not needed at the call site here.

/**
 * Emits all elements from the given [channel] to this flow collector and [cancels][cancel] (consumes)
 * the channel afterwards. If you need to iterate over the channel without consuming it,
 * a regular `for` loop should be used instead.
 *
 * Note, that emitting values from a channel into a flow is not atomic. A value that was received from the
 * channel many not reach the flow collector if it was cancelled and will be lost.
 *
 * This function provides a more efficient shorthand for `channel->consume_each([&](T value) { emit(value); })`.
 * See [consume_each][ReceiveChannel.consumeEach].
 *
 * Upstream:
 *   public suspend fun <T> FlowCollector<T>.emitAll(channel: ReceiveChannel<T>): Unit =
 *       emitAllImpl(channel, consume = true)
 */
template <typename T>
[[suspend]]
void* emit_all(
    FlowCollector<T>* receiver,
    channels::ReceiveChannel<T>* channel,
    std::shared_ptr<Continuation<void*>> completion);

/**
 * Private helper. Iterates the channel and emits to the collector; cancels the channel
 * on the way out when `consume` is true. Mirrors the upstream `emitAllImpl` private function.
 *
 * Upstream:
 *   private suspend fun <T> FlowCollector<T>.emitAllImpl(channel: ReceiveChannel<T>, consume: Boolean) {
 *       ensureActive()
 *       var cause: Throwable? = null
 *       try {
 *           for (element in channel) {
 *               emit(element)
 *           }
 *       } catch (e: Throwable) {
 *           cause = e
 *           throw e
 *       } finally {
 *           if (consume) channel.cancelConsumed(cause)
 *       }
 *   }
 */
template <typename T>
[[suspend]]
void* emit_all_impl(
    FlowCollector<T>* receiver,
    channels::ReceiveChannel<T>* channel,
    bool consume,
    std::shared_ptr<Continuation<void*>> completion);

/**
 * Represents the given receive channel as a hot flow and [receives][ReceiveChannel.receive] from the channel
 * in fan-out fashion every time this flow is collected. One element will be emitted to one collector only.
 *
 * See also [consume_as_flow] which ensures that the resulting flow is collected just once.
 *
 * ### Cancellation semantics
 *
 * - Flow collectors are cancelled when the original channel is [closed][SendChannel.close] with an exception.
 * - Flow collectors complete normally when the original channel is [closed][SendChannel.close] normally.
 * - Failure or cancellation of the flow collector does not affect the channel.
 *   However, if a flow collector gets cancelled after receiving an element from the channel but before starting
 *   to process it, the element will be lost, and the `on_undelivered_element` callback of the [Channel],
 *   if provided on channel construction, will be invoked.
 *   See [Channel.receive] for details of the effect of the prompt cancellation guarantee on element delivery.
 *
 * ### Operator fusion
 *
 * Adjacent applications of [flow_on], [buffer], [conflate], and [produce_in] to the result of `receive_as_flow` are fused.
 * In particular, [produce_in] returns the original channel.
 * Calls to [flow_on] have generally no effect, unless [buffer] is used to explicitly request buffering.
 *
 * Upstream:
 *   public fun <T> ReceiveChannel<T>.receiveAsFlow(): Flow<T> = ChannelAsFlow(this, consume = false)
 */
template <typename T>
std::shared_ptr<Flow<T>> receive_as_flow(std::shared_ptr<channels::ReceiveChannel<T>> channel);

/**
 * Represents the given receive channel as a hot flow and [consumes][ReceiveChannel.consume] the channel
 * on the first collection from this flow. The resulting flow can be collected just once and throws
 * [IllegalStateException] when trying to collect it more than once.
 *
 * See also [receive_as_flow] which supports multiple collectors of the resulting flow.
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
 * Adjacent applications of [flow_on], [buffer], [conflate], and [produce_in] to the result of `consume_as_flow` are fused.
 * In particular, [produce_in] returns the original channel (but throws [IllegalStateException] on repeated calls).
 * Calls to [flow_on] have generally no effect, unless [buffer] is used to explicitly request buffering.
 *
 * Upstream:
 *   public fun <T> ReceiveChannel<T>.consumeAsFlow(): Flow<T> = ChannelAsFlow(this, consume = true)
 */
template <typename T>
std::shared_ptr<Flow<T>> consume_as_flow(std::shared_ptr<channels::ReceiveChannel<T>> channel);

/**
 * Represents an existing [channel] as [ChannelFlow] implementation.
 * It fuses with subsequent [flow_on] operators, but for the most part ignores the specified context.
 * However, additional [buffer] calls cause a separate buffering channel to be created and that is where
 * the context might play a role, because it is used by the producing coroutine.
 *
 * Upstream:
 *   private class ChannelAsFlow<T>(
 *       private val channel: ReceiveChannel<T>,
 *       private val consume: Boolean,
 *       context: CoroutineContext = EmptyCoroutineContext,
 *       capacity: Int = Channel.OPTIONAL_CHANNEL,
 *       onBufferOverflow: BufferOverflow = BufferOverflow.SUSPEND
 *   ) : ChannelFlow<T>(context, capacity, onBufferOverflow)
 */
template <typename T>
class ChannelAsFlow : public internal::ChannelFlow<T> {
public:
    ChannelAsFlow(
        std::shared_ptr<channels::ReceiveChannel<T>> channel,
        bool consume,
        std::shared_ptr<CoroutineContext> context = EmptyCoroutineContext::instance(),
        int capacity = channels::CHANNEL_OPTIONAL,
        channels::BufferOverflow on_buffer_overflow = channels::BufferOverflow::SUSPEND)
        : internal::ChannelFlow<T>(context, capacity, on_buffer_overflow),
          channel_(std::move(channel)),
          consume_(consume),
          consumed_(std::make_shared<std::atomic<bool>>(false)) {}

    /** Mirrors `private fun markConsumed()` in upstream. */
    void mark_consumed() {
        if (consume_) {
            bool expected = false;
            if (!consumed_->compare_exchange_strong(expected, true)) {
                // Upstream: check(!consumed.getAndSet(true)) { "..." }
                throw std::logic_error(
                    "ReceiveChannel.consume_as_flow can be collected just once");
            }
        }
    }

    /** Upstream: override fun create(...) = ChannelAsFlow(channel, consume, context, capacity, onBufferOverflow) */
    internal::ChannelFlow<T>* create(
        std::shared_ptr<CoroutineContext> context,
        int capacity,
        channels::BufferOverflow on_buffer_overflow) override {
        return new ChannelAsFlow(channel_, consume_, context, capacity, on_buffer_overflow);
    }

    /** Upstream: override fun dropChannelOperators(): Flow<T> = ChannelAsFlow(channel, consume) */
    Flow<T>* drop_channel_operators() override {
        return new ChannelAsFlow(channel_, consume_);
    }

    /**
     * Upstream:
     *   override suspend fun collectTo(scope: ProducerScope<T>) =
     *       SendingCollector(scope).emitAllImpl(channel, consume)
     */
    [[suspend]]
    void* collect_to(
        channels::ProducerScope<T>* scope,
        std::shared_ptr<Continuation<void*>> completion) override {
        internal::SendingCollector<T> collector(scope);
        return emit_all_impl(&collector, channel_.get(), consume_, completion);
    }

    /**
     * Upstream:
     *   override fun produceImpl(scope: CoroutineScope): ReceiveChannel<T> {
     *       markConsumed()
     *       return if (capacity == Channel.OPTIONAL_CHANNEL) channel else super.produceImpl(scope)
     *   }
     */
    std::shared_ptr<channels::ReceiveChannel<T>> produce_impl(CoroutineScope* scope) override {
        mark_consumed();
        if (this->capacity() == channels::CHANNEL_OPTIONAL) {
            return channel_;
        }
        return internal::ChannelFlow<T>::produce_impl(scope);
    }

    /**
     * Upstream:
     *   override suspend fun collect(collector: FlowCollector<T>) {
     *       if (capacity == Channel.OPTIONAL_CHANNEL) {
     *           markConsumed()
     *           collector.emitAllImpl(channel, consume)
     *       } else {
     *           super.collect(collector)
     *       }
     *   }
     */
    [[suspend]]
    void* collect(
        FlowCollector<T>* collector,
        std::shared_ptr<Continuation<void*>> completion) override {
        if (this->capacity() == channels::CHANNEL_OPTIONAL) {
            mark_consumed();
            return emit_all_impl(collector, channel_.get(), consume_, completion);
        }
        return internal::ChannelFlow<T>::collect(collector, completion);
    }

    /** Upstream: override fun additionalToStringProps(): String = "channel=$channel" */
    std::string additional_to_string_props() const override {
        return std::string("channel=") + std::to_string(
            reinterpret_cast<std::uintptr_t>(channel_.get()));
    }

private:
    std::shared_ptr<channels::ReceiveChannel<T>> channel_;
    bool consume_;
    std::shared_ptr<std::atomic<bool>> consumed_;
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
 * Use [buffer] operator on the flow before calling `produce_in` to specify a value other than
 * default and to control what happens when data is produced faster than it is consumed,
 * that is to control backpressure behavior.
 *
 * Upstream:
 *   public fun <T> Flow<T>.produceIn(scope: CoroutineScope): ReceiveChannel<T> =
 *       asChannelFlow().produceImpl(scope)
 */
template <typename T>
std::shared_ptr<channels::ReceiveChannel<T>> produce_in(
    std::shared_ptr<Flow<T>> flow,
    CoroutineScope* scope);

// ============================================================================
// Inline template implementations
// ============================================================================

template <typename T>
[[suspend]]
inline void* emit_all(
    FlowCollector<T>* receiver,
    channels::ReceiveChannel<T>* channel,
    std::shared_ptr<Continuation<void*>> completion) {
    return emit_all_impl(receiver, channel, /*consume=*/true, std::move(completion));
}

template <typename T>
[[suspend]]
inline void* emit_all_impl(
    FlowCollector<T>* receiver,
    channels::ReceiveChannel<T>* channel,
    bool consume,
    std::shared_ptr<Continuation<void*>> completion) {
    // Upstream: ensureActive()
    if (completion) {
        auto ctx = completion->get_context();
        if (ctx) context_ensure_active(*ctx);
    }
    // Upstream: var cause: Throwable? = null
    std::exception_ptr cause = nullptr;
    try {
        // Upstream: for (element in channel) { emit(element) }
        auto iterator = channel->iterator();
        while (true) {
            void* has_next_result =
                dsl::suspend(iterator->has_next(completion.get()));
            if (intrinsics::is_coroutine_suspended(has_next_result)) {
                return intrinsics::get_COROUTINE_SUSPENDED();
            }
            bool has_next =
                has_next_result && *static_cast<bool*>(has_next_result);
            if (!has_next) break;
            T element = iterator->next();
            void* emit_result =
                dsl::suspend(receiver->emit(std::move(element), completion.get()));
            if (intrinsics::is_coroutine_suspended(emit_result)) {
                return intrinsics::get_COROUTINE_SUSPENDED();
            }
        }
    } catch (...) {
        // Upstream: catch (e: Throwable) { cause = e; throw e }
        cause = std::current_exception();
        if (consume) channels::cancel_consumed(channel, cause);
        std::rethrow_exception(cause);
    }
    // Upstream: finally { if (consume) channel.cancelConsumed(cause) }
    if (consume) channels::cancel_consumed(channel, cause);
    return nullptr;
}

template <typename T>
inline std::shared_ptr<Flow<T>> receive_as_flow(
    std::shared_ptr<channels::ReceiveChannel<T>> channel) {
    return std::make_shared<ChannelAsFlow<T>>(std::move(channel), /*consume=*/false);
}

template <typename T>
inline std::shared_ptr<Flow<T>> consume_as_flow(
    std::shared_ptr<channels::ReceiveChannel<T>> channel) {
    return std::make_shared<ChannelAsFlow<T>>(std::move(channel), /*consume=*/true);
}

template <typename T>
inline std::shared_ptr<channels::ReceiveChannel<T>> produce_in(
    std::shared_ptr<Flow<T>> flow,
    CoroutineScope* scope) {
    // Upstream: asChannelFlow().produceImpl(scope)
    return internal::as_channel_flow(std::move(flow))->produce_impl(scope);
}

} // namespace kotlinx::coroutines::flow
