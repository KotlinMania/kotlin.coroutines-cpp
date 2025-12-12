#pragma once
#include "kotlinx/coroutines/channels/Channel.hpp"
#include "kotlinx/coroutines/channels/BufferedChannel.hpp"
#include "kotlinx/coroutines/channels/ConflatedBufferedChannel.hpp"
#include <functional>
#include <vector>
#include <exception>

namespace kotlinx {
namespace coroutines {
namespace channels {

// Implementation of createChannel factory
template <typename E>
std::shared_ptr<Channel<E>> createChannel(
    int capacity,
    BufferOverflow onBufferOverflow,
    OnUndeliveredElement<E> onUndeliveredElement
) {
    using namespace kotlinx::coroutines::channels; // For constants if needed, though they are in Channel<E>
    
    // Constants from Channel<E>
    int RENDEZVOUS = Channel<E>::RENDEZVOUS;
    int CONFLATED = Channel<E>::CONFLATED;
    int UNLIMITED = Channel<E>::UNLIMITED;
    int BUFFERED = Channel<E>::BUFFERED;
    int DEFAULT = Channel<E>::getDefaultBufferCapacity();

    if (capacity == RENDEZVOUS) {
        if (onBufferOverflow == BufferOverflow::SUSPEND) {
            return std::make_shared<BufferedChannel<E>>(RENDEZVOUS, onUndeliveredElement);
        } else {
            return std::make_shared<ConflatedBufferedChannel<E>>(1, onBufferOverflow, onUndeliveredElement);
        }
    } else if (capacity == CONFLATED) {
        if (onBufferOverflow != BufferOverflow::SUSPEND) {
             throw std::invalid_argument("CONFLATED capacity cannot be used with non-default onBufferOverflow");
        }
        return std::make_shared<ConflatedBufferedChannel<E>>(1, BufferOverflow::DROP_OLDEST, onUndeliveredElement);
    } else if (capacity == UNLIMITED) {
        return std::make_shared<BufferedChannel<E>>(UNLIMITED, onUndeliveredElement);
    } else if (capacity == BUFFERED) {
        if (onBufferOverflow == BufferOverflow::SUSPEND) {
            return std::make_shared<BufferedChannel<E>>(DEFAULT, onUndeliveredElement);
        } else {
            return std::make_shared<ConflatedBufferedChannel<E>>(1, onBufferOverflow, onUndeliveredElement);
        }
    } else {
        if (onBufferOverflow == BufferOverflow::SUSPEND) {
            return std::make_shared<BufferedChannel<E>>(capacity, onUndeliveredElement);
        } else {
             return std::make_shared<ConflatedBufferedChannel<E>>(capacity, onBufferOverflow, onUndeliveredElement);
        }
    }
}

// Helper for consume (RAII)
template <typename E>
struct ConsumedChannelGuard {
    ReceiveChannel<E>* channel;
    std::exception_ptr& cause;

    ConsumedChannelGuard(ReceiveChannel<E>* c, std::exception_ptr& ex) : channel(c), cause(ex) {}
    ~ConsumedChannelGuard() {
        // Simple cancellation mapping for now
         // TODO: properly extract cause if possible or default
        channel->cancel(cause);
    }
};

template <typename E, typename R>
R consume(ReceiveChannel<E>* channel, std::function<R(ReceiveChannel<E>*)> block) {
    std::exception_ptr cause = nullptr;
    try {
        // In C++ we can't easily do 'finally' block that runs on return, 
        // but we can use RAII or explicit catch.
        // A manual try-catch-rethrow is often simpler for this specific pattern 
        // if we don't have a robust ScopeGuard.
        // But let's try a simple manual approach mimicking the Kotlin code.
        R result = block(channel);
        channel->cancel(nullptr);
        return result;
    } catch (...) {
        cause = std::current_exception();
        channel->cancel(cause);
        throw;
    }
}

template <typename E>
void consume(ReceiveChannel<E>* channel, std::function<void(ReceiveChannel<E>*)> block) {
    std::exception_ptr cause = nullptr;
    try {
        block(channel);
        channel->cancel(nullptr);
    } catch (...) {
        cause = std::current_exception();
        channel->cancel(cause);
        throw;
    }
}


template <typename E>
void consume_each(ReceiveChannel<E>* channel, std::function<void(E)> action) {
    consume<E>(channel, [&](ReceiveChannel<E>* c) {
        auto iterator = c->iterator();
        while (iterator->has_next()) {
            action(iterator->next());
        }
    });
}

template <typename E>
std::vector<E> to_list(ReceiveChannel<E>* channel) {
    std::vector<E> list;
    consume_each<E>(channel, [&](E e) {
        list.push_back(e);
    });
    return list;
}

} // namespace channels
} // namespace coroutines
} // namespace kotlinx
