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
#include "kotlinx/coroutines/ContinuationImpl.hpp"
#include "kotlinx/coroutines/CoroutineScope.hpp"
#include "kotlinx/coroutines/Job.hpp"
#include "kotlinx/coroutines/Result.hpp"
#include "kotlinx/coroutines/channels/BufferOverflow.hpp"
#include "kotlinx/coroutines/channels/Channel.hpp"
#include "kotlinx/coroutines/channels/Channels.hpp"
#include "kotlinx/coroutines/dsl/Suspend.hpp"
#include "kotlinx/coroutines/flow/Flow.hpp"
#include "kotlinx/coroutines/flow/FlowCollector.hpp"

namespace kotlinx::coroutines::flow {

template <typename T>
void* emit_all(
    FlowCollector<T>* receiver,
    channels::ReceiveChannel<T>* channel,
    Continuation<void*>* completion);

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
#include <memory>
#include <optional>
#include <string>

namespace kotlinx::coroutines::flow {

template <typename T>
class ChannelAsFlow;

/**
 * Coroutine state machine for emitAllImpl.
 *
 * Transliterated from: kotlinx-coroutines-core/common/src/flow/Channels.kt:28-41
 */
template <typename T>
class EmitAllContinuation : public ContinuationImpl {
public:
    void* _label = nullptr;
    std::shared_ptr<FlowCollector<T>> receiver_shared_;
    FlowCollector<T>* receiver_;
    channels::ReceiveChannel<T>* channel_;
    bool consume_;
    std::unique_ptr<channels::ChannelIterator<T>> iterator_;
    std::optional<T> element_;
    void* has_next_box_ = nullptr;
    void* emit_box_ = nullptr;
    bool has_next_ = false;
    std::exception_ptr cause_ = nullptr;
    std::shared_ptr<EmitAllContinuation<T>> keep_alive_;

    EmitAllContinuation(
        FlowCollector<T>* receiver,
        channels::ReceiveChannel<T>* channel,
        bool consume,
        std::shared_ptr<Continuation<void*>> completion)
        : ContinuationImpl(std::move(completion)),
          receiver_shared_(nullptr),
          receiver_(receiver),
          channel_(channel),
          consume_(consume) {
        (void)consume;
    }

    EmitAllContinuation(
        std::shared_ptr<FlowCollector<T>> receiver,
        channels::ReceiveChannel<T>* channel,
        bool consume,
        std::shared_ptr<Continuation<void*>> completion)
        : ContinuationImpl(std::move(completion)),
          receiver_shared_(receiver),
          receiver_(receiver ? receiver.get() : nullptr),
          channel_(channel),
          consume_(consume) {
        (void)consume;
    }

    void retain() {
        keep_alive_ = std::static_pointer_cast<EmitAllContinuation<T>>(shared_from_this());
    }

    void release() {
        auto self = std::move(keep_alive_);
        receiver_shared_ = nullptr;
    }

    void release_intercepted() override {
        ContinuationImpl::release_intercepted();
        release();
    }

    void* invoke_suspend(Result<void*> result) override {
        try {
            coroutine_begin(this)

            if (completion) {
                auto ctx = completion->get_context();
                if (ctx) context_ensure_active(*ctx);
            }

            iterator_ = channel_->iterator();
            while (true) {
                // Suspend point 1: has_next
                coroutine_yield_value(this, result, iterator_->has_next(this), has_next_box_);

                has_next_ = false;
                if (has_next_box_) {
                    auto* b = static_cast<bool*>(has_next_box_);
                    has_next_ = *b;
                    delete b;
                    has_next_box_ = nullptr;
                }

                if (!has_next_) {
                    break;
                }

                element_.emplace(iterator_->next());

                // Suspend point 2: emit
                coroutine_yield_value(this, result, receiver_->emit(std::move(*element_), this), emit_box_);
                element_.reset();
            }
        } catch (...) {
            cause_ = std::current_exception();
            if (consume_) {
                channels::cancel_consumed(channel_, cause_);
            }
            element_.reset();
            std::rethrow_exception(cause_);
        }

        if (consume_) {
            channels::cancel_consumed(channel_, cause_);
        }
        element_.reset();

        coroutine_end(this)
    }
};

/**
 * Private helper. Iterates the channel and emits to the collector; cancels the channel
 * on the way out when `consume` is true. Mirrors the upstream `emitAllImpl` private function.
 *
 * Transliterated from: kotlinx-coroutines-core/common/src/flow/Channels.kt:28-41
 */
template <typename T>
inline void* emit_all_impl(
    FlowCollector<T>* receiver,
    channels::ReceiveChannel<T>* channel,
    bool consume,
    Continuation<void*>* completion) {
    auto completion_shared = completion
        ? std::shared_ptr<Continuation<void*>>(completion, [](Continuation<void*>*){})
        : nullptr;
    auto coro = std::make_shared<EmitAllContinuation<T>>(
        receiver, channel, consume, std::move(completion_shared));
    coro->retain();
    void* res = nullptr;
    try {
        res = coro->invoke_suspend(Result<void*>::success(nullptr));
        if (res != intrinsics::get_COROUTINE_SUSPENDED()) {
            coro->release();
        }
    } catch (...) {
        coro->release();
        throw;
    }
    return res;
}

template <typename T>
inline void* emit_all_impl(
    std::shared_ptr<FlowCollector<T>> receiver,
    channels::ReceiveChannel<T>* channel,
    bool consume,
    Continuation<void*>* completion) {
    auto completion_shared = completion
        ? std::shared_ptr<Continuation<void*>>(completion, [](Continuation<void*>*){})
        : nullptr;
    auto coro = std::make_shared<EmitAllContinuation<T>>(
        std::move(receiver), channel, consume, std::move(completion_shared));
    coro->retain();
    void* res = nullptr;
    try {
        res = coro->invoke_suspend(Result<void*>::success(nullptr));
        if (res != intrinsics::get_COROUTINE_SUSPENDED()) {
            coro->release();
        }
    } catch (...) {
        coro->release();
        throw;
    }
    return res;
}

/**
 * Emits all elements from the given [channel] to this flow collector and [cancels][cancel] (consumes)
 * the channel afterwards. If you need to iterate over the channel without consuming it,
 * a regular `for` loop should be used instead.
 *
 * Transliterated from: kotlinx-coroutines-core/common/src/flow/Channels.kt:25-26
 */
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
 *
 * Transliterated from: kotlinx-coroutines-core/common/src/flow/Channels.kt:95-137
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
          consumed_(std::make_shared<std::atomic<bool>>(false)) {
        (void)consume;
    }

    // Transliterated from: kotlinx-coroutines-core/common/src/flow/Channels.kt:104-108
    void mark_consumed() {
        if (consume_) {
            bool expected = false;
            if (!consumed_->compare_exchange_strong(expected, true)) {
                throw std::logic_error(
                    "ReceiveChannel.consumeAsFlow can be collected just once");
            }
        }
    }

    // Transliterated from: kotlinx-coroutines-core/common/src/flow/Channels.kt:110-111
    internal::ChannelFlow<T>* create(
        std::shared_ptr<CoroutineContext> context,
        int capacity,
        channels::BufferOverflow on_buffer_overflow) override {
        return new ChannelAsFlow(channel_, consume_, context, capacity, on_buffer_overflow);
    }

    // Transliterated from: kotlinx-coroutines-core/common/src/flow/Channels.kt:113-114
    Flow<T>* drop_channel_operators() override {
        return new ChannelAsFlow(channel_, consume_);
    }

    // Transliterated from: kotlinx-coroutines-core/common/src/flow/Channels.kt:116-117
    void collect_to(channels::ProducerScope<T>* scope) override {
        std::shared_ptr<FlowCollector<T>> collector = std::make_shared<internal::SendingCollector<T>>(scope);
        (void)emit_all_impl(std::move(collector), channel_.get(), consume_, nullptr);
    }

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
            return emit_all_impl(collector, channel_.get(), consume_, completion);
        }
        return internal::ChannelFlow<T>::collect(collector, completion);
    }

    // Transliterated from: kotlinx-coroutines-core/common/src/flow/Channels.kt:136
    std::string additional_to_string_props() override {
        return std::string("channel=") + std::to_string(
            reinterpret_cast<std::uintptr_t>(channel_.get()));
    }

private:
    std::shared_ptr<channels::ReceiveChannel<T>> channel_;
    bool consume_;
    std::shared_ptr<std::atomic<bool>> consumed_;
};

/**
 * Represents the given receive channel as a hot flow and [receives][ReceiveChannel.receive] from the channel
 * in fan-out fashion every time this flow is collected. One element will be emitted to one collector only.
 *
 * Transliterated from: kotlinx-coroutines-core/common/src/flow/Channels.kt:65
 */
template <typename T>
inline std::shared_ptr<Flow<T>> receive_as_flow(
    std::shared_ptr<channels::ReceiveChannel<T>> channel) {
    return std::make_shared<ChannelAsFlow<T>>(std::move(channel), /*consume=*/false);
}

/**
 * Represents the given receive channel as a hot flow and [consumes][ReceiveChannel.consume] the channel
 * on the first collection from this flow. The resulting flow can be collected just once.
 *
 * Transliterated from: kotlinx-coroutines-core/common/src/flow/Channels.kt:87
 */
template <typename T>
inline std::shared_ptr<Flow<T>> consume_as_flow(
    std::shared_ptr<channels::ReceiveChannel<T>> channel) {
    return std::make_shared<ChannelAsFlow<T>>(std::move(channel), /*consume=*/true);
}

/**
 * Creates a [produce] coroutine that collects the given flow.
 *
 * Transliterated from: kotlinx-coroutines-core/common/src/flow/Channels.kt:154-158
 */
template <typename T>
inline std::shared_ptr<channels::ReceiveChannel<T>> produce_in(
    std::shared_ptr<Flow<T>> flow,
    CoroutineScope* scope) {
    return internal::as_channel_flow(std::move(flow))->produce_impl(scope);
}

} // namespace kotlinx::coroutines::flow
