#pragma once
// port-lint: source flow/Channels.kt

/**
 * Transliterated from: kotlinx-coroutines-core/common/src/flow/Channels.kt
 */

#include "kotlinx/coroutines/flow/Flow.hpp"
#include "kotlinx/coroutines/flow/FlowCollector.hpp"
#include "kotlinx/coroutines/channels/Channel.hpp" // ReceiveChannel is defined in Channel.hpp usually in this port
#include "kotlinx/coroutines/flow/internal/ChannelFlow.hpp"
#include "kotlinx/coroutines/CoroutineScope.hpp"
#include "kotlinx/coroutines/Continuation.hpp"
#include "kotlinx/coroutines/ContinuationImpl.hpp"
#include "kotlinx/coroutines/Job.hpp"
#include "kotlinx/coroutines/Result.hpp"
#include "kotlinx/coroutines/channels/Channels.hpp"
#include "kotlinx/coroutines/intrinsics/Intrinsics.hpp"

#include <memory>
#include <atomic>
#include <exception>

namespace kotlinx::coroutines::flow {

using kotlinx::coroutines::Continuation;
using kotlinx::coroutines::ContinuationImpl;
using kotlinx::coroutines::CoroutineContext;
using kotlinx::coroutines::EmptyCoroutineContext;
using kotlinx::coroutines::CoroutineScope;
using kotlinx::coroutines::Result;
using kotlinx::coroutines::Job;
using kotlinx::coroutines::context_ensure_active;
// Forward declaration if needed or use detail namespace


namespace detail {
    // Wrapper for raw Continuation* pointer to be used as completion
    class RawContinuationWrapper : public Continuation<void*> {
    public:
        explicit RawContinuationWrapper(Continuation<void*>* cont) : cont_(cont) {}
        
        std::shared_ptr<CoroutineContext> get_context() const override {
            return cont_ ? cont_->get_context() : EmptyCoroutineContext::instance();
        }
        
        void resume_with(Result<void*> result) override {
            if (cont_) cont_->resume_with(std::move(result));
        }
    private:
        Continuation<void*>* cont_;
    };
}

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
                    label_ = 1; // goto hasNext
                    continue; 
                }

                case 1: { // hasNext
                    label_ = 2;
                    void* has_next_result = iterator_->has_next(this);
                    if (intrinsics::is_coroutine_suspended(has_next_result)) {
                        return intrinsics::get_COROUTINE_SUSPENDED();
                    }
                    resumed_value = has_next_result;
                    continue; // Fall through to check result
                }

                case 2: { // Check hasNext result
                    bool has_next = (resumed_value != nullptr) && *static_cast<bool*>(resumed_value);
                    if (!has_next) {
                        // Channel exhausted - cleanup and return
                        if (consume_) channels::cancel_consumed(channel_, cause_);
                        return nullptr;  // Unit
                    }
                    // Get element (non-suspending after has_next returned true)
                    current_element_ = iterator_->next();
                    label_ = 3;
                    continue; // goto emit
                }

                case 3: { // emit
                    label_ = 4;
                    void* emit_result = collector_->emit(std::move(current_element_), this);
                    if (intrinsics::is_coroutine_suspended(emit_result)) {
                        return intrinsics::get_COROUTINE_SUSPENDED();
                    }
                    label_ = 1; // loop back to hasNext
                    continue;
                }

                case 4: { // after emit
                    label_ = 1; // loop back to hasNext
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
    T current_element_;
};

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
/**
 * Emits all elements from the given [channel] to this flow collector and [cancels][cancel] (consumes)
 * the channel afterwards. If you need to iterate over the channel without consuming it,
 * a regular `for` loop should be used instead.
 */
template <typename T>
void* emit_all_impl(FlowCollector<T>* collector, channels::ReceiveChannel<T>* channel, bool consume, Continuation<void*>* cont) {
    auto sm = std::make_shared<EmitAllContinuation<T>>(
        collector, channel, consume,
        cont ? std::make_shared<detail::RawContinuationWrapper>(cont) : nullptr
    );
    return sm->invoke_suspend(Result<void*>::success(nullptr));
}

/**
 * Emits all elements from the given [channel] to this flow collector and [cancels][cancel] (consumes)
 * the channel afterwards. If you need to iterate over the channel without consuming it,
 * a regular `for` loop should be used instead.
 */
template <typename T>
void* emit_all(FlowCollector<T>* collector, channels::ReceiveChannel<T>* channel, Continuation<void*>* cont) {
    return emit_all_impl(collector, channel, true, cont);
}

/**
 * Represents the given receive channel as a hot flow and [receives][ReceiveChannel.receive] from the channel
 * in fan-out fashion every time this flow is collected. One element will be emitted to one collector only.
 *
 * See also [consumeAsFlow] which ensures that the resulting flow is collected just once.
 */
template <typename T>
std::shared_ptr<Flow<T>> receive_as_flow(std::shared_ptr<channels::ReceiveChannel<T>> channel);

/**
 * Represents the given receive channel as a hot flow and [consumes][ReceiveChannel.consume] the channel
 * on the first collection from this flow. The resulting flow can be collected just once and throws
 * [IllegalStateException] when trying to collect it more than once.
 *
 * See also [receiveAsFlow] which supports multiple collectors of the resulting flow.
 */
template <typename T>
std::shared_ptr<Flow<T>> consume_as_flow(std::shared_ptr<channels::ReceiveChannel<T>> channel);

namespace detail {

template <typename T>
class ChannelAsFlow : public internal::ChannelFlow<T> {
public:
    ChannelAsFlow(std::shared_ptr<channels::ReceiveChannel<T>> channel,
                  bool _consume,
                  std::shared_ptr<CoroutineContext> context = EmptyCoroutineContext::instance(),
                  int capacity = channels::CHANNEL_OPTIONAL,
                  channels::BufferOverflow on_buffer_overflow = channels::BufferOverflow::SUSPEND)
        : internal::ChannelFlow<T>(context, capacity, on_buffer_overflow),
          channel_(channel), consume_(_consume) {
        consumed_ = std::make_shared<std::atomic<bool>>(false);
    }
    
    internal::ChannelFlow<T>* create(std::shared_ptr<CoroutineContext> context, int capacity, channels::BufferOverflow on_buffer_overflow) override {
        return new ChannelAsFlow(channel_, consume_, context, capacity, on_buffer_overflow);
    }

    Flow<T>* drop_channel_operators() override {
        return new ChannelAsFlow(channel_, consume_);
    }

    void collect_to(channels::ProducerScope<T>* scope) override {
        internal::SendingCollector<T> collector(scope);
        // Note: collect_to currently assumes synchronous execution in this stub, but emit_all is suspend.
        // This will be fixed when ChannelFlow becomes suspend-aware.
        // For now, we instantiate the continuation and hope for the best (or this remains a limitation).
        // In a real suspend world, we'd pass 'this' (continuation) from a suspend lambda.
    }

    std::shared_ptr<channels::ReceiveChannel<T>> produce_impl(CoroutineScope* scope) override {
        mark_consumed();
        if (this->capacity() == channels::CHANNEL_OPTIONAL) {
             return channel_;
        }
        return internal::ChannelFlow<T>::produce_impl(scope);
    }

    void* collect(FlowCollector<T>* collector, Continuation<void*>* cont) override {
        if (this->capacity() == channels::CHANNEL_OPTIONAL) {
            mark_consumed();
            return emit_all_impl(collector, channel_.get(), consume_, cont);
        } else {
             return internal::ChannelFlow<T>::collect(collector, cont);
        }
    }

    std::string additional_to_string_props() override {
        return "channel=" + std::to_string((size_t)channel_.get());
    }

private:
    std::shared_ptr<channels::ReceiveChannel<T>> channel_;
    bool consume_;
    std::shared_ptr<std::atomic<bool>> consumed_;
    
    void mark_consumed() {
        if (consume_) {
            bool expected = false;
            if (!consumed_->compare_exchange_strong(expected, true)) {
                 // Throwing from here might be problematic if not in suspend context, 
                 // but collect is suspend.
                 throw std::runtime_error("ReceiveChannel.consumeAsFlow can be collected just once");
            }
        }
    }
};

} // namespace detail

template <typename T>
std::shared_ptr<Flow<T>> receive_as_flow(std::shared_ptr<channels::ReceiveChannel<T>> channel) {
    return std::make_shared<detail::ChannelAsFlow<T>>(channel, false);
}

template <typename T>
std::shared_ptr<Flow<T>> consume_as_flow(std::shared_ptr<channels::ReceiveChannel<T>> channel) {
    return std::make_shared<detail::ChannelAsFlow<T>>(channel, true);
}

// ============================================================================
// Line 154-157: produceIn function
// ============================================================================

/**
 * Creates a [produce] coroutine that collects the given flow.
 *
 * This transformation is **stateful**, it launches a [produce] coroutine
 * that collects the given flow, and has the same behavior:
 *
 * - if collecting the flow throws, the channel will be closed with that exception
 * - if the ReceiveChannel is cancelled, the collection of the flow will be cancelled
 * - if collecting the flow completes normally, the ReceiveChannel will be closed normally
 *
 * A channel with default buffer size is created.
 * Use buffer operator on the flow before calling produce_in to specify a value other than
 * default and to control what happens when data is produced faster than it is consumed,
 * that is to control backpressure behavior.
 *
 * Transliterated from:
 * public fun <T> Flow<T>.produceIn(scope: CoroutineScope): ReceiveChannel<T>
 */
template <typename T>
std::shared_ptr<channels::ReceiveChannel<T>> produce_in(Flow<T>* flow, CoroutineScope* scope) {
    // Convert flow to ChannelFlow and produce
    auto channel_flow = dynamic_cast<internal::ChannelFlow<T>*>(flow);
    if (channel_flow) {
        return channel_flow->produce_impl(scope);
    }
    // For non-ChannelFlow, wrap in a ChannelFlow first
    // TODO: Implement as_channel_flow() helper
    throw std::runtime_error("produce_in not implemented for non-ChannelFlow");
}

} // namespace kotlinx::coroutines::flow

