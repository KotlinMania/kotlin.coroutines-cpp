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
 * TODO: This implementation needs to be rewritten to use Kotlin-style coroutines
 * (Continuation<void*>* pattern) instead of C++20 coroutines.
 *
 * For now, send() and receive() that would suspend throw an exception.
 * Use try_send() and try_receive() for non-suspending operations.
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

        // Slow path: would need to suspend
        // TODO: Implement proper Kotlin-style suspension
        // For now, return an awaiter that indicates suspension is needed
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

        // Slow path: would need to suspend
        // TODO: Implement proper Kotlin-style suspension
        // For now, return an awaiter that indicates suspension is needed
        return ChannelAwaiter<E>(true /* needs_suspend */, nullptr);
    }

    ChannelAwaiter<ChannelResult<E>> receive_catching() override {
        auto result = try_receive();
        if (result.is_success() || result.is_closed()) {
            return ChannelAwaiter<ChannelResult<E>>(std::move(result));
        }

        // Slow path: would need to suspend
        // TODO: Implement proper Kotlin-style suspension
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
        // TODO: Implement timeout
        return ChannelAwaiter<ChannelResult<E>>(ChannelResult<E>::failure());
    }

    std::shared_ptr<ChannelIterator<E>> iterator() override {
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
