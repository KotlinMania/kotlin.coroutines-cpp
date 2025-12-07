#pragma once
#include "Channel.hpp"
#include <deque>
#include <condition_variable>
#include <mutex>
#include <functional>

namespace kotlinx {
namespace coroutines {
namespace channels {

/**
 * The buffered channel implementation, which also serves as a rendezvous channel when capacity is zero.
 *
 * This is a simplified implementation using mutexes and deques for basic functionality.
 * A full implementation would use the lock-free algorithm from the Kotlin coroutines paper.
 */
template <typename E>
class BufferedChannel : public Channel<E> {
public:
    /**
     * Creates a new BufferedChannel with the specified capacity.
     * @param capacity Channel capacity; use Channel::RENDEZVOUS for rendezvous channel
     *                 and Channel::UNLIMITED for unlimited capacity.
     * @param onUndeliveredElement Optional handler for elements that cannot be delivered
     */
    BufferedChannel(int capacity, OnUndeliveredElement<E> onUndeliveredElement = nullptr)
        : capacity_(capacity), onUndeliveredElement_(onUndeliveredElement), closed_(false) {
        if (capacity < 0 && capacity != Channel<E>::UNLIMITED) {
            throw std::invalid_argument("Invalid channel capacity: " + std::to_string(capacity) + ", should be >= 0");
        }
    }

    virtual ~BufferedChannel() = default;

    // SendChannel interface implementation
    bool is_closed_for_send() const override {
        std::lock_guard<std::mutex> lock(mtx);
        return closed_;
    }

    void send(E element) override {
        std::unique_lock<std::mutex> lock(mtx);
        not_full.wait(lock, [this]() {
            return closed_ || (capacity_ == Channel<E>::UNLIMITED) ||
                   (static_cast<int>(buffer.size()) < capacity_);
        });

        if (closed_) {
            if (closeCause_) {
                std::rethrow_exception(closeCause_);
            }
            throw ClosedSendChannelException("Channel was closed");
        }

        buffer.push_back(std::move(element));
        lock.unlock();
        not_empty.notify_one();
    }

    ChannelResult<void> try_send(E element) override {
        std::lock_guard<std::mutex> lock(mtx);
        if (closed_) {
            return ChannelResult<void>::closed(closeCause_);
        }

        if (capacity_ != Channel<E>::UNLIMITED && static_cast<int>(buffer.size()) >= capacity_) {
            return ChannelResult<void>::failure();
        }

        buffer.push_back(std::move(element));
        not_empty.notify_one();
        return ChannelResult<void>::success();
    }

    bool close(std::exception_ptr cause = nullptr) override {
        std::lock_guard<std::mutex> lock(mtx);
        if (closed_) {
            return false;
        }
        closed_ = true;
        closeCause_ = cause;
        not_full.notify_all();
        not_empty.notify_all();

        // Call close handlers
        for (auto& handler : closeHandlers_) {
            try {
                handler(cause);
            } catch (...) {
                // Ignore exceptions in close handlers
            }
        }

        // Call onUndeliveredElement for remaining elements
        if (onUndeliveredElement_) {
            for (const auto& element : buffer) {
                try {
                    onUndeliveredElement_(element, cause);
                } catch (...) {
                    // Ignore exceptions in onUndeliveredElement
                }
            }
        }
        buffer.clear();

        return true;
    }

    void invoke_on_close(std::function<void(std::exception_ptr)> handler) override {
        std::lock_guard<std::mutex> lock(mtx);
        if (closed_) {
            handler(closeCause_);
        } else {
            closeHandlers_.push_back(handler);
        }
    }

    // ReceiveChannel interface implementation
    bool is_closed_for_receive() const override {
        std::lock_guard<std::mutex> lock(mtx);
        return closed_ && buffer.empty();
    }

    bool is_empty() const override {
        std::lock_guard<std::mutex> lock(mtx);
        return buffer.empty() && !closed_;
    }

    E receive() override {
        std::unique_lock<std::mutex> lock(mtx);
        not_empty.wait(lock, [this]() {
            return closed_ || !buffer.empty();
        });

        if (!buffer.empty()) {
            E element = std::move(buffer.front());
            buffer.pop_front();
            lock.unlock();
            not_full.notify_one();
            return element;
        }

        if (closed_) {
            if (closeCause_) {
                std::rethrow_exception(closeCause_);
            }
            throw ClosedReceiveChannelException("Channel was closed");
        }

        throw std::runtime_error("Unexpected state in receive()");
    }

    ChannelResult<E> receive_catching() override {
        std::unique_lock<std::mutex> lock(mtx);
        not_empty.wait(lock, [this]() {
            return closed_ || !buffer.empty();
        });

        if (!buffer.empty()) {
            E element = std::move(buffer.front());
            buffer.pop_front();
            lock.unlock();
            not_full.notify_one();
            return ChannelResult<E>::success(std::move(element));
        }

        if (closed_) {
            return ChannelResult<E>::closed(closeCause_);
        }

        throw std::runtime_error("Unexpected state in receive_catching()");
    }

    ChannelResult<E> try_receive() override {
        std::lock_guard<std::mutex> lock(mtx);
        if (!buffer.empty()) {
            E element = std::move(buffer.front());
            buffer.pop_front();
            not_full.notify_one();
            return ChannelResult<E>::success(std::move(element));
        }

        if (closed_) {
            return ChannelResult<E>::closed(closeCause_);
        }

        return ChannelResult<E>::failure();
    }

    std::shared_ptr<ChannelIterator<E>> iterator() override {
        return std::make_shared<BufferedChannelIterator>(this);
    }

    void cancel(std::exception_ptr cause = nullptr) override {
        close(cause ? cause : std::make_exception_ptr(std::runtime_error("Channel was cancelled")));
    }

private:
    /**
     * Iterator implementation for BufferedChannel.
     */
    class BufferedChannelIterator : public ChannelIterator<E> {
    public:
        BufferedChannelIterator(BufferedChannel<E>* channel) : channel_(channel), has_next_(false) {}

        bool has_next() override {
            if (has_next_) {
                return true;
            }

            auto result = channel_->try_receive();
            if (result.is_success()) {
                next_element_ = std::move(*result.get_or_null());
                has_next_ = true;
                return true;
            } else if (result.is_closed()) {
                if (result.exception_or_null()) {
                    std::rethrow_exception(result.exception_or_null());
                }
                return false; // Channel closed normally
            }
            return false; // No element available
        }

        E next() override {
            if (!has_next_ && !has_next()) {
                throw std::runtime_error("No more elements in channel");
            }
            has_next_ = false;
            return std::move(next_element_);
        }

    private:
        BufferedChannel<E>* channel_;
        bool has_next_;
        E next_element_;
    };

protected:
    // For ConflatedBufferedChannel to access
    int capacity_;
    OnUndeliveredElement<E> onUndeliveredElement_;
    std::atomic<bool> closed_;
    std::exception_ptr closeCause_;
    std::deque<E> buffer;
    mutable std::mutex mtx;
    std::condition_variable not_empty;
    std::condition_variable not_full;
    std::vector<std::function<void(std::exception_ptr)>> closeHandlers_;
};

} // namespace channels
} // namespace coroutines
} // namespace kotlinx