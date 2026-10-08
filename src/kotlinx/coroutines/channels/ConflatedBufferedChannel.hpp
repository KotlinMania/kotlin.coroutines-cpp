#pragma once
// port-lint: source channels/ConflatedBufferedChannel.kt
/**
 * Transliterated from: kotlinx-coroutines-core/common/src/channels/ConflatedBufferedChannel.kt
 */
//
// Kotlin imports:
// - kotlinx.coroutines.channels.BufferOverflow.*
// - kotlinx.coroutines.channels.ChannelResult.Companion.success
// - kotlinx.coroutines.internal.*
// - kotlinx.coroutines.selects.*

#include "kotlinx/coroutines/channels/BufferedChannel.hpp"
#include "kotlinx/coroutines/channels/BufferOverflow.hpp"
#include "kotlinx/coroutines/internal/OnUndeliveredElement.hpp"

namespace kotlinx {
namespace coroutines {
namespace channels {

/**
 * Line 8-13: This is a special [BufferedChannel] extension that supports [DROP_OLDEST] and [DROP_LATEST]
 * strategies for buffer overflowing. This implementation ensures that `send(e)` never suspends,
 * either extracting the first element ([DROP_OLDEST]) or dropping the sending one ([DROP_LATEST])
 * when the channel capacity exceeds.
 */
// Transliterated from: kotlinx-coroutines-core/common/src/channels/ConflatedBufferedChannel.kt:8-18
template <typename E>
class ConflatedBufferedChannel : public BufferedChannel<E> {
private:
    int capacity_;
    BufferOverflow on_buffer_overflow_;

public:
    // Transliterated from: kotlinx-coroutines-core/common/src/channels/ConflatedBufferedChannel.kt:14-26
    ConflatedBufferedChannel(int capacity, BufferOverflow on_buffer_overflow,
                             OnUndeliveredElement<E> on_undelivered_element = nullptr)
        : BufferedChannel<E>(capacity, on_undelivered_element)
        , capacity_(capacity)
        , on_buffer_overflow_(on_buffer_overflow) {
        if (on_buffer_overflow == BufferOverflow::SUSPEND) {
            throw std::invalid_argument(
                "This implementation does not support suspension for senders, use BufferedChannel instead");
        }
        if (capacity < 1) {
            throw std::invalid_argument(
                "Buffered channel capacity must be at least 1, but " + std::to_string(capacity) + " was specified");
        }
    }

    // Transliterated from: kotlinx-coroutines-core/common/src/channels/ConflatedBufferedChannel.kt:28-29
    bool is_conflated_drop_oldest() const override {
        return on_buffer_overflow_ == BufferOverflow::DROP_OLDEST;
    }

    // Transliterated from: kotlinx-coroutines-core/common/src/channels/ConflatedBufferedChannel.kt:31-40
    void* send(E element, Continuation<void*>* continuation) override {
        auto result = try_send_impl(element, true);
        if (result.is_closed()) {
            if (this->on_undelivered_element_) {
                std::unique_ptr<internal::UndeliveredElementException> exception(
                    internal::call_undelivered_element_catching_exception<E>(this->on_undelivered_element_, element));
                if (exception) {
                    exception->add_suppressed(this->send_exception());
                    throw *exception;
                }
            }
            std::rethrow_exception(this->send_exception());
        }
        (void)continuation;
        return nullptr;
    }

    // Transliterated from: kotlinx-coroutines-core/common/src/channels/ConflatedBufferedChannel.kt:42-47
    void* send_broadcast(E element, Continuation<void*>* continuation) override {
        // Should never suspend, implement via `trySend(..)`.
        auto result = try_send_impl(std::move(element), true);
        if (result.is_success()) {
            (void)continuation;
            return new bool(true);  // Boxing
        }
        return new bool(false);
    }

    // Transliterated from: kotlinx-coroutines-core/common/src/channels/ConflatedBufferedChannel.kt:49-49
    ChannelResult<void> try_send(E element) override {
        return try_send_impl(std::move(element), false);
    }

    // Transliterated from: kotlinx-coroutines-core/common/src/channels/ConflatedBufferedChannel.kt:88-88
    bool should_send_suspend() const override {
        return false;  // never suspends
    }

private:
    // Transliterated from: kotlinx-coroutines-core/common/src/channels/ConflatedBufferedChannel.kt:51-53
    ChannelResult<void> try_send_impl(E element, bool is_send_op) {
        if (on_buffer_overflow_ == BufferOverflow::DROP_LATEST) {
            return try_send_drop_latest(std::move(element), is_send_op);
        } else {
            return this->try_send_drop_oldest(std::move(element));
        }
    }

    // Transliterated from: kotlinx-coroutines-core/common/src/channels/ConflatedBufferedChannel.kt:55-69
    ChannelResult<void> try_send_drop_latest(E element, bool is_send_op) {
        // Try to send the element without suspension.
        auto result = BufferedChannel<E>::try_send(element);
        // Complete on success or if this channel is closed.
        if (result.is_success() || result.is_closed()) return result;
        // This channel is full. Drop the sending element.
        // Call the `onUndeliveredElement` lambda ONLY for 'send()' invocations,
        // for 'trySend()' it is responsibility of the caller
        if (is_send_op && this->on_undelivered_element_) {
            // Upstream: onUndeliveredElement?.callUndeliveredElementCatchingException(element)
            // The C++ port uses the same helper; trySend callers pass false for is_send_op
            // because the upstream contract makes them responsible for calling the
            // undelivered-element hook themselves.
            std::unique_ptr<internal::UndeliveredElementException> exception(
                internal::call_undelivered_element_catching_exception<E>(this->on_undelivered_element_, element));
            if (exception) throw *exception;
        }
        return ChannelResult<void>::success();
    }

protected:
    // Transliterated from: kotlinx-coroutines-core/common/src/channels/ConflatedBufferedChannel.kt:72-86
    void register_select_for_send(selects::SelectInstance<void*>* select, void* element) override {
        auto result = try_send(*static_cast<E*>(element));
        if (result.is_success()) {
            select->select_in_registration_phase(nullptr);
            return;
        }
        if (result.is_closed()) {
            select->select_in_registration_phase(&CHANNEL_CLOSED());
            return;
        }
        throw std::logic_error("unreachable");
    }

};

} // namespace channels
} // namespace coroutines
} // namespace kotlinx
