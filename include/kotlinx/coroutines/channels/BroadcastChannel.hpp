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
        
        std::shared_ptr<ReceiveChannel<E>> subscriber;
        if (BufferedChannel<E>::capacity == Channel<E>::CONFLATED) {
             auto sub = std::make_shared<SubscriberConflated>(this);
             if (has_conflated_element) {
                 sub->try_send(last_conflated_element);
             }
             subscriber = sub;
        } else {
             subscriber = std::make_shared<SubscriberBuffered>(this, BufferedChannel<E>::capacity);
        }
        
        // If broadcast is closed, close subscriber immediately?
        if (BufferedChannel<E>::is_closed_for_send()) {
             // In Kotlin: only if lastConflatedElement is NO_ELEMENT (handled above)
             // simplified: close if no element to replay
             if (!has_conflated_element) {
                 subscriber->close(BufferedChannel<E>::get_close_cause());
                 return subscriber; 
             }
        }
        
        subscribers.push_back(subscriber);
        return subscriber;
    }
    
    /**
     * Closes this channel with an optional [cause].
     * This closes the channel for senders and closes all subscribers.
     */
    // #############################
    // # The `send(..)` Operations #
    // #############################

    void send(E element) override {
        std::vector<std::shared_ptr<ReceiveChannel<E>>> current_subscribers;
        {
             std::lock_guard<std::mutex> g(lock);
             if (BufferedChannel<E>::is_closed_for_send()) {
                 throw BufferedChannel<E>::get_send_exception();
             }
             if (BufferedChannel<E>::capacity == Channel<E>::CONFLATED) {
                 last_conflated_element = element;
                 has_conflated_element = true;
             }
             current_subscribers = subscribers;
        }

        for (auto& sub : current_subscribers) {
            // Cast back to SendChannel or use a method that allows external send
            // Subscribers are BufferedChannel<E>, so they have send/trySend
             auto channel = std::dynamic_pointer_cast<BufferedChannel<E>>(sub);
             if (channel) {
                 // Ignore result? Kotlin uses sendBroadcast which returns success/fail
                 // We use try_send for simplicity or suspend?
                 // Kotlin implementation uses 'sendBroadcast' which is trySend essentially for buffer
                 channel->try_send(element);
             }
        }
    }

    bool close(std::exception_ptr cause = nullptr) override {
         std::lock_guard<std::mutex> g(lock);
         bool result = BufferedChannel<E>::close(cause);
         
         for (auto& sub : subscribers) {
             auto channel = std::dynamic_pointer_cast<BufferedChannel<E>>(sub);
             if (channel) {
                 channel->close(cause);
             }
         }
         // Clean up subscribers if needed, but Kotlin filters them lazily or on close
         return result;
    }

private:
    E last_conflated_element;
    bool has_conflated_element = false;

} // namespace channels
} // namespace coroutines
} // namespace kotlinx
