#pragma once
#include "Channel.hpp"
#include "kotlinx/coroutines/CancellableContinuation.hpp"
#include <deque>
#include <mutex>
#include <functional>
#include <optional>

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

    virtual ~BufferedChannel() {
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
            return ChannelResult<void>::success();
        }

        // Buffer full
        return ChannelResult<void>::failure();
    }

    ChannelAwaiter<void> send(E element) override {
        // Fast path
        auto result = try_send(element);
        if (result.is_success()) return ChannelAwaiter<void>(); // Ready

        if (result.is_closed()) {
             auto cause = result.exception_or_null();
             if (cause) std::rethrow_exception(cause);
             throw ClosedSendChannelException("Channel was closed");
        }

        /*
         * TODO: STUB - Channel send suspension not implemented
         *
         * Kotlin source: BufferedChannel.send() in BufferedChannel.kt
         *
         * What's missing:
         * - Should suspend using suspend_cancellable_coroutine pattern:
         *   return suspendCancellableCoroutine { cont ->
         *       // Add to senders queue
         *       // Register cancellation handler to remove from queue
         *       // Resume when receiver takes the element
         *   }
         * - Requires proper Continuation<void*>* parameter (Kotlin-style suspend)
         * - Need senders queue to track waiting senders
         *
         * Current behavior: Returns "needs_suspend" flag but doesn't actually suspend
         *   Caller must handle this by polling or using try_send()
         * Correct behavior: Suspend coroutine until buffer has space or receiver ready
         *
         * Workaround: Use try_send() in a loop with delay, or use unlimited buffer
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
         * TODO: STUB - Channel receive suspension not implemented
         *
         * Kotlin source: BufferedChannel.receive() in BufferedChannel.kt
         *
         * What's missing:
         * - Should suspend using suspend_cancellable_coroutine pattern:
         *   return suspendCancellableCoroutine { cont ->
         *       // Add to receivers queue
         *       // Register cancellation handler to remove from queue
         *       // Resume with value when sender provides element
         *   }
         * - Requires proper Continuation<void*>* parameter (Kotlin-style suspend)
         * - Need receivers queue to track waiting receivers
         *
         * Current behavior: Returns "needs_suspend" flag but doesn't actually suspend
         *   Caller must handle this by polling or using try_receive()
         * Correct behavior: Suspend coroutine until element available
         *
         * Workaround: Use try_receive() in a loop with delay
         */
        return ChannelAwaiter<E>(true /* needs_suspend */, nullptr);
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
        return ChannelAwaiter<ChannelResult<E>>(true /* needs_suspend */, nullptr);
    }

    ChannelResult<E> try_receive() override {
        std::lock_guard<std::mutex> lock(mtx);

        if (!buffer_.empty()) {
            E el = std::move(buffer_.front());
            buffer_.pop_front();
            return ChannelResult<E>::success(std::move(el));
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

    std::vector<std::function<void(std::exception_ptr)>> close_handlers_;
};

} // channels
} // coroutines
} // kotlinx
