// port-lint: source kotlinx-coroutines-core/common/src/flow/Channels.kt
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
#include "kotlinx/coroutines/Job.hpp"
#include "kotlinx/coroutines/Result.hpp"
#include "kotlinx/coroutines/channels/BufferOverflow.hpp"
#include "kotlinx/coroutines/channels/Channel.hpp"
#include "kotlinx/coroutines/channels/Channels.hpp"
#include "kotlinx/coroutines/flow/Flow.hpp"
#include "kotlinx/coroutines/flow/FlowCollector.hpp"
#include "kotlinx/coroutines/flow/internal/ThrowingCollector.hpp"

namespace kotlinx::coroutines::flow {

// Transliterated from: kotlinx-coroutines-core/common/src/flow/Channels.kt:25-26
template <typename T>
void* emit_all(
    FlowCollector<T>* receiver,
    channels::ReceiveChannel<T>* channel,
    Continuation<void*>* completion);

// Transliterated from: kotlinx-coroutines-core/common/src/flow/Channels.kt:28-41
template <typename T>
void* emit_all_impl(
    FlowCollector<T>* receiver,
    channels::ReceiveChannel<T>* channel,
    bool consume,
    Continuation<void*>* completion);

} // namespace kotlinx::coroutines::flow

#include "kotlinx/coroutines/flow/internal/ChannelFlow.hpp"
#include "kotlinx/coroutines/flow/internal/SendingCollector.hpp"
#include "kotlinx/coroutines/intrinsics/Intrinsics.hpp"

#include <atomic>
#include <exception>
#include <functional>
#include <memory>
#include <optional>
#include <string>

namespace kotlinx::coroutines::flow {

namespace internal {

// Transliterated from: kotlinx-coroutines-core/common/src/flow/Channels.kt:104-108
void mark_channel_consumed(bool consume, std::atomic<bool>& consumed);

// Transliterated from: kotlinx-coroutines-core/common/src/flow/Channels.kt:28-41
// NOTE(port): Type erasure keeps the source loop concrete in Channels.cpp.
// The Clang frontend generates its suspension frame and retained local storage.
void* emit_all_erased(
    std::function<void()> make_iterator,
    std::function<void*(Continuation<void*>*)> has_next,
    std::function<void*(Continuation<void*>*)> emit_next,
    std::function<void()> finish_emit,
    std::function<void(std::exception_ptr)> cancel_consumed,
    bool consume,
    std::shared_ptr<Continuation<void*>> completion);

// Transliterated from: kotlinx-coroutines-core/common/src/flow/Channels.kt:28-41
// NOTE(port): Typed bindings carry the actual iterator and element through the
// concrete loop. Shared owners stay owned; raw arguments stay borrowed.
template <typename T>
struct EmitAllArguments {
    FlowCollector<T>* receiver;
    channels::ReceiveChannel<T>* channel;
    std::shared_ptr<FlowCollector<T>> receiver_owner;
    std::shared_ptr<channels::ReceiveChannel<T>> channel_owner;
    std::unique_ptr<channels::ChannelIterator<T>> iterator;
    std::optional<T> element;
};

// Transliterated from: kotlinx-coroutines-core/common/src/flow/Channels.kt:28-41
template <typename T>
inline void* bind_emit_all(
    FlowCollector<T>* receiver,
    channels::ReceiveChannel<T>* channel,
    bool consume,
    std::shared_ptr<Continuation<void*>> completion,
    std::shared_ptr<FlowCollector<T>> receiver_owner = nullptr,
    std::shared_ptr<channels::ReceiveChannel<T>> channel_owner = nullptr) {
    ensure_active(receiver);
    auto args = std::make_shared<EmitAllArguments<T>>(EmitAllArguments<T>{
        receiver, channel, std::move(receiver_owner), std::move(channel_owner), {}, {}});
    return emit_all_erased(
        [args] { args->iterator = args->channel->iterator(); },
        [args](Continuation<void*>* frame) { return args->iterator->has_next(frame); },
        [args](Continuation<void*>* frame) {
            args->element.emplace(args->iterator->next());
            return args->receiver->emit(std::move(*args->element), frame);
        },
        [args] { args->element.reset(); },
        [args](std::exception_ptr cause) { channels::cancel_consumed(args->channel, cause); },
        consume, std::move(completion));
}

} // namespace internal

// Transliterated from: kotlinx-coroutines-core/common/src/flow/Channels.kt:28-41
template <typename T>
inline void* emit_all_impl(
    FlowCollector<T>* receiver,
    channels::ReceiveChannel<T>* channel,
    bool consume,
    Continuation<void*>* completion) {
    return internal::bind_emit_all(receiver, channel, consume,
        kotlinx::coroutines::internal::retain_continuation(completion));
}

// Transliterated from: kotlinx-coroutines-core/common/src/flow/Channels.kt:28-41
template <typename T>
inline void* emit_all_impl(
    FlowCollector<T>* receiver,
    std::shared_ptr<channels::ReceiveChannel<T>> channel,
    bool consume,
    std::shared_ptr<Continuation<void*>> completion) {
    auto* channel_ptr = channel.get();
    return internal::bind_emit_all<T>(receiver, channel_ptr, consume,
        std::move(completion), nullptr, std::move(channel));
}

// Transliterated from: kotlinx-coroutines-core/common/src/flow/Channels.kt:28-41
template <typename T>
inline void* emit_all_impl(
    std::shared_ptr<FlowCollector<T>> receiver,
    std::shared_ptr<channels::ReceiveChannel<T>> channel,
    bool consume,
    std::shared_ptr<Continuation<void*>> completion) {
    auto* receiver_ptr = receiver.get();
    auto* channel_ptr = channel.get();
    return internal::bind_emit_all(receiver_ptr, channel_ptr, consume,
        std::move(completion), std::move(receiver), std::move(channel));
}

// Transliterated from: kotlinx-coroutines-core/common/src/flow/Channels.kt:28-41
template <typename T>
inline void* emit_all_impl(
    std::shared_ptr<FlowCollector<T>> receiver,
    channels::ReceiveChannel<T>* channel,
    bool consume,
    std::shared_ptr<Continuation<void*>> completion) {
    auto* receiver_ptr = receiver.get();
    return internal::bind_emit_all(receiver_ptr, channel, consume,
        std::move(completion), std::move(receiver));
}

// Transliterated from: kotlinx-coroutines-core/common/src/flow/Channels.kt:28-41
template <typename T>
inline void* emit_all_impl(
    std::shared_ptr<FlowCollector<T>> receiver,
    channels::ReceiveChannel<T>* channel,
    bool consume,
    Continuation<void*>* completion) {
    return emit_all_impl(std::move(receiver), channel, consume,
        kotlinx::coroutines::internal::retain_continuation(completion));
}

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
// Transliterated from: kotlinx-coroutines-core/common/src/flow/Channels.kt:25-26
template <typename T>
inline void* emit_all(
    FlowCollector<T>* receiver,
    channels::ReceiveChannel<T>* channel,
    Continuation<void*>* completion) {
    return emit_all_impl(receiver, channel, /*consume=*/true, completion);
}

/**
 * Represents an existing [channel] as [ChannelFlow] implementation.
 * It fuses with subsequent [flowOn] operators, but for the most part ignores the specified context.
 * However, additional [buffer] calls cause a separate buffering channel to be created and that is where
 * the context might play a role, because it is used by the producing coroutine.
 */
// Transliterated from: kotlinx-coroutines-core/common/src/flow/Channels.kt:95-137
// NOTE(port): The source-private generic class requires a header definition for
// arbitrary C++ element types. Its namespace preserves the source declaration.
template <typename T>
class ChannelAsFlow : public internal::ChannelFlow<T> {
public:
    // Transliterated from: kotlinx-coroutines-core/common/src/flow/Channels.kt:95-102
    ChannelAsFlow(
        std::shared_ptr<channels::ReceiveChannel<T>> channel,
        bool consume,
        std::shared_ptr<CoroutineContext> context = EmptyCoroutineContext::instance(),
        int capacity = channels::CHANNEL_OPTIONAL,
        channels::BufferOverflow on_buffer_overflow = channels::BufferOverflow::SUSPEND)
        : internal::ChannelFlow<T>(context, capacity, on_buffer_overflow),
          channel_(std::move(channel)),
          consume_(consume),
          consumed_(false) {}

private:
    // Transliterated from: kotlinx-coroutines-core/common/src/flow/Channels.kt:104-108
    void mark_consumed() {
        internal::mark_channel_consumed(consume_, consumed_);
    }

protected:
    // Transliterated from: kotlinx-coroutines-core/common/src/flow/Channels.kt:110-111
    internal::ChannelFlow<T>* create(
        std::shared_ptr<CoroutineContext> context,
        int capacity,
        channels::BufferOverflow on_buffer_overflow) override {
        return new ChannelAsFlow(channel_, consume_, context, capacity, on_buffer_overflow);
    }

public:
    // Transliterated from: kotlinx-coroutines-core/common/src/flow/Channels.kt:113-114
    Flow<T>* drop_channel_operators() override {
        return new ChannelAsFlow(channel_, consume_);
    }

protected:
    // Transliterated from: kotlinx-coroutines-core/common/src/flow/Channels.kt:116-117
    void* collect_to(channels::ProducerScope<T>* scope,
                     std::shared_ptr<Continuation<void*>> completion) override {
        std::shared_ptr<FlowCollector<T>> collector = std::make_shared<internal::SendingCollector<T>>(scope);
        return emit_all_impl(std::move(collector), channel_, consume_, std::move(completion));
    }

public:
    // Transliterated from: kotlinx-coroutines-core/common/src/flow/Channels.kt:119-125
    std::shared_ptr<channels::ReceiveChannel<T>> produce_impl(CoroutineScope* scope) override {
        mark_consumed();
        if (this->capacity() == channels::CHANNEL_OPTIONAL) {
            return channel_;
        }
        return internal::ChannelFlow<T>::produce_impl(scope);
    }

    // Transliterated from: kotlinx-coroutines-core/common/src/flow/Channels.kt:127-134
    void* collect(
        FlowCollector<T>* collector,
        Continuation<void*>* completion) override {
        if (this->capacity() == channels::CHANNEL_OPTIONAL) {
            mark_consumed();
            return emit_all_impl(collector, channel_, consume_,
                kotlinx::coroutines::internal::retain_continuation(completion));
        }
        return internal::ChannelFlow<T>::collect(collector, completion);
    }

    // Transliterated from: kotlinx-coroutines-core/common/src/flow/Channels.kt:136-136
    std::optional<std::string> additional_to_string_props() override {
        return std::string("channel=") + std::to_string(
            reinterpret_cast<std::uintptr_t>(channel_.get()));
    }

private:
    std::shared_ptr<channels::ReceiveChannel<T>> channel_;
    bool consume_;
    std::atomic<bool> consumed_;
};

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
// Transliterated from: kotlinx-coroutines-core/common/src/flow/Channels.kt:65-65
template <typename T>
inline std::shared_ptr<Flow<T>> receive_as_flow(
    std::shared_ptr<channels::ReceiveChannel<T>> channel) {
    return std::make_shared<ChannelAsFlow<T>>(std::move(channel), /*consume=*/false);
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
// Transliterated from: kotlinx-coroutines-core/common/src/flow/Channels.kt:87-87
template <typename T>
inline std::shared_ptr<Flow<T>> consume_as_flow(
    std::shared_ptr<channels::ReceiveChannel<T>> channel) {
    return std::make_shared<ChannelAsFlow<T>>(std::move(channel), /*consume=*/true);
}

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
// Transliterated from: kotlinx-coroutines-core/common/src/flow/Channels.kt:154-157
template <typename T>
inline std::shared_ptr<channels::ReceiveChannel<T>> produce_in(
    std::shared_ptr<Flow<T>> flow,
    CoroutineScope* scope) {
    return internal::as_channel_flow(std::move(flow))->produce_impl(scope);
}

} // namespace kotlinx::coroutines::flow
