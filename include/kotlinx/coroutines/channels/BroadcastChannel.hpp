#pragma once

#include "kotlinx/coroutines/channels/BufferedChannel.hpp"
#include "kotlinx/coroutines/channels/ConflatedBufferedChannel.hpp"
#include "kotlinx/coroutines/channels/Channel.hpp"
#include <vector>
#include <mutex>
#include <algorithm>
#include <memory>
#include <optional>

namespace kotlinx {
namespace coroutines {
namespace channels {

/**
 * Broadcast channel implementation.
 * Transliterated from BroadcastChannel.kt.
 */
template <typename E>
class BroadcastChannel : public BufferedChannel<E> {
public:
    // Constants
    static constexpr int UNLIMITED = Channel<E>::UNLIMITED;
    static constexpr int CONFLATED = Channel<E>::CONFLATED;
    static constexpr int BUFFERED = Channel<E>::BUFFERED;

    BroadcastChannel(int capacity) 
        : BufferedChannel<E>(Channel<E>::RENDEZVOUS), // BroadcastChannelImpl inits with RENDEZVOUS
          capacity_(capacity) 
    {
        if (capacity == 0) throw std::invalid_argument("Unsupported 0 capacity for BroadcastChannel");
    }

    // ###########################
    // # Subscription Management #
    // ###########################

    std::shared_ptr<ReceiveChannel<E>> open_subscription() {
        std::lock_guard<std::recursive_mutex> g(lock_);
        
        std::shared_ptr<ReceiveChannel<E>> s;
        if (capacity_ == CONFLATED) {
            s = std::make_shared<SubscriberConflated>(this);
        } else {
            s = std::make_shared<SubscriberBuffered>(this, capacity_);
        }

        if (this->is_closed_for_send() && !has_conflated_element_) {
            s->close(this->get_close_cause());
            return s;
        }

        if (has_conflated_element_) {
            // Kotlin: s.trySend(value)
            // We use static_cast/dynamic_cast to access try_send if ReceiveChannel doesn't have it?
            // ReceiveChannel has try_receive, but not try_send.
            // s is concrete Subscriber type which is a SendChannel.
            // We need to cast or use the concrete type.
            if (capacity_ == CONFLATED) {
                std::static_pointer_cast<SubscriberConflated>(s)->try_send(last_conflated_element_);
            } else {
                std::static_pointer_cast<SubscriberBuffered>(s)->try_send(last_conflated_element_);
            }
        }
        
        subscribers_.push_back(s);
        return s;
    }

    void remove_subscriber(ReceiveChannel<E>* s) {
        std::lock_guard<std::recursive_mutex> g(lock_);
        auto it = std::remove_if(subscribers_.begin(), subscribers_.end(), 
            [s](const std::shared_ptr<ReceiveChannel<E>>& sub) { return sub.get() == s; });
        subscribers_.erase(it, subscribers_.end());
    }

    // #############################
    // # The `send(..)` Operations #
    // #############################

    ChannelAwaiter<void> send(E element) override {
        // We need to construct a ChannelAwaiter that suspends if necessary.
        // Kotlin: send is suspend.
        // It iterates subscribers and calls send(element).
        // Since we can't easily yield in a raw C++ loop without a proper coroutine type,
        // we might return a simpler awaiter if we assume fast path, but strict fidelity requires suspension.
        
        // C++20 coroutine transformation:
        // This function must be a coroutine if it uses co_await.
        // But the interface is virtual ChannelAwaiter<void> send(E).
        // We need to implement it manually or use a helper that returns ChannelAwaiter.
        
        return suspend_send_logic(std::move(element));
    }

    ChannelResult<void> try_send(E element) override {
        std::lock_guard<std::recursive_mutex> g(lock_);
        
        if (this->is_closed_for_send()) {
            return ChannelResult<void>::closed(this->get_close_cause());
        }

        // Check if any subscriber should suspend (unsupported in simple try_send?)
        // Kotlin: check if any subscriber.shouldSendSuspend()
        // We skip this check for now or assume false for try_send validity.
        
        if (capacity_ == CONFLATED) {
            last_conflated_element_ = element;
            has_conflated_element_ = true;
        }

        for (auto& sub : subscribers_) {
            // dynamic_cast to SendChannel? ReceiveChannel* is stored.
            // But they are BufferedChannels.
            auto ch = std::dynamic_pointer_cast<SendChannel<E>>(sub);
            if (ch) ch->try_send(element);
        }

        return ChannelResult<void>::success();
    }

    // ############################
    // # Closing and Cancellation #
    // ############################

    bool close(std::exception_ptr cause = nullptr) override {
        std::lock_guard<std::recursive_mutex> g(lock_);
        
        for (auto& sub : subscribers_) {
             // sub is ReceiveChannel, need to cast to close?
             // ReceiveChannel has cancel, SendChannel has close.
             // Subscribers are BufferedChannels (Send + Receive).
             auto ch = std::dynamic_pointer_cast<BufferedChannel<E>>(sub);
             if (ch) ch->close(cause);
        }

        // Filter subscribers (lazy cleanup)
        // Kotlin: subscribers = subscribers.filter { it.hasElements() }
        
        return BufferedChannel<E>::close(cause);
    }
    
    // Protected for strictness?
protected:
    class SubscriberBuffered : public BufferedChannel<E> {
        BroadcastChannel* broadcast_;
    public:
        SubscriberBuffered(BroadcastChannel* b, int cap) 
            : BufferedChannel<E>(cap), broadcast_(b) {}
            
        bool close(std::exception_ptr cause = nullptr) override {
             broadcast_->remove_subscriber(this);
             return BufferedChannel<E>::close(cause);
        }
        
        // cancelImpl in Kotlin -> close/cancel logic here
        void cancel(std::exception_ptr cause = nullptr) override {
            broadcast_->remove_subscriber(this);
            BufferedChannel<E>::cancel(cause);
        }
    };

    class SubscriberConflated : public ConflatedBufferedChannel<E> {
        BroadcastChannel* broadcast_;
    public:
        SubscriberConflated(BroadcastChannel* b) 
            : ConflatedBufferedChannel<E>(1, BufferOverflow::DROP_OLDEST), broadcast_(b) {}
            
        void cancel(std::exception_ptr cause = nullptr) override {
            broadcast_->remove_subscriber(this);
            ConflatedBufferedChannel<E>::cancel(cause);
        }
    };

private:
    int capacity_;
    std::recursive_mutex lock_;
    std::vector<std::shared_ptr<ReceiveChannel<E>>> subscribers_;
    
    E last_conflated_element_;
    bool has_conflated_element_ = false;

    // Helper for send coroutine
    // This is where we implement the suspending loop.
    // NOTE: This uses a struct/lambda that returns ChannelAwaiter<void>
    // but simplified to just standard suspension logic?
    // We cannot easily 'co_await' inside this method if we are not in a coroutine context that returns a Task.
    // ChannelAwaiter is an Awaitable, not a Coroutine Return Object (Promise).
    // We need a helper `Task` or `Coroutine` return type to write `co_await`.
    // I will mock this for now as user requested "sketch". 
    // Just iterating and try_send is the fast path sketch.
    // The Suspended path requires a `Task` type we defined elsewhere (or need to).
    
    ChannelAwaiter<void> suspend_send_logic(E element) {
        // Sketch: assume fast path for now OR blocking if we can't suspend.
        // Ideally:
        // co_await suspend_always{};
        
        // Let's implement the lock logic and loop
        std::vector<std::shared_ptr<SendChannel<E>>> subs;
        {
             std::lock_guard<std::recursive_mutex> g(lock_);
             if (this->is_closed_for_send()) throw ClosedSendChannelException();
             
             if (capacity_ == CONFLATED) {
                 last_conflated_element_ = element;
                 has_conflated_element_ = true;
             }
             
             for (auto& s : subscribers_) {
                 auto ch = std::dynamic_pointer_cast<SendChannel<E>>(s);
                 if (ch) subs.push_back(ch);
             }
        }
        
        // Loop and send (simplified non-suspending for sketch, as we lack co_await infrastructure in this header)
        for (auto& s : subs) {
            // Ideally: co_await s->send(element);
            // Sketch: try_send fallback
            s->try_send(element); 
        }
        
        return ChannelAwaiter<void>(); // Ready/Success
    }

};

} // namespace channels
} // namespace coroutines
} // namespace kotlinx
