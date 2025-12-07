#pragma once
#include "kotlinx/coroutines/channels/Channel.hpp"
#include <atomic>
#include <memory>
#include <vector>
#include <deque>
#include <condition_variable>
#include <mutex>

namespace kotlinx {
namespace coroutines {
namespace channels {

template <typename E>
struct ChannelSegment {
    long long id;
    ChannelSegment<E>* prev;
    ChannelSegment<E>* next;
    // Data storage (implied, likely an array of std::optional<E> or similar)
    // std::vector<E> data; 
    
    // Pointers/State for the segment algorithms
    
    ChannelSegment(long long id, ChannelSegment<E>* prev) : id(id), prev(prev), next(nullptr) {}
    virtual ~ChannelSegment() = default;
    
    // Stub methods for logic used in BufferedChannel
    void cleanPrev() { 
        if (prev) { delete prev; prev = nullptr; } // rudimentary cleanup
    }
    void onSlotCleaned() {}
};

/**
 * The buffered channel implementation, which also serves as a rendezvous channel when the capacity is zero.
 * The high-level structure bases on a conceptually infinite array for storing elements and waiting requests,
 * separate counters of [send] and [receive] invocations that were ever performed, and an additional counter
 * that indicates the end of the logical buffer by counting the number of array cells it ever contained.
 *
 * The key idea is that both [send] and [receive] start by incrementing their counters, assigning the array cell
 * referenced by the counter. In case of rendezvous channels, the operation either suspends and stores its continuation
 * in the cell or makes a rendezvous with the opposite request.
 *
 * Please see the ["Fast and Scalable Channels in Kotlin Coroutines"](https://arxiv.org/abs/2211.04986)
 * paper by Nikita Koval, Roman Elizarov, and Dan Alistarh for the detailed algorithm description.
 */
template <typename E>
class BufferedChannel : public Channel<E> {
public:
    int capacity;

    BufferedChannel(int capacity) : capacity(capacity) {
        // init logic
    }

    virtual ~BufferedChannel() = default;

protected:
    // State
    std::deque<E> buffer;
    mutable std::mutex mtx;
    std::condition_variable not_empty;
    std::condition_variable not_full;
    bool closed = false;
    std::exception_ptr close_cause = nullptr;
    std::vector<std::function<void(std::exception_ptr)>> close_handlers;

public:
    // SendChannel overrides
    bool is_closed_for_send() const override {
        std::lock_guard<std::mutex> lock(mtx);
        return closed;
    }

    void send(E element) override {
        std::unique_lock<std::mutex> lock(mtx);
        if (closed) {
             throw std::runtime_error("Channel is closed for send");
        }
        // Suspension logic (blocking for now)
        if (capacity != Channel<E>::UNLIMITED) {
            not_full.wait(lock, [this]() { 
                return (buffer.size() < (size_t)capacity) || closed; 
            });
        }
        if (closed) {
             throw std::runtime_error("Channel is closed for send");
        }
        buffer.push_back(element);
        not_empty.notify_one();
    }

    ChannelResult<void> try_send(E element) override {
        std::lock_guard<std::mutex> lock(mtx);
        if (closed) return ChannelResult<void>::closed_result(close_cause);
        if (capacity != Channel<E>::UNLIMITED && buffer.size() >= (size_t)capacity) {
            return ChannelResult<void>::failure();
        }
        buffer.push_back(element);
        not_empty.notify_one();
        return ChannelResult<void>::success(nullptr);
    }

    bool close(std::exception_ptr cause = nullptr) override {
        std::lock_guard<std::mutex> lock(mtx);
        if (closed) return false;
        closed = true;
        close_cause = cause;
        not_empty.notify_all();
        not_full.notify_all();
        
        // Invoke handlers
        for (auto& handler : close_handlers) {
            // handlers should be safe, or we wrap in try-catch
            try { handler(cause); } catch(...) {}
        }
        close_handlers.clear();
        return true;
    }

    void invoke_on_close(std::function<void(std::exception_ptr)> handler) override {
        std::lock_guard<std::mutex> lock(mtx);
        if (closed) {
            // Already closed, invoke immediately
            try { handler(close_cause); } catch(...) {}
        } else {
            close_handlers.push_back(handler);
        }
    }

    // ReceiveChannel overrides
    bool is_closed_for_receive() const override {
        std::lock_guard<std::mutex> lock(mtx);
        return closed && buffer.empty();
    }

    bool is_empty() const override {
        std::lock_guard<std::mutex> lock(mtx);
        return buffer.empty();
    }

    E receive() override {
        std::unique_lock<std::mutex> lock(mtx);
        not_empty.wait(lock, [this]() { 
            return !buffer.empty() || closed; 
        });
        
        if (buffer.empty() && closed) {
             // throw ClosedReceiveChannelException
             throw std::runtime_error("Channel is closed for receive");
        }
        
        E val = buffer.front();
        buffer.pop_front();
        not_full.notify_one();
        return val;
    }

    ChannelResult<E> receive_catching() override {
         std::unique_lock<std::mutex> lock(mtx);
         not_empty.wait(lock, [this]() { 
            return !buffer.empty() || closed; 
        });
        
        if (buffer.empty() && closed) {
             return ChannelResult<E>::closed_result(close_cause);
        }
        
        E val = buffer.front();
        buffer.pop_front(); // ERROR: Val copy?
        // E needs to be copyable or movable.
        not_full.notify_one();
        // Here we need to return a pointer or wrapper. ChannelResult holds a pointer?
        // ChannelResult definition in Channel.hpp expects T*. 
        // This is tricky for value types.
        // We might need to change ChannelResult design or allocate.
        // For now, let's assume we can new it, or change ChannelResult.
        return ChannelResult<E>::success(new E(val)); // Memory leak risk?
    }

    ChannelResult<E> try_receive() override {
        std::lock_guard<std::mutex> lock(mtx);
        if (buffer.empty()) {
            if (closed) return ChannelResult<E>::closed_result(close_cause);
            return ChannelResult<E>::failure();
        }
        E val = buffer.front();
        buffer.pop_front();
        not_full.notify_one();
        return ChannelResult<E>::success(new E(val));
    }

    std::shared_ptr<ChannelIterator<E>> iterator() override {
        // Implement iterator
        return std::make_shared<BufferedChannelIterator>(this);
    }

    void cancel(std::exception_ptr cause = nullptr) override {
        close(cause);
    }
    
private:
    class BufferedChannelIterator : public ChannelIterator<E> {
        BufferedChannel<E>* channel;
        E nextVal;
        bool hasNextVal = false;
    public:
        BufferedChannelIterator(BufferedChannel<E>* c) : channel(c) {}
        
        bool has_next() override {
             auto result = channel->receive_catching();
             if (result.is_success()) {
                 nextVal = *result.get_or_null();
                 delete result.get_or_null(); // Cleanup if we allocated
                 hasNextVal = true;
                 return true;
             }
             return false;
        }
        
        E next() override {
            if (!hasNextVal) throw std::runtime_error("No next element");
            hasNextVal = false;
            return nextVal;
        }
    };
};

} // namespace channels
} // namespace coroutines
} // namespace kotlinx
