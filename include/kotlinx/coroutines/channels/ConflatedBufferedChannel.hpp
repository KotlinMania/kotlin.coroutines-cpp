#pragma once
#include "kotlinx/coroutines/channels/BufferedChannel.hpp"

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
    enum class BufferOverflow {
        SUSPEND,
        DROP_OLDEST,
        DROP_LATEST
    };

    BufferOverflow onBufferOverflow;

    ConflatedBufferedChannel(int capacity, BufferOverflow onBufferOverflow = BufferOverflow::DROP_OLDEST) 
        : BufferedChannel<E>(capacity), onBufferOverflow(onBufferOverflow) {
        if (onBufferOverflow == BufferOverflow::SUSPEND) {
            throw std::invalid_argument("ConflatedBufferedChannel does not support SUSPEND strategy");
        }
    }

    void send(E element) override {
        // Never suspends
        try_send(element);
    }

    ChannelResult<void> try_send(E element) override {
        std::lock_guard<std::mutex> lock(BufferedChannel<E>::mtx);
        if (BufferedChannel<E>::closed) {
             return ChannelResult<void>::closed_result(BufferedChannel<E>::close_cause);
        }

        auto& buffer = BufferedChannel<E>::buffer;
        int cap = BufferedChannel<E>::capacity;

        if (cap != Channel<E>::UNLIMITED && buffer.size() >= (size_t)cap) {
            if (onBufferOverflow == BufferOverflow::DROP_LATEST) {
                // Drop this element
                return ChannelResult<void>::success(nullptr);
            }
            if (onBufferOverflow == BufferOverflow::DROP_OLDEST) {
                // Drop oldest
                if (!buffer.empty()) {
                    buffer.pop_front();
                }
            }
        }
        
        buffer.push_back(element);
        BufferedChannel<E>::not_empty.notify_one();
        return ChannelResult<void>::success(nullptr);
    }
};

} // namespace channels
} // namespace coroutines
} // namespace kotlinx
