/**
 * @file Channels.cpp
 * @brief Flow-Channel integration utilities
 *
 * Transliterated from: kotlinx-coroutines-core/common/src/flow/Channels.kt
 *
 * TODO:
 * - Implement suspend/coroutine semantics
 * - Implement Channel operations
 */

#include "kotlinx/coroutines/flow/Flow.hpp"
#include "kotlinx/coroutines/flow/FlowCollector.hpp"
#include "kotlinx/coroutines/channels/Channel.hpp"
#include <exception>

namespace kotlinx {
namespace coroutines {
namespace flow {

using namespace kotlinx::coroutines::channels;

/**
 * Emits all elements from the given channel to this flow collector and cancels (consumes)
 * the channel afterwards. If you need to iterate over the channel without consuming it,
 * a regular `for` loop should be used instead.
 *
 * Note, that emitting values from a channel into a flow is not atomic. A value that was received from the
 * channel many not reach the flow collector if it was cancelled and will be lost.
 *
 * This function provides a more efficient shorthand for `channel.consumeEach { value -> emit(value) }`.
 * See consume_each() on ReceiveChannel.
 */
template<typename T>
void emit_all(FlowCollector<T>* collector, ReceiveChannel<T>* channel) {
    // TODO: Implement suspend function semantics
    emit_all_impl(collector, channel, true);
}

template<typename T>
void emit_all_impl(FlowCollector<T>* collector, ReceiveChannel<T>* channel, bool consume) {
    // TODO: Implement suspend function semantics
    // collector->ensure_active();
    std::exception_ptr cause = nullptr;
    try {
        // TODO: Implement channel iteration
        // In Kotlin: for (element in channel) { collector.emit(element) }
        // In C++ we need to use try_receive() or an async iteration pattern
        while (true) {
            auto result = channel->try_receive();
            if (result.is_closed()) break;
            if (result.is_success()) {
                collector->emit(result.get_or_throw());
            }
            // If failure (empty), we'd normally suspend - for now just break
            break;
        }
    } catch (...) {
        cause = std::current_exception();
        throw;
    }
    if (consume) {
        channel->cancel(cause);
    }
}

/**
 * Represents the given receive channel as a hot flow and receives from the channel
 * in fan-out fashion every time this flow is collected. One element will be emitted to one collector only.
 *
 * See also consume_as_flow() which represents a channel as a flow that consumes the channel.
 */
template<typename T>
std::shared_ptr<Flow<T>> receive_as_flow(ReceiveChannel<T>* channel) {
    // TODO: Implement ChannelAsFlow
    // return std::make_shared<ChannelAsFlow<T>>(channel, false);
    throw std::logic_error("receive_as_flow not yet implemented");
}

/**
 * Represents the given receive channel as a hot flow and consumes the channel
 * on first collection from this flow. The resulting flow can be collected only once.
 *
 * See also receive_as_flow() which represents a channel as a flow without consuming it.
 */
template<typename T>
std::shared_ptr<Flow<T>> consume_as_flow(ReceiveChannel<T>* channel) {
    // TODO: Implement ChannelAsFlow with consume=true
    // return std::make_shared<ChannelAsFlow<T>>(channel, true);
    throw std::logic_error("consume_as_flow not yet implemented");
}

// Explicit template instantiations for common types
template void emit_all<int>(FlowCollector<int>*, ReceiveChannel<int>*);
template void emit_all_impl<int>(FlowCollector<int>*, ReceiveChannel<int>*, bool);

} // namespace flow
} // namespace coroutines
} // namespace kotlinx
