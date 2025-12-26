#pragma once
/**
 * @file Channels.hpp
 * @brief Flow-Channel integration utilities.
 *
 * Transliterated from: kotlinx-coroutines-core/common/src/flow/Channels.kt
 *
 * Provides conversion between flows and channels:
 * - emit_all() - emit all channel elements into a flow collector
 * - receive_as_flow() - represent channel as a hot flow (fan-out)
 * - consume_as_flow() - represent channel as a one-time consumable flow
 * - produce_in() - create a produce coroutine that collects the given flow
 *
 * Private implementations (emit_all_impl, ChannelAsFlow) are exposed due to
 * template requirements but should not be used directly.
 */

#include "kotlinx/coroutines/Continuation.hpp"
#include "kotlinx/coroutines/ContinuationImpl.hpp"
#include "kotlinx/coroutines/Job.hpp"
#include "kotlinx/coroutines/CoroutineScope.hpp"
#include "kotlinx/coroutines/Result.hpp"
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

// Forward declarations to break circular dependency
namespace kotlinx::coroutines::flow::internal {
template <typename T> class ChannelFlow;
template <typename T> class SendingCollector;
} // namespace kotlinx::coroutines::flow::internal

namespace kotlinx::coroutines::flow {

/**
 * Emits all elements from the given channel to this flow collector and cancels (consumes)
 * the channel afterwards. If you need to iterate over the channel without consuming it,
 * a regular loop should be used instead.
 *
 * Note that emitting values from a channel into a flow is not atomic. A value that was
 * received from the channel may not reach the flow collector if it was cancelled and
 * will be lost.
 *
 * This function provides a more efficient shorthand for:
 *   channel.consumeEach { value -> emit(value) }
 *
 * @see consume_each
 */
template <typename T>
void* emit_all(
    FlowCollector<T>* collector,
    channels::ReceiveChannel<T>* channel,
    Continuation<void*>* cont
);  // Forward declaration, defined after emit_all_impl

/**
 * State machine for emit_all_impl that properly suspends on channel receive.
 *
 * Transliterates the Kotlin:
 *   for (element in channel) { emit(element) }
 *
 * Which desugars to:
 *   val iterator = channel.iterator()
 *   while (iterator.hasNext()) {  // suspends
 *       emit(iterator.next())     // suspends
 *   }
 */
template <typename T>
class EmitAllContinuation : public ContinuationImpl {
public:
    EmitAllContinuation(
        FlowCollector<T>* collector,
        channels::ReceiveChannel<T>* channel,
        bool consume,
        std::shared_ptr<Continuation<void*>> completion
    ) : ContinuationImpl(std::move(completion)),
        collector_(collector),
        channel_(channel),
        consume_(consume),
        label_(0),
        cause_(nullptr) {}

    void* invoke_suspend(Result<void*> result) override {
        // Store result from previous suspension point for case 2/4 resumption
        void* resumed_value = result.is_success() ? result.get_or_throw() : nullptr;

        // Check for exception from previous suspension point
        if (result.is_failure()) {
            cause_ = result.exception_or_null();
            if (consume_) channels::cancel_consumed(channel_, cause_);
            std::rethrow_exception(cause_);
        }

        // State machine loop - handles both initial entry and resumption
        while (true) {
            switch (label_) {
                case 0: {
                    // Initial: ensureActive() and get iterator
                    if (this->completion) {
                        auto ctx = this->completion->get_context();
                        if (ctx) context_ensure_active(*ctx);
                    }
                    iterator_ = channel_->iterator();
                    label_ = 1;
                    continue;  // Fall through to case 1
                }

                case 1: {
                    // Call has_next() - may suspend
                    label_ = 2;
                    void* has_next_result = iterator_->has_next(this);
                    if (intrinsics::is_coroutine_suspended(has_next_result)) {
                        return intrinsics::get_COROUTINE_SUSPENDED();
                    }
                    // Unbox bool result
                    resumed_value = has_next_result;
                    continue;  // Fall through to case 2
                }

                case 2: {
                    // Resumed from has_next() - check result
                    has_next_value_ = (resumed_value != nullptr) &&
                                      *static_cast<bool*>(resumed_value);
                    if (!has_next_value_) {
                        // Channel exhausted - cleanup and return
                        if (consume_) channels::cancel_consumed(channel_, cause_);
                        return nullptr;  // Unit
                    }
                    // Get element (non-suspending after has_next returned true)
                    current_element_ = iterator_->next();
                    label_ = 3;
                    continue;  // Fall through to case 3
                }

                case 3: {
                    // Emit element - may suspend
                    label_ = 4;
                    void* emit_result = collector_->emit(std::move(current_element_), this);
                    if (intrinsics::is_coroutine_suspended(emit_result)) {
                        return intrinsics::get_COROUTINE_SUSPENDED();
                    }
                    // Emit completed synchronously
                    label_ = 1;
                    continue;  // Loop back to has_next
                }

                case 4: {
                    // Resumed from emit() - loop back to has_next
                    label_ = 1;
                    continue;
                }

                default:
                    throw std::logic_error("Invalid state in EmitAllContinuation");
            }
        }
    }

private:
    FlowCollector<T>* collector_;
    channels::ReceiveChannel<T>* channel_;
    bool consume_;
    int label_;
    std::exception_ptr cause_;
    std::unique_ptr<channels::ChannelIterator<T>> iterator_;
    bool has_next_value_;
    T current_element_;
};

/**
 * Internal implementation with configurable "consume" behavior.
 * WARNING: Private implementation - do not use directly.
 *
 * Transliterated from: private suspend fun <T> FlowCollector<T>.emitAllImpl(
 *     channel: ReceiveChannel<T>, consume: Boolean
 * )
 */
template <typename T>
void* emit_all_impl(
    FlowCollector<T>* collector,
    channels::ReceiveChannel<T>* channel,
    bool consume,
    Continuation<void*>* cont
) {
    // Create state machine and start it
    auto sm = std::make_shared<EmitAllContinuation<T>>(
        collector, channel, consume,
        cont ? std::shared_ptr<Continuation<void*>>(cont, [](Continuation<void*>*){})
             : nullptr
    );
    return sm->invoke_suspend(Result<void*>::success(nullptr));
}

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
 * Represents an existing channel as ChannelFlow implementation.
 * WARNING: Private class - use receive_as_flow() or consume_as_flow() instead.
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
        return new ChannelAsFlow<T>(channel_, consume_);
    }

    void collect_to(channels::ProducerScope<T>* scope) override {
        internal::SendingCollector<T> sending(scope);
        (void)emit_all_impl<T>(&sending, channel_.get(), consume_, nullptr);
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
    std::atomic<bool> consumed_{false};

    void mark_consumed() {
        if (!consume_) return;
        bool already = consumed_.exchange(true, std::memory_order_acq_rel);
        if (already) {
            throw std::logic_error("ReceiveChannel.consumeAsFlow can be collected just once");
        }
    }

    std::shared_ptr<channels::ReceiveChannel<T>> channel_;
    bool consume_;
};

/**
 * Represents the given receive channel as a hot flow and receives from the channel
 * in fan-out fashion every time this flow is collected. One element will be emitted
 * to one collector only.
 *
 * See also consume_as_flow() which ensures that the resulting flow is collected just once.
 *
 * ### Cancellation semantics
 *
 * - Flow collectors are cancelled when the original channel is closed with an exception.
 * - Flow collectors complete normally when the original channel is closed normally.
 * - Failure or cancellation of the flow collector does not affect the channel.
 *   However, if a flow collector gets cancelled after receiving an element from the
 *   channel but before starting to process it, the element will be lost, and the
 *   on_undelivered_element callback of the Channel, if provided on channel construction,
 *   will be invoked.
 *
 * ### Operator fusion
 *
 * Adjacent applications of flow_on, buffer, conflate, and produce_in to the result of
 * receive_as_flow are fused. In particular, produce_in returns the original channel.
 * Calls to flow_on have generally no effect, unless buffer is used to explicitly
 * request buffering.
 */
template <typename T>
inline std::shared_ptr<Flow<T>> receive_as_flow(std::shared_ptr<channels::ReceiveChannel<T>> channel) {
    return std::make_shared<ChannelAsFlow<T>>(std::move(channel), false);
}

/**
 * Represents the given receive channel as a hot flow and consumes the channel
 * on the first collection from this flow. The resulting flow can be collected just once
 * and throws std::logic_error when trying to collect it more than once.
 *
 * See also receive_as_flow() which supports multiple collectors of the resulting flow.
 *
 * ### Cancellation semantics
 *
 * - Flow collector is cancelled when the original channel is closed with an exception.
 * - Flow collector completes normally when the original channel is closed normally.
 * - If the flow collector fails with an exception (for example, by getting cancelled),
 *   the source channel is cancelled.
 *
 * ### Operator fusion
 *
 * Adjacent applications of flow_on, buffer, conflate, and produce_in to the result of
 * consume_as_flow are fused. In particular, produce_in returns the original channel
 * (but throws std::logic_error on repeated calls). Calls to flow_on have generally no
 * effect, unless buffer is used to explicitly request buffering.
 */
template <typename T>
inline std::shared_ptr<Flow<T>> consume_as_flow(std::shared_ptr<channels::ReceiveChannel<T>> channel) {
    return std::make_shared<ChannelAsFlow<T>>(std::move(channel), true);
}

/**
 * Creates a produce coroutine that collects the given flow.
 *
 * This transformation is stateful, it launches a produce coroutine that collects
 * the given flow, and has the same behavior:
 *
 * - If collecting the flow throws, the channel will be closed with that exception.
 * - If the ReceiveChannel is cancelled, the collection of the flow will be cancelled.
 * - If collecting the flow completes normally, the ReceiveChannel will be closed normally.
 *
 * A channel with default buffer size is created. Use buffer operator on the flow before
 * calling produce_in to specify a value other than default and to control what happens
 * when data is produced faster than it is consumed, that is to control backpressure behavior.
 */
template <typename T>
std::shared_ptr<channels::ReceiveChannel<T>> produce_in(
    std::shared_ptr<Flow<T>> flow,
    CoroutineScope* scope
);  // Defined in ChannelFlow.hpp after as_channel_flow

} // namespace kotlinx::coroutines::flow
