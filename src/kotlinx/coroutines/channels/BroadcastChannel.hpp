#pragma once
// Transliterated from: kotlinx-coroutines-core/common/src/channels/BroadcastChannel.kt
//
// @file:Suppress("FunctionName", "DEPRECATION", "DEPRECATION_ERROR")
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
#include "kotlinx/coroutines/CoroutineStart.hpp"
#include "kotlinx/coroutines/Builders.hpp"
#include "kotlinx/coroutines/Exceptions.hpp"
#include "kotlinx/coroutines/internal/Concurrent.hpp"
#include "kotlinx/coroutines/selects/Select.hpp"
#include "kotlinx/coroutines/intrinsics/Intrinsics.hpp"
#include <vector>
#include <mutex>
#include <algorithm>
#include <memory>
#include <optional>
#include <unordered_map>
#include <sstream>
#include <cassert>



namespace kotlinx::coroutines::channels {

// In C++ we use a bool flag (has_last_conflated_element_) instead of a sentinel Symbol.

// ============================================================================
// @suppress obsolete since 1.5.0, WARNING since 1.7.0, ERROR since 1.9.0
// @Deprecated(level = DeprecationLevel.ERROR, message = "BroadcastChannel is deprecated...")
// ============================================================================

/**
 * Line 19: public interface BroadcastChannel<E> : SendChannel<E>
 *
 * @deprecated BroadcastChannel is deprecated in favour of SharedFlow and is no longer supported.
 */
template <typename E>
class BroadcastChannel : public virtual SendChannel<E> {
public:
    virtual ~BroadcastChannel() = default;

    virtual std::shared_ptr<ReceiveChannel<E>> open_subscription() = 0;

    virtual void cancel(std::exception_ptr cause = nullptr) = 0;

    // (Hidden for binary compatibility - not transliterated)
};

// ============================================================================
// Factory function
// @suppress obsolete since 1.5.0, WARNING since 1.7.0, ERROR since 1.9.0
// ============================================================================

// Forward declaration
template <typename E>
class BroadcastChannelImpl;

template <typename E>
class ConflatedBroadcastChannel;

/**
 * Line 42-49: Factory function for BroadcastChannel
 *
 * @deprecated BroadcastChannel is deprecated in favour of SharedFlow and StateFlow.
 */
template <typename E>
[[deprecated("BroadcastChannel is deprecated in favour of SharedFlow and StateFlow")]]
std::shared_ptr<BroadcastChannel<E>> create_broadcast_channel(int capacity) {
    if (capacity == 0) {
        throw std::invalid_argument("Unsupported 0 capacity for BroadcastChannel");
    } else if (capacity == Channel<E>::UNLIMITED) {
        throw std::invalid_argument("Unsupported UNLIMITED capacity for BroadcastChannel");
    } else if (capacity == Channel<E>::CONFLATED) {
        return std::make_shared<ConflatedBroadcastChannel<E>>();
    } else if (capacity == Channel<E>::BUFFERED) {
        return std::make_shared<BroadcastChannelImpl<E>>(Channel<E>::channel_default_capacity());
    } else {
        return std::make_shared<BroadcastChannelImpl<E>>(capacity);
    }
}

// ============================================================================
// @suppress obsolete since 1.5.0, WARNING since 1.7.0, ERROR since 1.9.0
// ============================================================================

/**
 * Line 56-76: ConflatedBroadcastChannel
 *
 * @deprecated ConflatedBroadcastChannel is deprecated in favour of SharedFlow.
 */
template <typename E>
class ConflatedBroadcastChannel : public BroadcastChannel<E> {
private:
    std::shared_ptr<BroadcastChannelImpl<E>> broadcast_;

public:
    ConflatedBroadcastChannel()
        : broadcast_(std::make_shared<BroadcastChannelImpl<E>>(Channel<E>::CONFLATED)) {}

    explicit ConflatedBroadcastChannel(E value)
        : ConflatedBroadcastChannel() {
        try_send(std::move(value));
    }

    ~ConflatedBroadcastChannel() override = default;

    // Delegation to broadcast_

    E get_value() const { return broadcast_->get_value(); }

    std::optional<E> get_value_or_null() const { return broadcast_->get_value_or_null(); }

    // BroadcastChannel interface delegation
    std::shared_ptr<ReceiveChannel<E>> open_subscription() override {
        return broadcast_->open_subscription();
    }

    void cancel(std::exception_ptr cause = nullptr) override {
        broadcast_->cancel(cause);
    }

    // SendChannel interface delegation
    bool is_closed_for_send() const override { return broadcast_->is_closed_for_send(); }

    void* send(E element, Continuation<void*>* continuation) override {
        return broadcast_->send(std::move(element), continuation);
    }

    ChannelResult<void> try_send(E element) override {
        return broadcast_->try_send(std::move(element));
    }

    bool close(std::exception_ptr cause = nullptr) override {
        return broadcast_->close(cause);
    }

    void invoke_on_close(std::function<void(std::exception_ptr)> handler) override {
        broadcast_->invoke_on_close(std::move(handler));
    }

    selects::SelectClause2<E, SendChannel<E>*>& on_send() override {
        return broadcast_->on_send();
    }
};

// ============================================================================
// ============================================================================

/**
 * Line 88-360: BroadcastChannelImpl
 *
 * A common implementation for both the broadcast channel with a buffer of fixed capacity
 * and the conflated broadcast channel.
 *
 * Note: elements sent to this channel while there are no openSubscription subscribers
 * are immediately lost.
 */
template <typename E>
class BroadcastChannelImpl : public BufferedChannel<E>, public BroadcastChannel<E> {
private:
    // Forward declarations for inner classes
    class SubscriberBuffered;
    class SubscriberConflated;

    int capacity_;

    mutable std::recursive_mutex lock_;

    std::vector<std::shared_ptr<BufferedChannel<E>>> subscribers_;

    // We use a variant approach: optional + special marker check
    E last_conflated_element_;
    bool has_last_conflated_element_ = false;

    std::unordered_map<void*, void*> on_send_internal_result_;

public:
    explicit BroadcastChannelImpl(int capacity)
        : BufferedChannel<E>(Channel<E>::RENDEZVOUS, nullptr),
          capacity_(capacity) {
        if (!(capacity >= 1 || capacity == Channel<E>::CONFLATED)) {
            throw std::invalid_argument(
                "BroadcastChannel capacity must be positive or Channel.CONFLATED, but " +
                std::to_string(capacity) + " was specified");
        }
    }

    ~BroadcastChannelImpl() override = default;

    // ###########################
    // # Subscription Management #
    // ###########################

    std::shared_ptr<ReceiveChannel<E>> open_subscription() override {
        std::lock_guard<std::recursive_mutex> guard(lock_);

        std::shared_ptr<BufferedChannel<E>> s;
        if (capacity_ == Channel<E>::CONFLATED) {
            s = std::make_shared<SubscriberConflated>(this);
        } else {
            s = std::make_shared<SubscriberBuffered>(this, capacity_);
        }

        if (BufferedChannel<E>::is_closed_for_send() && !has_last_conflated_element_) {
            s->close(BufferedChannel<E>::close_cause());
            return s;
        }

        if (has_last_conflated_element_) {
            s->try_send(get_value());
        }

        subscribers_.push_back(s);

        return s;
    }

    void remove_subscriber(ReceiveChannel<E>* s) {
        std::lock_guard<std::recursive_mutex> guard(lock_);
        subscribers_.erase(
            std::remove_if(subscribers_.begin(), subscribers_.end(),
                [s](const std::shared_ptr<BufferedChannel<E>>& sub) {
                    return sub.get() == s;
                }),
            subscribers_.end());
    }

    // #############################
    // # The `send(..)` Operations #
    // #############################

    /**
     * Line 161-184: override suspend fun send(element: E)
     *
     * Sends the specified element to all subscribers.
     *
     * **!!! THIS IMPLEMENTATION IS NOT LINEARIZABLE !!!**
     */
    void* send(E element, Continuation<void*>* continuation) override {
        std::vector<std::shared_ptr<BufferedChannel<E>>> subs;
        {
            std::lock_guard<std::recursive_mutex> guard(lock_);

            if (BufferedChannel<E>::is_closed_for_send()) {
                throw_send_exception();
            }

            if (capacity_ == Channel<E>::CONFLATED) {
                last_conflated_element_ = element;
                has_last_conflated_element_ = true;
            }

            subs = subscribers_;
        }

        for (auto& sub : subs) {
            bool success = sub->send_broadcast(element);

            if (!success && BufferedChannel<E>::is_closed_for_send()) {
                throw_send_exception();
            }
        }

        (void)continuation;
        return nullptr;
    }

    ChannelResult<void> try_send(E element) override {
        std::lock_guard<std::recursive_mutex> guard(lock_);

        if (BufferedChannel<E>::is_closed_for_send()) {
            return BufferedChannel<E>::try_send(std::move(element));
        }

        bool should_suspend = std::any_of(subscribers_.begin(), subscribers_.end(),
            [](const std::shared_ptr<BufferedChannel<E>>& sub) {
                return sub->should_send_suspend();
            });

        if (should_suspend) {
            return ChannelResult<void>::failure();
        }

        if (capacity_ == Channel<E>::CONFLATED) {
            last_conflated_element_ = element;
            has_last_conflated_element_ = true;
        }

        for (auto& sub : subscribers_) {
            sub->try_send(element);
        }

        return ChannelResult<void>::success();
    }

    // ###########################################
    // # The `select` Expression: onSend { ... } #
    // ###########################################

    /**
     * Line 209-273: override fun registerSelectForSend(select: SelectInstance<*>, element: Any?)
     *
     * It is extremely complicated to support sending via `select` for broadcasts,
     * as the operation should wait on multiple subscribers simultaneously.
     * At the same time, broadcasts are obsolete, so we need a simple implementation
     * that works somehow. Here is a tricky work-around. First, we launch a new
     * coroutine that performs plain `send(..)` operation and tries to complete
     * this `select` via `trySelect`, independently on whether it is in the
     * registration or in the waiting phase. On success, the operation finishes.
     * On failure, if another clause is already selected or the `select` operation
     * has been cancelled, we observe non-linearizable behaviour, as this `onSend`
     * clause is completed as well. However, we believe that such a non-linearizability
     * is fine for obsolete API. The last case is when the `select` operation is still
     * in the registration case, so this `onSend` clause should be re-registered.
     * The idea is that we keep information that this `onSend` clause is already selected
     * and finish immediately.
     */
    template <typename R>
    void register_select_for_send(selects::SelectInstance<R>* select, E element) {
        // selected, finishing immediately in this case.
        {
            std::lock_guard<std::recursive_mutex> guard(lock_);
            auto it = on_send_internal_result_.find(static_cast<void*>(select));
            if (it != on_send_internal_result_.end()) {
                void* result = it->second;
                on_send_internal_result_.erase(it);
                // `result` is either Unit (nullptr) or CHANNEL_CLOSED.
                select->select_in_registration_phase(result);
                return;
            }
        }

        // and tries to select this `onSend` clause at the end.
        // Kotlin: CoroutineScope(select.context).launch(start = CoroutineStart.UNDISPATCHED) { ... }
        auto context = select->get_context();

        // The launch block - captured by value for safety
        auto self = this;
        auto launch_block = [self, select, element]() {
            bool success = false;
            try {
                // Blocking send for now - Kotlin would suspend
                self->send(element, nullptr);
                success = true;
            } catch (const ClosedSendChannelException&) {
                if (self->is_closed_for_send()) {
                    success = false;
                } else {
                    throw;
                }
            } catch (...) {
                if (self->is_closed_for_send()) {
                    success = false;
                } else {
                    throw;
                }
            }

            // try to complete the `select` operation.
            {
                std::lock_guard<std::recursive_mutex> guard(self->lock_);
                assert(self->on_send_internal_result_.find(static_cast<void*>(select)) ==
                       self->on_send_internal_result_.end());
                self->on_send_internal_result_[static_cast<void*>(select)] =
                    success ? nullptr : static_cast<void*>(&CHANNEL_CLOSED());
                bool selected = select->try_select(self, nullptr);
                // For now, always remove since we don't have REREGISTER detection
                if (selected) {
                    self->on_send_internal_result_.erase(static_cast<void*>(select));
                }
            }
        };

        // Launch with UNDISPATCHED start
        // Kotlin: CoroutineScope(select.context).launch(start = CoroutineStart.UNDISPATCHED) { ... }
        // Create a temporary scope from context and launch
        class TempScope : public CoroutineScope {
            std::shared_ptr<CoroutineContext> ctx_;
        public:
            explicit TempScope(std::shared_ptr<CoroutineContext> c) : ctx_(std::move(c)) {}
            std::shared_ptr<CoroutineContext> get_coroutine_context() const override { return ctx_; }
        };
        TempScope scope(context);

        coroutines::launch(&scope, context, CoroutineStart::UNDISPATCHED,
            [launch_block](CoroutineScope*) {
                launch_block();
            });
    }

    // ############################
    // # Closing and Cancellation #
    // ############################

    bool close(std::exception_ptr cause = nullptr) override {
        std::lock_guard<std::recursive_mutex> guard(lock_);

        for (auto& sub : subscribers_) {
            sub->close(cause);
        }

        subscribers_.erase(
            std::remove_if(subscribers_.begin(), subscribers_.end(),
                [](const std::shared_ptr<BufferedChannel<E>>& sub) {
                    return !sub->has_elements();
                }),
            subscribers_.end());

        return BufferedChannel<E>::close(cause);
    }

    bool cancel_impl(std::exception_ptr cause) {
        std::lock_guard<std::recursive_mutex> guard(lock_);

        for (auto& sub : subscribers_) {
            sub->cancel(cause);
        }

        has_last_conflated_element_ = false;

        return BufferedChannel<E>::cancel_impl(cause);
    }

    // BroadcastChannel::cancel delegates to cancelImpl
    void cancel(std::exception_ptr cause = nullptr) override {
        cancel_impl(cause);
    }

    bool is_closed_for_send() const override {
        std::lock_guard<std::recursive_mutex> guard(lock_);
        return BufferedChannel<E>::is_closed_for_send();
    }

    // ########################################
    // # ConflatedBroadcastChannel Operations #
    // ########################################

    E get_value() const {
        std::lock_guard<std::recursive_mutex> guard(lock_);

        if (BufferedChannel<E>::is_closed_for_send()) {
            auto cause = BufferedChannel<E>::close_cause();
            if (cause) {
                std::rethrow_exception(cause);
            }
            throw std::logic_error("This broadcast channel is closed");
        }

        if (!has_last_conflated_element_) {
            throw std::logic_error("No value");
        }

        return last_conflated_element_;
    }

    std::optional<E> get_value_or_null() const {
        std::lock_guard<std::recursive_mutex> guard(lock_);

        if (BufferedChannel<E>::is_closed_for_receive()) {
            return std::nullopt;
        }

        if (!has_last_conflated_element_) {
            return std::nullopt;
        }

        return last_conflated_element_;
    }

    // #################
    // # For Debugging #
    // #################

    std::string to_string() const {
        std::lock_guard<std::recursive_mutex> guard(lock_);
        std::ostringstream oss;

        if (has_last_conflated_element_) {
            oss << "CONFLATED_ELEMENT=<element>; ";
        }
        oss << "BROADCAST=<" << BufferedChannel<E>::to_string() << ">; ";
        oss << "SUBSCRIBERS=<";
        for (size_t i = 0; i < subscribers_.size(); ++i) {
            if (i > 0) oss << ";";
            oss << subscribers_[i]->to_string();
        }
        oss << ">";
        return oss.str();
    }

    // SendChannel interface - delegate to BufferedChannel
    void invoke_on_close(std::function<void(std::exception_ptr)> handler) override {
        BufferedChannel<E>::invoke_on_close(std::move(handler));
    }

    selects::SelectClause2<E, SendChannel<E>*>& on_send() override {
        return BufferedChannel<E>::on_send();
    }

private:
    void throw_send_exception() {
        auto cause = BufferedChannel<E>::close_cause();
        if (cause) {
            std::rethrow_exception(cause);
        }
        throw ClosedSendChannelException("Channel was closed");
    }

    // ##############################
    // # Subscriber Implementations #
    // ##############################

    class SubscriberBuffered : public BufferedChannel<E> {
        BroadcastChannelImpl* broadcast_;
    public:
        SubscriberBuffered(BroadcastChannelImpl* broadcast, int capacity)
            : BufferedChannel<E>(capacity, nullptr),
              broadcast_(broadcast) {}

        bool cancel_impl(std::exception_ptr cause) override {
            broadcast_->remove_subscriber(this);
            return BufferedChannel<E>::cancel_impl(cause);
        }

        void cancel(std::exception_ptr cause = nullptr) override {
            cancel_impl(cause);
        }
    };

    class SubscriberConflated : public ConflatedBufferedChannel<E> {
        BroadcastChannelImpl* broadcast_;
    public:
        explicit SubscriberConflated(BroadcastChannelImpl* broadcast)
            : ConflatedBufferedChannel<E>(1, BufferOverflow::DROP_OLDEST, nullptr),
              broadcast_(broadcast) {}

        bool cancel_impl(std::exception_ptr cause) override {
            broadcast_->remove_subscriber(this);
            return ConflatedBufferedChannel<E>::cancel_impl(cause);
        }

        void cancel(std::exception_ptr cause = nullptr) override {
            cancel_impl(cause);
        }
    };
};

} // namespace kotlinx::coroutines::channels


