/**
 * @file Channel.cpp
 * @brief Implementation of Channel factory and helpers.
 *
 * NOTE: The detailed API documentation, KDocs, and class definitions are located
 * in the companion header file: `include/kotlinx/coroutines/channels/Channel.hpp`.
 */

#include "../../../../include/kotlinx/coroutines/channels/Channel.hpp"
#include "../../../../include/kotlinx/coroutines/channels/BufferedChannel.hpp"
#include "../../../../include/kotlinx/coroutines/channels/ConflatedBufferedChannel.hpp"
#include <memory>
#include <functional>

namespace kotlinx {
namespace coroutines {
namespace channels {

/**
 * Creates a channel with the specified capacity, buffer overflow strategy, and onUndeliveredElement handler.
 *
 * This function implements the complete factory logic from the Kotlin implementation:
 * - RENDEZVOUS: Uses BufferedChannel(0) for SUSPEND, ConflatedBufferedChannel(1) for other strategies
 * - CONFLATED: Always uses ConflatedBufferedChannel(1, DROP_OLDEST)
 * - UNLIMITED: Uses BufferedChannel(UNLIMITED)
 * - BUFFERED: Uses default capacity with SUSPEND, or size 1 with overflow strategy
 * - Custom capacity: Uses BufferedChannel(capacity) for SUSPEND, ConflatedBufferedChannel for other strategies
 */
template <typename E>
std::shared_ptr<Channel<E>> createChannel(
    int capacity,
    BufferOverflow onBufferOverflow,
    OnUndeliveredElement<E> onUndeliveredElement
) {
    switch (capacity) {
        case Channel<E>::RENDEZVOUS: {
            if (onBufferOverflow == BufferOverflow::SUSPEND) {
                // Efficient rendezvous channel implementation
                return std::make_shared<BufferedChannel<E>>(Channel<E>::RENDEZVOUS, onUndeliveredElement);
            } else {
                // Support buffer overflow with conflated channel
                return std::make_shared<ConflatedBufferedChannel<E>>(1, onBufferOverflow, onUndeliveredElement);
            }
        }

        case Channel<E>::CONFLATED: {
            // CONFLATED capacity cannot be used with non-default onBufferOverflow
            if (onBufferOverflow != BufferOverflow::SUSPEND) {
                throw std::invalid_argument("CONFLATED capacity cannot be used with non-default onBufferOverflow");
            }
            return std::make_shared<ConflatedBufferedChannel<E>>(1, BufferOverflow::DROP_OLDEST, onUndeliveredElement);
        }

        case Channel<E>::UNLIMITED: {
            // Ignores onBufferOverflow: it has buffer, but it never overflows
            return std::make_shared<BufferedChannel<E>>(Channel<E>::UNLIMITED, onUndeliveredElement);
        }

        case Channel<E>::BUFFERED: {
            // Uses default capacity with SUSPEND
            if (onBufferOverflow == BufferOverflow::SUSPEND) {
                return std::make_shared<BufferedChannel<E>>(Channel<E>::getDefaultBufferCapacity(), onUndeliveredElement);
            } else {
                return std::make_shared<ConflatedBufferedChannel<E>>(1, onBufferOverflow, onUndeliveredElement);
            }
        }

        default: {
            if (capacity < 0) {
                throw std::invalid_argument("Invalid channel capacity: " + std::to_string(capacity) + ", should be >= 0");
            }

            if (onBufferOverflow == BufferOverflow::SUSPEND) {
                return std::make_shared<BufferedChannel<E>>(capacity, onUndeliveredElement);
            } else {
                return std::make_shared<ConflatedBufferedChannel<E>>(capacity, onBufferOverflow, onUndeliveredElement);
            }
        }
    }
}

// Explicit instantiations for common types
template std::shared_ptr<Channel<int>> createChannel<int>(int, BufferOverflow, OnUndeliveredElement<int>);
template std::shared_ptr<Channel<std::string>> createChannel<std::string>(int, BufferOverflow, OnUndeliveredElement<std::string>);

} // namespace channels
} // namespace coroutines
} // namespace kotlinx
