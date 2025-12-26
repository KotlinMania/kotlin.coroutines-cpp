#pragma once
/**
 * @file Channels.hpp
 * @brief Flow-Channel integration utilities.
 *
 * Transliterated from: kotlinx-coroutines-core/common/src/flow/Channels.kt
 *
 * Provides conversion between flows and channels:
 * - emitAll() - emit all channel elements into a flow collector
 * - receiveAsFlow() - represent channel as a hot flow (fan-out)
 * - consumeAsFlow() - represent channel as a one-time consumable flow
 * - produceIn() - create a produce coroutine that collects the given flow
 */

#include "kotlinx/coroutines/Continuation.hpp"
#include "kotlinx/coroutines/Job.hpp"
#include "kotlinx/coroutines/CoroutineScope.hpp"
#include "kotlinx/coroutines/channels/Channel.hpp"
#include "kotlinx/coroutines/channels/Channels.hpp"
#include "kotlinx/coroutines/channels/ProducerScope.hpp"
#include "kotlinx/coroutines/flow/Flow.hpp"
#include "kotlinx/coroutines/flow/FlowCollector.hpp"
#include "kotlinx/coroutines/intrinsics/Intrinsics.hpp"
#include "kotlinx/coroutines/context_impl.hpp"
#include <atomic>
#include <exception>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <thread>

// Forward declarations to break circular dependency
namespace kotlinx::coroutines::flow::internal {
template <typename T> class ChannelFlow;
template <typename T> class SendingCollector;
} // namespace kotlinx::coroutines::flow::internal

namespace kotlinx::coroutines::flow {

/**
 * Line 25-26: Emits all elements from the given [channel] to this flow collector and [cancels][cancel] (consumes)
 * the channel afterwards.
 *
 * Kotlin: public suspend fun <T> FlowCollector<T>.emitAll(channel: ReceiveChannel<T>): Unit
 */
template <typename T>
void* emit_all(
    FlowCollector<T>* collector,
    channels::ReceiveChannel<T>* channel,
    Continuation<void*>* cont
);  // Forward declaration, defined after emit_all_impl

/**
 * Line 28-41: Internal implementation with configurable "consume" behavior.
 *
 * Kotlin: private suspend fun <T> FlowCollector<T>.emitAllImpl(channel: ReceiveChannel<T>, consume: Boolean)
 */
template <typename T>
void* emit_all_impl(
    FlowCollector<T>* collector,
    channels::ReceiveChannel<T>* channel,
    bool consume,
    Continuation<void*>* cont
) {
    // Line 29: ensureActive()
    if (cont) {
        auto ctx = cont->get_context();
        if (ctx) context_ensure_active(*ctx);
    }

    // Line 30: var cause: Throwable? = null
    std::exception_ptr cause = nullptr;

    // Line 31-40: try { for (element in channel) { emit(element) } } catch/finally
    try {
        // Line 32-34: for (element in channel) { emit(element) }
        // TODO(suspend-plugin): Replace with proper suspend iteration when channels support it
        while (true) {
            auto result = channel->try_receive();
            if (result.is_closed()) {
                break;
            }
            if (result.is_success()) {
                void* emitted = collector->emit(result.get_or_throw(), cont);
                if (emitted == intrinsics::get_COROUTINE_SUSPENDED()) {
                    return emitted;
                }
                continue;
            }
            // TODO(semantics): Kotlin would suspend here; we busy-wait
            std::this_thread::yield();
        }
    } catch (...) {
        // Line 35-37: catch (e: Throwable) { cause = e; throw e }
        cause = std::current_exception();
        // Line 38-40: finally { if (consume) channel.cancelConsumed(cause) }
        if (consume) {
            channels::cancel_consumed(channel, cause);
        }
        throw;
    }
    // Line 38-40: finally { if (consume) channel.cancelConsumed(cause) } - normal path
    if (consume) {
        channels::cancel_consumed(channel, cause);
    }

    return nullptr;
}

/**
 * Line 25-26: emit_all implementation
 */
template <typename T>
inline void* emit_all(
    FlowCollector<T>* collector,
    channels::ReceiveChannel<T>* channel,
    Continuation<void*>* cont
) {
    return emit_all_impl(collector, channel, /*consume=*/true, cont);
}

} // namespace kotlinx::coroutines::flow

// Include ChannelFlow and SendingCollector after emit_all_impl is defined
#include "kotlinx/coroutines/flow/internal/ChannelFlow.hpp"
#include "kotlinx/coroutines/flow/internal/SendingCollector.hpp"

namespace kotlinx::coroutines::flow {

/**
 * Line 95-137: Represents an existing [channel] as [ChannelFlow] implementation.
 *
 * Kotlin: private class ChannelAsFlow<T>(channel, consume, context, capacity, onBufferOverflow) : ChannelFlow<T>
 */
template <typename T>
class ChannelAsFlow final : public internal::ChannelFlow<T> {
public:
    // Line 95-101: Constructor
    ChannelAsFlow(
        std::shared_ptr<channels::ReceiveChannel<T>> channel,
        bool consume,
        std::shared_ptr<CoroutineContext> context = EmptyCoroutineContext::instance(),
        int capacity = channels::Channel<T>::OPTIONAL_CHANNEL,
        channels::BufferOverflow on_buffer_overflow = channels::BufferOverflow::SUSPEND
    )
        : internal::ChannelFlow<T>(context ? std::move(context) : EmptyCoroutineContext::instance(),
                                   capacity,
                                   on_buffer_overflow),
          channel_(std::move(channel)),
          consume_(consume) {}

    ~ChannelAsFlow() override = default;

    // Line 110-111: override fun create(context, capacity, onBufferOverflow): ChannelFlow<T>
    internal::ChannelFlow<T>* create(std::shared_ptr<CoroutineContext> context,
                                     int capacity,
                                     channels::BufferOverflow on_buffer_overflow) override {
        return new ChannelAsFlow<T>(channel_, consume_, std::move(context), capacity, on_buffer_overflow);
    }

    // Line 113-114: override fun dropChannelOperators(): Flow<T>
    Flow<T>* drop_channel_operators() override {
        return new ChannelAsFlow<T>(channel_, consume_);
    }

    // Line 116-117: override suspend fun collectTo(scope: ProducerScope<T>)
    void collect_to(channels::ProducerScope<T>* scope) override {
        internal::SendingCollector<T> sending(scope);
        // Kotlin: SendingCollector(scope).emitAllImpl(channel, consume)
        (void)::kotlinx::coroutines::flow::emit_all_impl<T>(&sending, channel_.get(), consume_, /*cont=*/nullptr);
    }

    // Line 119-125: override fun produceImpl(scope: CoroutineScope): ReceiveChannel<T>
    std::shared_ptr<channels::ReceiveChannel<T>> produce_impl(CoroutineScope* scope) override {
        mark_consumed(); // Line 120: fail fast on repeated attempt to collect it
        // Line 121-124: if (capacity == OPTIONAL_CHANNEL) channel else super.produceImpl(scope)
        if (this->capacity() == channels::Channel<T>::OPTIONAL_CHANNEL) {
            return channel_;
        }
        return internal::ChannelFlow<T>::produce_impl(scope);
    }

    // Line 127-134: override suspend fun collect(collector: FlowCollector<T>)
    void* collect(FlowCollector<T>* collector, Continuation<void*>* continuation) override {
        // Line 128-131: if (capacity == OPTIONAL_CHANNEL) { markConsumed(); collector.emitAllImpl(channel, consume) }
        if (this->capacity() == channels::Channel<T>::OPTIONAL_CHANNEL) {
            mark_consumed();
            return ::kotlinx::coroutines::flow::emit_all_impl<T>(collector, channel_.get(), consume_, continuation);
        }
        // Line 132-133: else { super.collect(collector) }
        return internal::ChannelFlow<T>::collect(collector, continuation);
    }

    // Line 136: override fun additionalToStringProps(): String
    std::string additional_to_string_props() override {
        std::ostringstream oss;
        oss << "channel=" << channel_.get();
        return oss.str();
    }

private:
    // Line 102: private val consumed = atomic(false)
    std::atomic<bool> consumed_{false};

    // Line 104-108: private fun markConsumed()
    void mark_consumed() {
        if (!consume_) return;
        // Line 106: check(!consumed.getAndSet(true)) { "..." }
        bool already = consumed_.exchange(true, std::memory_order_acq_rel);
        if (already) {
            throw std::logic_error("ReceiveChannel.consumeAsFlow can be collected just once");
        }
    }

    std::shared_ptr<channels::ReceiveChannel<T>> channel_;
    bool consume_;
};

/**
 * Line 65: Represents the given receive channel as a hot flow and [receives] from the channel
 * in fan-out fashion every time this flow is collected.
 *
 * Kotlin: public fun <T> ReceiveChannel<T>.receiveAsFlow(): Flow<T> = ChannelAsFlow(this, consume = false)
 */
template <typename T>
inline std::shared_ptr<Flow<T>> receive_as_flow(std::shared_ptr<channels::ReceiveChannel<T>> channel) {
    return std::make_shared<ChannelAsFlow<T>>(std::move(channel), /*consume=*/false);
}

/**
 * Line 87: Represents the given receive channel as a hot flow and [consumes] the channel
 * on the first collection from this flow.
 *
 * Kotlin: public fun <T> ReceiveChannel<T>.consumeAsFlow(): Flow<T> = ChannelAsFlow(this, consume = true)
 */
template <typename T>
inline std::shared_ptr<Flow<T>> consume_as_flow(std::shared_ptr<channels::ReceiveChannel<T>> channel) {
    return std::make_shared<ChannelAsFlow<T>>(std::move(channel), /*consume=*/true);
}

/**
 * Line 154-157: Creates a [produce] coroutine that collects the given flow.
 *
 * Kotlin: public fun <T> Flow<T>.produceIn(scope: CoroutineScope): ReceiveChannel<T>
 */
template <typename T>
inline std::shared_ptr<channels::ReceiveChannel<T>> produce_in(
    std::shared_ptr<Flow<T>> flow,
    CoroutineScope* scope
) {
    // Line 157: asChannelFlow().produceImpl(scope)
    auto channel_flow = internal::as_channel_flow(std::move(flow));
    return channel_flow->produce_impl(scope);
}

} // namespace kotlinx::coroutines::flow
