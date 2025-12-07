#pragma once
#include "BufferedChannel.hpp"
#include "BufferOverflow.hpp"

namespace kotlinx {
namespace coroutines {
namespace channels {

/**
 * This is a special [BufferedChannel] extension that supports [DROP_OLDEST] and [DROP_LATEST]
 * strategies for buffer overflowing. This implementation ensures that `send(e)` never suspends,
 * either extracting the first element ([DROP_OLDEST]) or dropping the sending one ([DROP_LATEST])
 * when the channel capacity exceeds.
 */
template <typename E>
class ConflatedBufferedChannel : public BufferedChannel<E> {
public:
    BufferOverflow onBufferOverflow;

    ConflatedBufferedChannel(int capacity, BufferOverflow onBufferOverflow = BufferOverflow::DROP_OLDEST,
                             OnUndeliveredElement<E> onUndeliveredElement = nullptr)
        : BufferedChannel<E>(capacity, onUndeliveredElement), onBufferOverflow(onBufferOverflow) {
        if (onBufferOverflow == BufferOverflow::SUSPEND) {
            throw std::invalid_argument("ConflatedBufferedChannel does not support SUSPEND strategy");
        }
    }

    void send(E element) override {
        // Never suspends - implement via try_send
        auto result = try_send(element);
        if (result.is_closed()) {
            if (result.exception_or_null()) {
                std::rethrow_exception(result.exception_or_null());
            }
            throw ClosedSendChannelException("Channel was closed");
        }
    }

    ChannelResult<void> try_send(E element) override {
        std::lock_guard<std::mutex> lock(this->mtx);
        if (this->closed_) {
              return ChannelResult<void>::closed(this->closeCause_);
        }

        auto& buffer = this->buffer;
        int cap = this->capacity_;

        if (cap != Channel<E>::UNLIMITED && static_cast<int>(buffer.size()) >= cap) {
            if (onBufferOverflow == BufferOverflow::DROP_LATEST) {
                // Drop this element
                return ChannelResult<void>::success();
            }
            if (onBufferOverflow == BufferOverflow::DROP_OLDEST) {
                // Drop oldest
                if (!buffer.empty()) {
                    buffer.pop_front();
                }
            }
        }

        buffer.push_back(std::move(element));
        this->not_empty.notify_one();
        return ChannelResult<void>::success();
    }
};

} // namespace channels
} // namespace coroutines
} // namespace kotlinx
