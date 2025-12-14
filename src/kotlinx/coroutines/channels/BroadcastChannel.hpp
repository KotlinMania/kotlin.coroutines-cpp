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

// Line 362: private val NO_ELEMENT = Symbol("NO_ELEMENT")
// In C++ we use a bool flag (has_last_conflated_element_) instead of a sentinel Symbol.

// ============================================================================
// Line 14-35: public interface BroadcastChannel<E> : SendChannel<E>
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

    // Line 23: public fun openSubscription(): ReceiveChannel<E>
    virtual std::shared_ptr<ReceiveChannel<E>> open_subscription() = 0;

    // Line 28: public fun cancel(cause: CancellationException? = null)
    virtual void cancel(std::exception_ptr cause = nullptr) = 0;

    // Line 34: @Deprecated public fun cancel(cause: Throwable? = null): Boolean
    // (Hidden for binary compatibility - not transliterated)
};

// ============================================================================
// Line 37-49: public fun <E> BroadcastChannel(capacity: Int): BroadcastChannel<E>
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
    // Line 43-49: when (capacity) { ... }
    if (capacity == 0) {
        // Line 44
        throw std::invalid_argument("Unsupported 0 capacity for BroadcastChannel");
    } else if (capacity == Channel<E>::UNLIMITED) {
        // Line 45
        throw std::invalid_argument("Unsupported UNLIMITED capacity for BroadcastChannel");
    } else if (capacity == Channel<E>::CONFLATED) {
        // Line 46
        return std::make_shared<ConflatedBroadcastChannel<E>>();
    } else if (capacity == Channel<E>::BUFFERED) {
        // Line 47
        return std::make_shared<BroadcastChannelImpl<E>>(Channel<E>::channel_default_capacity());
    } else {
        // Line 48
        return std::make_shared<BroadcastChannelImpl<E>>(capacity);
    }
}

// ============================================================================
// Line 51-76: public class ConflatedBroadcastChannel<E>
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
    // Line 57: private val broadcast: BroadcastChannelImpl<E>
    std::shared_ptr<BroadcastChannelImpl<E>> broadcast_;

public:
    // Line 59: public constructor(): this(BroadcastChannelImpl<E>(capacity = CONFLATED))
    ConflatedBroadcastChannel()
        : broadcast_(std::make_shared<BroadcastChannelImpl<E>>(Channel<E>::CONFLATED)) {}

    // Line 63-65: public constructor(value: E) : this() { trySend(value) }
    explicit ConflatedBroadcastChannel(E value)
        : ConflatedBroadcastChannel() {
        try_send(std::move(value));
    }

    ~ConflatedBroadcastChannel() override = default;

    // Delegation to broadcast_

    // Line 70: public val value: E get() = broadcast.value
    E get_value() const { return broadcast_->get_value(); }

    // Line 75: public val valueOrNull: E? get() = broadcast.valueOrNull
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
// Line 78-360: internal class BroadcastChannelImpl<E>
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

    // Line 92: val capacity: Int
    int capacity_;

    // Line 103: private val lock = ReentrantLock()
    mutable std::recursive_mutex lock_;

    // Line 106: private var subscribers: List<BufferedChannel<E>> = emptyList()
    std::vector<std::shared_ptr<BufferedChannel<E>>> subscribers_;

    // Line 109: private var lastConflatedElement: Any? = NO_ELEMENT
    // We use a variant approach: optional + special marker check
    E last_conflated_element_;
    bool has_last_conflated_element_ = false;

    // Line 274: private val onSendInternalResult = HashMap<SelectInstance<*>, Any?>()
    std::unordered_map<void*, void*> on_send_internal_result_;

public:
    // Line 88-98: Constructor
    explicit BroadcastChannelImpl(int capacity)
        : BufferedChannel<E>(Channel<E>::RENDEZVOUS, nullptr),  // Line 93
          capacity_(capacity) {
        // Line 95-97: require(capacity >= 1 || capacity == CONFLATED)
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

    // Line 115-135: override fun openSubscription(): ReceiveChannel<E>
    std::shared_ptr<ReceiveChannel<E>> open_subscription() override {
        std::lock_guard<std::recursive_mutex> guard(lock_);

        // Line 118: val s = if (capacity == CONFLATED) SubscriberConflated() else SubscriberBuffered()
        std::shared_ptr<BufferedChannel<E>> s;
        if (capacity_ == Channel<E>::CONFLATED) {
            s = std::make_shared<SubscriberConflated>(this);
        } else {
            s = std::make_shared<SubscriberBuffered>(this, capacity_);
        }

        // Line 123-126: if (isClosedForSend && lastConflatedElement === NO_ELEMENT)
        if (BufferedChannel<E>::is_closed_for_send() && !has_last_conflated_element_) {
            s->close(BufferedChannel<E>::close_cause());
            return s;
        }

        // Line 129-131: if (lastConflatedElement !== NO_ELEMENT) s.trySend(value)
        if (has_last_conflated_element_) {
            s->try_send(get_value());
        }

        // Line 133: subscribers += s
        subscribers_.push_back(s);

        // Line 134: return s
        return s;
    }

    // Line 137-139: private fun removeSubscriber(s: ReceiveChannel<E>)
    void remove_subscriber(ReceiveChannel<E>* s) {
        std::lock_guard<std::recursive_mutex> guard(lock_);
        // Line 138: subscribers = subscribers.filter { it !== s }
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

            // Line 164: if (isClosedForSend) throw sendException
            if (BufferedChannel<E>::is_closed_for_send()) {
                throw_send_exception();
            }

            // Line 166: if (capacity == CONFLATED) lastConflatedElement = element
            if (capacity_ == Channel<E>::CONFLATED) {
                last_conflated_element_ = element;
                has_last_conflated_element_ = true;
            }

            // Line 168: subscribers (get reference under lock)
            subs = subscribers_;
        }

        // Line 175-183: send to each subscriber
        for (auto& sub : subs) {
            // Line 179: val success = it.sendBroadcast(element)
            bool success = sub->send_broadcast(element);

            // Line 182: if (!success && isClosedForSend) throw sendException
            if (!success && BufferedChannel<E>::is_closed_for_send()) {
                throw_send_exception();
            }
        }

        (void)continuation;
        return nullptr;
    }

    // Line 186-203: override fun trySend(element: E): ChannelResult<Unit>
    ChannelResult<void> try_send(E element) override {
        std::lock_guard<std::recursive_mutex> guard(lock_);

        // Line 188: if (isClosedForSend) return super.trySend(element)
        if (BufferedChannel<E>::is_closed_for_send()) {
            return BufferedChannel<E>::try_send(std::move(element));
        }

        // Line 191: val shouldSuspend = subscribers.any { it.shouldSendSuspend() }
        bool should_suspend = std::any_of(subscribers_.begin(), subscribers_.end(),
            [](const std::shared_ptr<BufferedChannel<E>>& sub) {
                return sub->should_send_suspend();
            });

        // Line 192: if (shouldSuspend) return ChannelResult.failure()
        if (should_suspend) {
            return ChannelResult<void>::failure();
        }

        // Line 194: if (capacity == CONFLATED) lastConflatedElement = element
        if (capacity_ == Channel<E>::CONFLATED) {
            last_conflated_element_ = element;
            has_last_conflated_element_ = true;
        }

        // Line 200: subscribers.forEach { it.trySend(element) }
        for (auto& sub : subscribers_) {
            sub->try_send(element);
        }

        // Line 202: return ChannelResult.success(Unit)
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
        // Line 228-235: First, check whether this `onSend` clause is already
        // selected, finishing immediately in this case.
        {
            std::lock_guard<std::recursive_mutex> guard(lock_);
            // Line 229: val result = onSendInternalResult.remove(select)
            auto it = on_send_internal_result_.find(static_cast<void*>(select));
            if (it != on_send_internal_result_.end()) {
                // Line 230-233: already selected!
                void* result = it->second;
                on_send_internal_result_.erase(it);
                // `result` is either Unit (nullptr) or CHANNEL_CLOSED.
                select->select_in_registration_phase(result);
                return;
            }
        }

        // Line 238-272: Start a new coroutine that performs plain `send(..)`
        // and tries to select this `onSend` clause at the end.
        // Kotlin: CoroutineScope(select.context).launch(start = CoroutineStart.UNDISPATCHED) { ... }
        auto context = select->get_context();

        // The launch block - captured by value for safety
        auto self = this;
        auto launch_block = [self, select, element]() {
            // Line 239-250: val success: Boolean = try { send(element); true } catch...
            bool success = false;
            try {
                // Blocking send for now - Kotlin would suspend
                self->send(element, nullptr);
                // Line 241-242: The element has been successfully sent!
                success = true;
            } catch (const ClosedSendChannelException&) {
                // Line 248: if (isClosedForSend && (t is ClosedSendChannelException...)) false
                if (self->is_closed_for_send()) {
                    success = false;
                } else {
                    throw;
                }
            } catch (...) {
                // Line 249: else throw t
                if (self->is_closed_for_send()) {
                    success = false;
                } else {
                    throw;
                }
            }

            // Line 253-270: Mark this `onSend` clause as selected and
            // try to complete the `select` operation.
            {
                std::lock_guard<std::recursive_mutex> guard(self->lock_);
                // Line 255: assert { onSendInternalResult[select] == null }
                assert(self->on_send_internal_result_.find(static_cast<void*>(select)) ==
                       self->on_send_internal_result_.end());
                // Line 257: onSendInternalResult[select] = if (success) Unit else CHANNEL_CLOSED
                self->on_send_internal_result_[static_cast<void*>(select)] =
                    success ? nullptr : static_cast<void*>(&CHANNEL_CLOSED());
                // Line 260: val trySelectResult = select.trySelectDetailed(this@BroadcastChannelImpl, Unit)
                bool selected = select->try_select(self, nullptr);
                // Line 261-268: if (trySelectResult !== TrySelectDetailedResult.REREGISTER)
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

    // Line 280-290: override fun close(cause: Throwable?): Boolean
    bool close(std::exception_ptr cause = nullptr) override {
        std::lock_guard<std::recursive_mutex> guard(lock_);

        // Line 282: subscribers.forEach { it.close(cause) }
        for (auto& sub : subscribers_) {
            sub->close(cause);
        }

        // Line 287: subscribers = subscribers.filter { it.hasElements() }
        subscribers_.erase(
            std::remove_if(subscribers_.begin(), subscribers_.end(),
                [](const std::shared_ptr<BufferedChannel<E>>& sub) {
                    return !sub->has_elements();
                }),
            subscribers_.end());

        // Line 289: super.close(cause)
        return BufferedChannel<E>::close(cause);
    }

    // Line 292-300: override fun cancelImpl(cause: Throwable?): Boolean
    bool cancel_impl(std::exception_ptr cause) {
        std::lock_guard<std::recursive_mutex> guard(lock_);

        // Line 295: subscribers.forEach { it.cancelImpl(cause) }
        for (auto& sub : subscribers_) {
            sub->cancel(cause);
        }

        // Line 297: lastConflatedElement = NO_ELEMENT
        has_last_conflated_element_ = false;

        // Line 299: super.cancelImpl(cause)
        return BufferedChannel<E>::cancel_impl(cause);
    }

    // BroadcastChannel::cancel delegates to cancelImpl
    void cancel(std::exception_ptr cause = nullptr) override {
        cancel_impl(cause);
    }

    // Line 302-304: override val isClosedForSend: Boolean
    bool is_closed_for_send() const override {
        std::lock_guard<std::recursive_mutex> guard(lock_);
        return BufferedChannel<E>::is_closed_for_send();
    }

    // ########################################
    // # ConflatedBroadcastChannel Operations #
    // ########################################

    // Line 331-340: val value: E
    E get_value() const {
        std::lock_guard<std::recursive_mutex> guard(lock_);

        // Line 333-335: if (isClosedForSend) throw closeCause ?: IllegalStateException
        if (BufferedChannel<E>::is_closed_for_send()) {
            auto cause = BufferedChannel<E>::close_cause();
            if (cause) {
                std::rethrow_exception(cause);
            }
            throw std::logic_error("This broadcast channel is closed");
        }

        // Line 337: if (lastConflatedElement === NO_ELEMENT) error("No value")
        if (!has_last_conflated_element_) {
            throw std::logic_error("No value");
        }

        // Line 339: lastConflatedElement as E
        return last_conflated_element_;
    }

    // Line 343-350: val valueOrNull: E?
    std::optional<E> get_value_or_null() const {
        std::lock_guard<std::recursive_mutex> guard(lock_);

        // Line 345: if (isClosedForReceive) null
        if (BufferedChannel<E>::is_closed_for_receive()) {
            return std::nullopt;
        }

        // Line 347: else if (lastConflatedElement === NO_ELEMENT) null
        if (!has_last_conflated_element_) {
            return std::nullopt;
        }

        // Line 349: else lastConflatedElement as E
        return last_conflated_element_;
    }

    // #################
    // # For Debugging #
    // #################

    // Line 356-359: override fun toString()
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

    // Line 310-316: private inner class SubscriberBuffered
    class SubscriberBuffered : public BufferedChannel<E> {
        BroadcastChannelImpl* broadcast_;
    public:
        SubscriberBuffered(BroadcastChannelImpl* broadcast, int capacity)
            : BufferedChannel<E>(capacity, nullptr),
              broadcast_(broadcast) {}

        // Line 311-315: public override fun cancelImpl(cause: Throwable?): Boolean
        bool cancel_impl(std::exception_ptr cause) override {
            broadcast_->remove_subscriber(this);
            return BufferedChannel<E>::cancel_impl(cause);
        }

        void cancel(std::exception_ptr cause = nullptr) override {
            cancel_impl(cause);
        }
    };

    // Line 318-324: private inner class SubscriberConflated
    class SubscriberConflated : public ConflatedBufferedChannel<E> {
        BroadcastChannelImpl* broadcast_;
    public:
        explicit SubscriberConflated(BroadcastChannelImpl* broadcast)
            : ConflatedBufferedChannel<E>(1, BufferOverflow::DROP_OLDEST, nullptr),
              broadcast_(broadcast) {}

        // Line 319-323: public override fun cancelImpl(cause: Throwable?): Boolean
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


