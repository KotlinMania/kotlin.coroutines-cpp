#pragma once
// Kotlin source: kotlinx-coroutines-core/common/src/channels/BufferedChannel.kt
//
// Kotlin imports:
// - kotlinx.coroutines.*
// - kotlinx.coroutines.channels.ChannelResult.Companion.{closed, failure, success}
// - kotlinx.coroutines.internal.*
// - kotlinx.coroutines.selects.*
// - kotlinx.coroutines.selects.TrySelectDetailedResult.*
#include "kotlinx/coroutines/channels/Channel.hpp"
#include "kotlinx/coroutines/CancellableContinuation.hpp"
#include "kotlinx/coroutines/Job.hpp"
#include "kotlinx/coroutines/CoroutineScope.hpp"
#include "kotlinx/coroutines/internal/Symbol.hpp"
#include "kotlinx/coroutines/internal/Concurrent.hpp"
#include "kotlinx/coroutines/selects/Select.hpp"
#include <deque>
#include <mutex>
#include <functional>
#include <optional>
#include <memory>
#include <string>

namespace kotlinx {
namespace coroutines {
namespace channels {

/**
 * The buffered channel implementation.
 *
 * BufferedChannel provides a channel with a configurable buffer capacity that
 * allows producers and consumers to operate asynchronously. Elements sent to
 * the channel are stored in the buffer until received by consumers.
 *
 * Buffer capacity determines the behavior:
 * - RENDEZVOUS (0): No buffer, sender and receiver must meet
 * - UNLIMITED: Infinite buffer, sender never suspends
 * - CONFLATED (-1): Buffer size 1, new element replaces existing
 * - Positive N: Fixed buffer of size N
 * TODO: Implement Kotlin-style coroutines (Continuation<void*>* pattern)
 * @note This implementation needs to be rewritten to use Kotlin-style coroutines
 *       (Continuation<void*>* pattern) instead of C++20 coroutines.
 *       For now, send() and receive() that would suspend return a needs_suspend flag.
 *       Use try_send() and try_receive() for non-suspending operations.
 *
 * @tparam E The type of elements in the channel.
 */
template <typename E>
class BufferedChannel : public Channel<E> {
public:
    BufferedChannel(int capacity, OnUndeliveredElement<E> onUndeliveredElement = nullptr)
        : capacity_(capacity), on_undelivered_element_(onUndeliveredElement), closed_(false) {
        if (capacity < 0 && capacity != Channel<E>::UNLIMITED) {
            throw std::invalid_argument("Invalid channel capacity: " + std::to_string(capacity));
        }
    }

    ~BufferedChannel() override {
        close(std::make_exception_ptr(std::runtime_error("Channel destroyed")));
    }

    // SendChannel interface
    bool is_closed_for_send() const override {
        std::lock_guard<std::mutex> lock(mtx);
        return closed_;
    }

    ChannelResult<void> try_send(E element) override {
        std::unique_lock<std::mutex> lock(mtx);
        if (closed_) return ChannelResult<void>::closed(close_cause_);

        // Space in buffer?
        if (capacity_ == Channel<E>::UNLIMITED ||
           (static_cast<int>(buffer_.size()) < capacity_)) {
            buffer_.push_back(std::move(element));
            try_resume_receivers(); // Resume any waiting receivers
            return ChannelResult<void>::success();
        }

        // Buffer full - check if there are waiting receivers (rendezvous)
        if (!receivers_queue_.empty()) {
            // Direct handoff to waiting receiver
            auto receiver = receivers_queue_.front();
            receivers_queue_.pop_front();
            // TODO: receiver->resume(element);
            return ChannelResult<void>::success();
        }

        // Buffer full and no waiting receivers
        return ChannelResult<void>::failure();
    }

    ChannelAwaiter<void> send(E element) override {
        // Fast path - try to send immediately
        auto result = try_send(element);
        if (result.is_success()) {
            try_resume_receivers(); // Try to resume any waiting receivers
            return ChannelAwaiter<void>(); // Ready
        }

        if (result.is_closed()) {
             auto cause = result.exception_or_null();
             if (cause) std::rethrow_exception(cause);
             throw ClosedSendChannelException(std::string("Channel was closed"));
        }

        /*
         * SUSPENSION INFRASTRUCTURE PLACEHOLDER
         *
         * In a proper implementation, this would use suspend_cancellable_coroutine:
         *
         * return suspend_cancellable_coroutine<void>([&](auto& continuation) {
         *     enqueue_sender(continuation.shared_from_this(), std::move(element));
         *     continuation.invoke_on_cancellation([&]() {
         *         // Remove from senders queue if cancelled
         *         remove_sender_from_queue(continuation.shared_from_this());
         *     });
         * }, caller_continuation);
         *
         * For now: Return needs_suspend flag (existing behavior)
         * TODO: Replace with actual suspension once Continuation interface is updated
         */
        return ChannelAwaiter<void>(true /* needs_suspend */);
    }

    bool close(std::exception_ptr cause = nullptr) override {
        {
            std::lock_guard<std::mutex> lock(mtx);
            if (closed_) return false;

            closed_ = true;
            close_cause_ = cause;
        }

        for (auto& handler : close_handlers_) {
            try { handler(cause); } catch (...) {}
        }

        return true;
    }

    void invoke_on_close(std::function<void(std::exception_ptr)> handler) override {
        std::lock_guard<std::mutex> lock(mtx);
        if (closed_) {
             try { handler(close_cause_); } catch (...) {}
        } else {
            close_handlers_.push_back(handler);
        }
    }

    // ReceiveChannel interface
    bool is_closed_for_receive() const override {
        std::lock_guard<std::mutex> lock(mtx);
        return closed_ && buffer_.empty();
    }

    bool is_empty() const override {
        std::lock_guard<std::mutex> lock(mtx);
        return buffer_.empty();
    }

    ChannelAwaiter<E> receive() override {
        auto result = try_receive();
        if (result.is_success()) {
            return ChannelAwaiter<E>(result.get_or_throw()); // Ready with value
        }
        if (result.is_closed()) {
             if (result.exception_or_null()) std::rethrow_exception(result.exception_or_null());
             throw ClosedReceiveChannelException("Channel closed");
        }

        /*
         * SUSPENSION INFRASTRUCTURE PLACEHOLDER
         *
         * In a proper implementation, this would use suspend_cancellable_coroutine:
         *
         * return suspend_cancellable_coroutine<E>([&](auto& continuation) {
         *     enqueue_receiver(continuation.shared_from_this());
         *     continuation.invoke_on_cancellation([&]() {
         *         // Remove from receivers queue if cancelled
         *         remove_receiver_from_queue(continuation.shared_from_this());
         *     });
         * }, caller_continuation);
         *
         * For now: Return needs_suspend flag (existing behavior)
         * TODO: Replace with actual suspension once Continuation interface is updated
         */
        return ChannelAwaiter<E>(true /* needs_suspend */);
    }

    ChannelAwaiter<ChannelResult<E>> receive_catching() override {
        auto result = try_receive();
        if (result.is_success() || result.is_closed()) {
            return ChannelAwaiter<ChannelResult<E>>(std::move(result));
        }

        /*
         * TODO: STUB - Channel receiveCatching suspension not implemented
         *
         * Kotlin source: BufferedChannel.receiveCatching() in BufferedChannel.kt
         *
         * What's missing:
         * - Should suspend like receive() but wrap result in ChannelResult
         * - On success: ChannelResult.success(value)
         * - On close: ChannelResult.closed(cause) without throwing
         * - Requires same suspension infrastructure as receive()
         *
         * Current behavior: Returns "needs_suspend" flag but doesn't actually suspend
         * Correct behavior: Suspend until element available, return ChannelResult wrapper
         *
         * Workaround: Use try_receive() in a polling loop
         */
        return ChannelAwaiter<ChannelResult<E>>(true /* needs_suspend */);
    }

    ChannelResult<E> try_receive() override {
        std::lock_guard<std::mutex> lock(mtx);

        if (!buffer_.empty()) {
            E el = std::move(buffer_.front());
            buffer_.pop_front();
            try_resume_senders(); // Resume any waiting senders
            return ChannelResult<E>::success(std::move(el));
        }

        // Check if there are waiting senders (rendezvous)
        if (!senders_queue_.empty()) {
            // Direct handoff from waiting sender
            auto sender = senders_queue_.front();
            senders_queue_.pop_front();
            // TODO: Get element from sender and resume sender
            // For now, return failure since we can't access the element
            return ChannelResult<E>::failure();
        }

        if (closed_) return ChannelResult<E>::closed(close_cause_);

        return ChannelResult<E>::failure();
    }

    ChannelAwaiter<ChannelResult<E>> receive_with_timeout(long /*timeout*/) override {
        /*
         * TODO: STUB - Channel receive with timeout not implemented
         *
         * Kotlin source: Not a direct Kotlin API - this is a convenience extension
         *
         * What's missing:
         * - Should use withTimeout {} wrapper around receive()
         * - Requires: kotlinx.coroutines.withTimeout() implementation
         * - Requires: TimeoutCancellationException handling
         * - On timeout: return ChannelResult.closed() or throw
         *
         * Current behavior: Always returns failure immediately
         * Correct behavior: Wait up to timeout ms, then return result or timeout
         *
         * Dependencies:
         * - withTimeout() coroutine builder
         * - Delay/timer infrastructure
         * - receive() suspension working properly
         *
         * Workaround: Implement timeout externally with a timer and try_receive() polling
         */
        return ChannelAwaiter<ChannelResult<E>>(ChannelResult<E>::failure());
    }

    std::shared_ptr<ChannelIterator<E>> iterator() override {
        /*
         * TODO: STUB - Channel iterator not implemented
         *
         * Kotlin source: BufferedChannel.iterator() in BufferedChannel.kt
         *
         * What's missing:
         * - Should return a ChannelIterator that supports:
         *   - hasNext(): suspend function returning bool
         *   - next(): returns cached element from hasNext()
         * - hasNext() calls receive() internally, caches result
         * - Iterator enables for-each loops: for (item in channel) { ... }
         *
         * Current behavior: Returns nullptr - iteration not supported
         * Correct behavior: Return working ChannelIterator instance
         *
         * Dependencies:
         * - ChannelIterator class implementation
         * - receive() suspension working properly
         * - State machine for hasNext()/next() coordination
         *
         * Workaround: Use while loop with try_receive() manually
         */
        return nullptr;
    }

    // Select clause stubs (returning nullptr keeps API consistent without concrete impl yet)
    std::shared_ptr<selects::SelectClause2<E, SendChannel<E>>> on_send() override {
        return nullptr;
    }

    std::shared_ptr<selects::SelectClause1<E>> on_receive() override {
        return nullptr;
    }

    void cancel(std::exception_ptr cause = nullptr) override {
        close(cause ? cause : std::make_exception_ptr(std::runtime_error("Channel cancelled")));
    }

protected:
    int capacity_;
    OnUndeliveredElement<E> on_undelivered_element_;
    bool closed_;
    std::exception_ptr close_cause_;

    mutable std::mutex mtx;
    std::deque<E> buffer_;

    // Continuation queues for suspended operations
    std::deque<std::shared_ptr<CancellableContinuation<void>>> senders_queue_;
    std::deque<std::shared_ptr<CancellableContinuation<E>>> receivers_queue_;

    std::vector<std::function<void(std::exception_ptr)>> close_handlers_;

    // Queue management functions
    void enqueue_sender(std::shared_ptr<CancellableContinuation<void>> sender, E element) {
        std::lock_guard<std::mutex> lock(mtx);
        senders_queue_.push_back(sender);
        // TODO: Store element with sender for rendezvous channels
        (void)element; // Suppress unused parameter warning
    }

    void enqueue_receiver(std::shared_ptr<CancellableContinuation<E>> receiver) {
        std::lock_guard<std::mutex> lock(mtx);
        receivers_queue_.push_back(receiver);
    }

    void try_resume_senders() {
        std::lock_guard<std::mutex> lock(mtx);
        while (!senders_queue_.empty() && (buffer_.size() < static_cast<size_t>(capacity_) || capacity_ == Channel<E>::UNLIMITED)) {
            auto sender = senders_queue_.front();
            senders_queue_.pop_front();
            // TODO: Resume sender with success
            // sender->resume();
        }
    }

    void try_resume_receivers() {
        std::lock_guard<std::mutex> lock(mtx);
        while (!receivers_queue_.empty() && !buffer_.empty()) {
            auto receiver = receivers_queue_.front();
            receivers_queue_.pop_front();
            E element = std::move(buffer_.front());
            buffer_.pop_front();
            // TODO: Resume receiver with element
            // receiver->resume(element);
        }
    }
};

} // channels
} // coroutines
} // kotlinx
