#pragma once
// Transliterated from: kotlinx-coroutines-core/common/src/channels/BroadcastChannel.kt
//
// Kotlin imports:
// - kotlinx.coroutines.*
// - kotlinx.coroutines.channels.BufferOverflow.*
// - kotlinx.coroutines.channels.Channel.Factory.{BUFFERED, CHANNEL_DEFAULT_CAPACITY, CONFLATED, UNLIMITED}
// - kotlinx.coroutines.internal.*
// - kotlinx.coroutines.selects.*

#include "kotlinx/coroutines/channels/BufferedChannel.hpp"
#include "kotlinx/coroutines/channels/BufferOverflow.hpp"
#include "kotlinx/coroutines/channels/ConflatedBufferedChannel.hpp"
#include "kotlinx/coroutines/channels/Channel.hpp"
#include "kotlinx/coroutines/Job.hpp"
#include "kotlinx/coroutines/CoroutineScope.hpp"
#include "kotlinx/coroutines/Exceptions.hpp"
#include "kotlinx/coroutines/internal/Symbol.hpp"
#include "kotlinx/coroutines/internal/Concurrent.hpp"
#include "kotlinx/coroutines/selects/Select.hpp"
#include "kotlinx/coroutines/intrinsics/Intrinsics.hpp"
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

    explicit BroadcastChannel(int capacity)
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
            // TODO: ReceiveChannel doesn't have close() - use cancel() instead
            s->cancel(this->close_cause_);
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
        // This project does not use C++20 coroutines (`co_await`/`co_return`).
        // Suspending behavior must be expressed via the Continuation ABI / ChannelAwaiter machinery.
        
        return suspend_send_logic(std::move(element));
    }

    ChannelResult<void> try_send(E element) override {
        std::lock_guard<std::recursive_mutex> g(lock_);
        
        if (this->is_closed_for_send()) {
            return ChannelResult<void>::closed(this->close_cause_);
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
        explicit SubscriberConflated(BroadcastChannel* b)
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
    // This project does not use C++20 coroutines (`co_await`/`co_return`).
    // When this needs to suspend, it must do so via ChannelAwaiter/Continuation-style mechanics.
    
    ChannelAwaiter<void> suspend_send_logic(E element) {
        // Sketch: assume fast path for now OR blocking if we can't suspend.
        
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
        
        // Loop and send (simplified non-suspending sketch)
        for (auto& s : subs) {
            // Sketch: try_send fallback
            s->try_send(element); 
        }
        
        return ChannelAwaiter<void>(); // Ready/Success
    }

};

} // namespace channels
} // namespace coroutines
} // namespace kotlinx
