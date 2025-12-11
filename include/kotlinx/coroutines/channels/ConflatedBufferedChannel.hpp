#pragma once
#include "BufferedChannel.hpp"
#include "BufferOverflow.hpp"

namespace kotlinx {
namespace coroutines {
namespace channels {

/**
 * This is a special [BufferedChannel] extension that supports [DROP_OLDEST] and [DROP_LATEST]
 * strategies for buffer overflowing. This implementation ensures that `send(e)` never suspends.
 */
template <typename E>
class ConflatedBufferedChannel : public BufferedChannel<E> {
public:
    BufferOverflow on_buffer_overflow;

    ConflatedBufferedChannel(int capacity, BufferOverflow on_buffer_overflow = BufferOverflow::DROP_OLDEST,
                             OnUndeliveredElement<E> onUndeliveredElement = nullptr)
        : BufferedChannel<E>(capacity, onUndeliveredElement), on_buffer_overflow(on_buffer_overflow) {
        if (on_buffer_overflow == BufferOverflow::SUSPEND) {
            throw std::invalid_argument("ConflatedBufferedChannel does not support SUSPEND strategy");
        }
    }

    ChannelAwaiter<void> send(E element) override {
        // Never suspends - implement via try_send
        auto result = try_send(element);
        if (result.is_closed()) {
            if (result.exception_or_null()) {
                std::rethrow_exception(result.exception_or_null());
            }
            throw ClosedSendChannelException("Channel was closed");
        }
        return ChannelAwaiter<void>(); // Ready
    }

    ChannelResult<void> try_send(E element) override {
        std::unique_lock<std::mutex> lock(this->mtx);
        if (this->closed_) {
              return ChannelResult<void>::closed(this->close_cause_);
        }

        auto& buffer = this->buffer_;
        int cap = this->capacity_;

        if (cap != Channel<E>::UNLIMITED && static_cast<int>(buffer.size()) >= cap) {
            if (on_buffer_overflow == BufferOverflow::DROP_LATEST) {
                // Drop this element
                return ChannelResult<void>::success();
            }
            if (on_buffer_overflow == BufferOverflow::DROP_OLDEST) {
                // Drop oldest
                if (!buffer.empty()) {
                    buffer.pop_front();
                }
            }
        }

        buffer.push_back(std::move(element));
        return ChannelResult<void>::success();
    }
};

} // namespace channels
} // namespace coroutines
} // namespace kotlinx
