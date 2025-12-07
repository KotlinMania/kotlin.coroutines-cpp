// Transliterated from: reactive/kotlinx-coroutines-reactive/src/Channel.kt
// TODO: Implement semantic correctness for reactive streams channel operations

namespace kotlinx {
namespace coroutines {
namespace reactive {

// TODO: #include <kotlinx/atomicfu/atomic.hpp>
// TODO: #include <kotlinx/coroutines/channels/channels.hpp>
// TODO: #include <kotlinx/coroutines/flow/flow.hpp>
// TODO: #include <org/reactivestreams/Publisher.hpp>
// TODO: #include <org/reactivestreams/Subscriber.hpp>
// TODO: #include <org/reactivestreams/Subscription.hpp>

/**
 * Subscribes to this [Publisher] and performs the specified action for each received element.
 *
 * If [action] throws an exception at some point, the subscription is cancelled, and the exception is rethrown from
 * [collect]. Also, if the publisher signals an error, that error is rethrown from [collect].
 */
// TODO: implement coroutine suspension
template<typename T>
void collect(Publisher<T>& publisher, std::function<void(T)> action) {
    auto channel = to_channel(publisher);
    channel.consume_each(action);
}

// @PublishedApi
template<typename T>
ReceiveChannel<T> to_channel(Publisher<T>& publisher, int request = 1) {
    auto channel = SubscriptionChannel<T>(request);
    publisher.subscribe(channel);
    return channel;
}

// @Suppress("INVISIBLE_REFERENCE", "INVISIBLE_MEMBER", "SubscriberImplementation")
template<typename T>
class SubscriptionChannel : public BufferedChannel<T>, public Subscriber<T> {
private:
    int request_;
    std::atomic<Subscription*> subscription_{nullptr};
    // requested from subscription minus number of received minus number of enqueued receivers,
    // can be negative if we have receivers, but no subscription yet
    std::atomic<int> requested_{0};

public:
    explicit SubscriptionChannel(int request) :
        BufferedChannel<T>(Channel::kUnlimited),
        request_(request) {
        if (request < 0) {
            throw std::invalid_argument("Invalid request size: " + std::to_string(request));
        }
    }

    // --------------------- BufferedChannel overrides -------------------------------
    // @Suppress("CANNOT_OVERRIDE_INVISIBLE_MEMBER")
    void on_receive_enqueued() override {
        // TODO: Implement lock-free loop on _requested
        // This should handle requesting more from subscription when needed
    }

    // @Suppress("CANNOT_OVERRIDE_INVISIBLE_MEMBER")
    void on_receive_dequeued() override {
        requested_.fetch_add(1);
    }

    // @Suppress("CANNOT_OVERRIDE_INVISIBLE_MEMBER")
    void on_closed_idempotent() override {
        Subscription* old_sub = subscription_.exchange(nullptr);
        if (old_sub != nullptr) {
            old_sub->cancel(); // cancel exactly once
        }
    }

    // --------------------- Subscriber overrides -------------------------------
    void on_subscribe(Subscription& s) override {
        subscription_.store(&s);
        while (true) { // lock-free loop on _requested
            if (is_closed_for_send()) {
                s.cancel();
                return;
            }
            int was_requested = requested_.load();
            if (was_requested >= request_) return; // ok -- normal story
            // otherwise, receivers came before we had subscription or need to make initial request
            // try to fixup by making request
            if (!requested_.compare_exchange_weak(was_requested, request_)) continue;
            s.request(static_cast<long>(request_ - was_requested));
            return;
        }
    }

    void on_next(T t) override {
        requested_.fetch_sub(1);
        try_send(t); // Safe to ignore return value here, expectedly racing with cancellation
    }

    void on_complete() override {
        close(nullptr);
    }

    void on_error(std::exception_ptr e) override {
        close(e);
    }
};

/** @suppress */
// @Deprecated(
//     message = "Transforming publisher to channel is deprecated, use asFlow() instead",
//     level = DeprecationLevel.HIDDEN) // ERROR in 1.4, HIDDEN in 1.6.0
[[deprecated("Transforming publisher to channel is deprecated, use as_flow() instead")]]
template<typename T>
ReceiveChannel<T> open_subscription(Publisher<T>& publisher, int request = 1) {
    auto channel = SubscriptionChannel<T>(request);
    publisher.subscribe(channel);
    return channel;
}

} // namespace reactive
} // namespace coroutines
} // namespace kotlinx

/*
 * TODO List for semantic implementation:
 *
 * 1. Implement BufferedChannel<T> base class
 * 2. Implement ReceiveChannel<T> interface
 * 3. Implement atomic operations (atomicfu equivalent)
 * 4. Implement Channel capacity constants (UNLIMITED, etc.)
 * 5. Implement lock-free algorithms for request management
 * 6. Implement try_send and consume_each methods
 * 7. Implement is_closed_for_send() check
 * 8. Implement close() with optional exception
 * 9. Implement reactive streams Subscriber interface
 * 10. Implement reactive streams Subscription interface
 * 11. Add thread-safety guarantees
 * 12. Test backpressure handling
 * 13. Test cancellation behavior
 */
