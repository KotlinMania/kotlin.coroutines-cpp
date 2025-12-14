#pragma once
// Transliterated from: kotlinx-coroutines-core/common/src/channels/ConflatedBufferedChannel.kt
//
// Kotlin imports:
// - kotlinx.coroutines.channels.BufferOverflow.*
// - kotlinx.coroutines.channels.ChannelResult.Companion.success
// - kotlinx.coroutines.internal.*
// - kotlinx.coroutines.selects.*

#include "kotlinx/coroutines/channels/BufferedChannel.hpp"
#include "kotlinx/coroutines/channels/BufferOverflow.hpp"

namespace kotlinx {
namespace coroutines {
namespace channels {

/**
 * Line 8-13: This is a special [BufferedChannel] extension that supports [DROP_OLDEST] and [DROP_LATEST]
 * strategies for buffer overflowing. This implementation ensures that `send(e)` never suspends,
 * either extracting the first element ([DROP_OLDEST]) or dropping the sending one ([DROP_LATEST])
 * when the channel capacity exceeds.
 */
template <typename E>
class ConflatedBufferedChannel : public BufferedChannel<E> {
private:
    // Line 15: private val capacity: Int
    int conflated_capacity_;
    // Line 16: private val onBufferOverflow: BufferOverflow
    BufferOverflow on_buffer_overflow_;

public:
    // Line 14-18: constructor
    ConflatedBufferedChannel(int capacity, BufferOverflow on_buffer_overflow = BufferOverflow::DROP_OLDEST,
                             OnUndeliveredElement<E> on_undelivered_element = nullptr)
        : BufferedChannel<E>(capacity, on_undelivered_element)
        , conflated_capacity_(capacity)
        , on_buffer_overflow_(on_buffer_overflow) {
        // Line 19-26: init block
        if (on_buffer_overflow == BufferOverflow::SUSPEND) {
            throw std::invalid_argument(
                "This implementation does not support suspension for senders, use BufferedChannel instead");
        }
        if (capacity < 1) {
            throw std::invalid_argument(
                "Buffered channel capacity must be at least 1, but " + std::to_string(capacity) + " was specified");
        }
    }

    // Line 28-29: override val isConflatedDropOldest: Boolean
    bool is_conflated_drop_oldest() const override {
        return on_buffer_overflow_ == BufferOverflow::DROP_OLDEST;
    }

    // Line 31-40: override suspend fun send(element: E)
    void* send(E element, Continuation<void*>* continuation) override {
        // Should never suspend, implement via `trySend(..)`.
        auto result = try_send_impl(std::move(element), true);
        if (result.is_closed()) {
            // fails only when this channel is closed
            auto cause = this->send_exception();
            if (this->on_undelivered_element_) {
                // TODO: callUndeliveredElementCatchingException
            }
            if (cause) std::rethrow_exception(cause);
            throw ClosedSendChannelException("Channel was closed");
        }
        (void)continuation;  // Never suspends
        return nullptr;
    }

    // Line 42-47: override suspend fun sendBroadcast(element: E): Boolean
    void* send_broadcast(E element, Continuation<void*>* continuation) override {
        // Should never suspend, implement via `trySend(..)`.
        auto result = try_send_impl(std::move(element), true);
        if (result.is_success()) {
            (void)continuation;
            return new bool(true);  // Boxing
        }
        return new bool(false);
    }

    // Line 49: override fun trySend(element: E): ChannelResult<Unit>
    ChannelResult<void> try_send(E element) override {
        return try_send_impl(std::move(element), false);
    }

    // Line 88: override fun shouldSendSuspend() = false
    bool should_send_suspend() const override {
        return false;  // never suspends
    }

private:
    // Line 51-53: private fun trySendImpl(element: E, isSendOp: Boolean)
    ChannelResult<void> try_send_impl(E element, bool is_send_op) {
        if (on_buffer_overflow_ == BufferOverflow::DROP_LATEST) {
            return try_send_drop_latest(std::move(element), is_send_op);
        } else {
            return try_send_drop_oldest(std::move(element));
        }
    }

    // Line 55-69: private fun trySendDropLatest(element: E, isSendOp: Boolean): ChannelResult<Unit>
    ChannelResult<void> try_send_drop_latest(E element, bool is_send_op) {
        // Try to send the element without suspension.
        auto result = BufferedChannel<E>::try_send(element);
        // Complete on success or if this channel is closed.
        if (result.is_success() || result.is_closed()) return result;
        // This channel is full. Drop the sending element.
        // Call the `onUndeliveredElement` lambda ONLY for 'send()' invocations,
        // for 'trySend()' it is responsibility of the caller
        if (is_send_op && this->on_undelivered_element_) {
            // TODO: callUndeliveredElementCatchingException
        }
        return ChannelResult<void>::success();
    }

    // Line 71-86: trySendDropOldest - calls into BufferedChannel's existing implementation
    // The DROP_OLDEST logic is handled by isConflatedDropOldest() being true
    ChannelResult<void> try_send_drop_oldest(E element) {
        // When isConflatedDropOldest is true, BufferedChannel handles dropping
        return BufferedChannel<E>::try_send(std::move(element));
    }
};

} // namespace channels
} // namespace coroutines
} // namespace kotlinx
