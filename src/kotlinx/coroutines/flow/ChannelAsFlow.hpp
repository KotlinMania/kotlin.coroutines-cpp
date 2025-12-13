#pragma once
/**
 * @file ChannelAsFlow.hpp
 * @brief ReceiveChannel <-> Flow adapters (receive_as_flow / consume_as_flow).
 *
 * Transliterated from: kotlinx-coroutines-core/common/src/flow/Channels.kt
 *
 * NOTE: This file is intentionally split out from `flow/Channels.hpp` to avoid a circular include:
 * `flow/internal/ChannelFlow.hpp` depends on `flow/Channels.hpp` for `emit_all`, so `flow/Channels.hpp`
 * cannot itself include `flow/internal/ChannelFlow.hpp`.
 */

#include "kotlinx/coroutines/Continuation.hpp"
#include "kotlinx/coroutines/context_impl.hpp"
#include "kotlinx/coroutines/channels/Channel.hpp"
#include "kotlinx/coroutines/flow/Channels.hpp"
#include "kotlinx/coroutines/flow/internal/ChannelFlow.hpp"
#include <atomic>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>

namespace kotlinx::coroutines::flow {

/**
 * Kotlin: private class ChannelAsFlow<T>(channel, consume, context, capacity, onBufferOverflow) : ChannelFlow<T>
 */
template <typename T>
class ChannelAsFlow final : public internal::ChannelFlow<T> {
public:
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

    internal::ChannelFlow<T>* create(std::shared_ptr<CoroutineContext> context,
                                     int capacity,
                                     channels::BufferOverflow on_buffer_overflow) override {
        return new ChannelAsFlow<T>(channel_, consume_, std::move(context), capacity, on_buffer_overflow);
    }

    Flow<T>* drop_channel_operators() override {
        // Kotlin: ChannelAsFlow(channel, consume)
        return new ChannelAsFlow<T>(channel_, consume_);
    }

    void collect_to(channels::ProducerScope<T>* scope) override {
        internal::SendingCollector<T> sending(scope);
        // Kotlin: SendingCollector(scope).emitAllImpl(channel, consume)
        // TODO(semantics): This loop is currently blocking (try_receive + yield) until channels are suspend-aware.
        (void)emit_all_impl<T>(&sending, channel_.get(), consume_, /*cont=*/nullptr);
    }

    std::shared_ptr<channels::ReceiveChannel<T>> produce_impl(CoroutineScope* scope) override {
        mark_consumed();
        if (this->capacity() == channels::Channel<T>::OPTIONAL_CHANNEL) {
            return channel_;
        }
        return internal::ChannelFlow<T>::produce_impl(scope);
    }

    void* collect(FlowCollector<T>* collector, Continuation<void*>* continuation) override {
        if (this->capacity() == channels::Channel<T>::OPTIONAL_CHANNEL) {
            mark_consumed();
            return emit_all_impl<T>(collector, channel_.get(), consume_, continuation);
        }
        return internal::ChannelFlow<T>::collect(collector, continuation);
    }

    std::string additional_to_string_props() override {
        std::ostringstream oss;
        oss << "channel=" << channel_.get();
        return oss.str();
    }

private:
    void mark_consumed() {
        if (!consume_) return;
        bool already = consumed_.exchange(true, std::memory_order_acq_rel);
        if (already) {
            throw std::logic_error("ReceiveChannel.consumeAsFlow can be collected just once");
        }
    }

    std::shared_ptr<channels::ReceiveChannel<T>> channel_;
    bool consume_;
    std::atomic<bool> consumed_{false};
};

/**
 * Kotlin: fun <T> ReceiveChannel<T>.receiveAsFlow(): Flow<T> = ChannelAsFlow(this, consume = false)
 */
template <typename T>
inline std::shared_ptr<Flow<T>> receive_as_flow(std::shared_ptr<channels::ReceiveChannel<T>> channel) {
    return std::make_shared<ChannelAsFlow<T>>(std::move(channel), /*consume=*/false);
}

/**
 * Kotlin: fun <T> ReceiveChannel<T>.consumeAsFlow(): Flow<T> = ChannelAsFlow(this, consume = true)
 */
template <typename T>
inline std::shared_ptr<Flow<T>> consume_as_flow(std::shared_ptr<channels::ReceiveChannel<T>> channel) {
    return std::make_shared<ChannelAsFlow<T>>(std::move(channel), /*consume=*/true);
}

} // namespace kotlinx::coroutines::flow

