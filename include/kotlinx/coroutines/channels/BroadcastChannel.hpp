#pragma once
#include "kotlinx/coroutines/channels/BufferedChannel.hpp"
#include "kotlinx/coroutines/channels/ConflatedBufferedChannel.hpp"
#include <vector>
#include <mutex>
#include <algorithm>

namespace kotlinx {
namespace coroutines {
namespace channels {

/**
 * Broadcast channel is a non-blocking primitive for communication between the sender and multiple receivers
 * that subscribe for the elements sent by the sender.
 *
 * @see BroadcastChannel(int)
 * @see ConflatedBroadcastChannel
 */
template <typename E>
class BroadcastChannel : public BufferedChannel<E> {
    std::vector<std::shared_ptr<ReceiveChannel<E>>> subscribers;
    std::mutex lock;

    // Inner classes mirroring Kotlin implementation
    class SubscriberBuffered : public BufferedChannel<E> {
        BroadcastChannel* broadcast;
    public:
        SubscriberBuffered(BroadcastChannel* broadcast, int capacity) 
            : BufferedChannel<E>(capacity), broadcast(broadcast) {}
            
        void cancel(std::exception_ptr cause = nullptr) override {
            broadcast->removeSubscriber(this);
            BufferedChannel<E>::cancel(cause);
        }
    };

    class SubscriberConflated : public ConflatedBufferedChannel<E> {
        BroadcastChannel* broadcast;
    public:
        SubscriberConflated(BroadcastChannel* broadcast) 
            : ConflatedBufferedChannel<E>(1), broadcast(broadcast) {} // capacity 1 for conflated
            
        void cancel(std::exception_ptr cause = nullptr) override {
            broadcast->removeSubscriber(this);
            ConflatedBufferedChannel<E>::cancel(cause);
        }
    };

    void removeSubscriber(ReceiveChannel<E>* subscriber) {
        std::lock_guard<std::mutex> g(lock);
        // Remove subscriber from list
        auto it = std::remove_if(subscribers.begin(), subscribers.end(), 
            [subscriber](const std::shared_ptr<ReceiveChannel<E>>& s) { return s.get() == subscriber; });
        subscribers.erase(it, subscribers.end());
    }

public:
    /**
     * Creates a broadcast channel with the specified [capacity].
     */
    BroadcastChannel(int capacity) : BufferedChannel<E>(capacity) {
        // Validation handled in base or here if needed
    }

    /**
     * Subscribes to this [BroadcastChannel] and returns a channel to receive elements from it.
     * The resulting channel shall be [closed][ReceiveChannel.close] to unsubscribe from this broadcast channel.
     */
    std::shared_ptr<ReceiveChannel<E>> open_subscription() {
        std::lock_guard<std::mutex> g(lock);
        // Check closed (TODO: use isClosedForSend)
        
        std::shared_ptr<ReceiveChannel<E>> subscriber;
        if (BufferedChannel<E>::capacity == Channel<E>::CONFLATED) {
             auto sub = std::make_shared<SubscriberConflated>(this);
             // TODO: Copy current value if present
             subscriber = sub;
        } else {
             subscriber = std::make_shared<SubscriberBuffered>(this, BufferedChannel<E>::capacity);
        }
        
        subscribers.push_back(subscriber);
        return subscriber;
    }
    
    /**
     * Closes this channel with an optional [cause].
     * This closes the channel for senders and closes all subscribers.
     */
    bool close(std::exception_ptr cause = nullptr) override {
        bool result = BufferedChannel<E>::close(cause);
        // TODO: propagate to subscribers if closed
        return result;
    }
    
    // Broadcast specific send logic...
};

} // namespace channels
} // namespace coroutines
} // namespace kotlinx
