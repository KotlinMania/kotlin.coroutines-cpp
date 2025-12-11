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
 * Refactored to use suspending coroutines (suspend_cancellable_coroutine) and ChannelAwaiter.
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

        // 1. Check if there are waiting receivers
        while (!receivers_.empty()) {
             auto* receiver = receivers_.front();
             receivers_.pop_front();
             
             if (receiver->try_resume(element)) {
                 return ChannelResult<void>::success();
             }
        }

        // 2. Space in buffer?
        if (capacity_ == Channel<E>::UNLIMITED || 
           (static_cast<int>(buffer_.size()) < capacity_)) {
            buffer_.push_back(std::move(element));
            return ChannelResult<void>::success();
        }

        // 3. Buffer full
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

        // Slow path: suspend
        return ChannelAwaiter<void>(suspend_cancellable_coroutine<void>([this, element](CancellableContinuation<void>& cont) mutable {
             std::unique_lock<std::mutex> lock(mtx);

             if (closed_) {
                 lock.unlock();
                 auto exc = close_cause_ ? close_cause_ : std::make_exception_ptr(ClosedSendChannelException("Channel closed"));
                 cont.resume_with_exception(exc);
                 return;
             }
             
             while (!receivers_.empty()) {
                  auto* r = receivers_.front();
                  receivers_.pop_front();
                  if (r->try_resume(element)) {
                      lock.unlock();
                      cont.resume();
                      return;
                  }
             }

             if (capacity_ == Channel<E>::UNLIMITED || static_cast<int>(buffer_.size()) < capacity_) {
                 buffer_.push_back(std::move(element));
                 lock.unlock();
                 cont.resume();
                 return;
             }
             
             senders_.push_back({element, &cont});
             
             cont.invoke_on_cancellation([this, &cont](std::exception_ptr) {
                 std::lock_guard<std::mutex> lock(mtx);
                 for (auto it = senders_.begin(); it != senders_.end(); ++it) {
                     if (it->cont == &cont) {
                         senders_.erase(it);
                         break;
                     }
                 }
             });
        }));
    }

    bool close(std::exception_ptr cause = nullptr) override {
        std::vector<CancellableContinuation<void>*> pendingSenders;
        std::vector<CancellableContinuation<E>*> pendingReceivers;
        
        {
            std::lock_guard<std::mutex> lock(mtx);
            if (closed_) return false;
            
            closed_ = true;
            close_cause_ = cause;

            for (auto& s : senders_) pendingSenders.push_back(s.cont);
            senders_.clear();
            
             for (auto* r : receivers_) pendingReceivers.push_back(r);
            receivers_.clear();
        }

        auto exc = cause ? cause : std::make_exception_ptr(ClosedSendChannelException("Channel closed"));

        for (auto* cont : pendingSenders) {
            cont->resume_with_exception(exc);
        }
        
        for (auto* cont : pendingReceivers) {
             cont->resume_with_exception(exc);
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
        // Fast path: try_receive logic inline?
        // Or call try_receive? try_receive moves element.
        // But try_receive returns ChannelResult<E>.
        // ChannelAwaiter<E> needs E.
        
        auto result = try_receive();
        if (result.is_success()) {
            return ChannelAwaiter<E>(result.get_or_throw()); // Ready with value
        }
        if (result.is_closed()) {
             if (result.exception_or_null()) std::rethrow_exception(result.exception_or_null());
             throw ClosedReceiveChannelException("Channel closed");
        }
        
        // Slow path
        return ChannelAwaiter<E>(suspend_cancellable_coroutine<E>([this](CancellableContinuation<E>& cont) {
             std::unique_lock<std::mutex> lock(mtx);
             
            if (!buffer_.empty()) {
                E el = std::move(buffer_.front());
                buffer_.pop_front();
                while (!senders_.empty()) {
                     auto& sender = senders_.front();
                     if (sender.cont->try_resume()) {
                         buffer_.push_back(std::move(sender.element));
                         senders_.pop_front();
                         break; 
                     }
                     senders_.pop_front();
                 }
                lock.unlock();
                cont.resume(el);
                return;
            }
            
            if (closed_) {
                 lock.unlock();
                 cont.resume_with_exception(close_cause_ ? close_cause_ : std::make_exception_ptr(ClosedReceiveChannelException()));
                 return;
            }
            
            while (!senders_.empty()) {
                auto& sender = senders_.front();
                if (sender.cont->try_resume()) {
                    E el = std::move(sender.element);
                    senders_.pop_front();
                    lock.unlock();
                    cont.resume(el);
                    return;
                }
                senders_.pop_front();
            }
            
            receivers_.push_back(&cont);
            
            cont.invoke_on_cancellation([this, &cont](std::exception_ptr) {
                std::lock_guard<std::mutex> lock(mtx);
                for (auto it = receivers_.begin(); it != receivers_.end(); ++it) {
                    if (*it == &cont) {
                        receivers_.erase(it);
                        break;
                    }
                }
            });
        }));
    }
    
    ChannelAwaiter<ChannelResult<E>> receive_catching() override {
        // Wrapper around receive() but catching exceptions?
        // No, receive() will throw if closed. receive_catching returns closed result.
        // We need a separate suspend logic that returns Result instead of E/throw.
        
        // Fast path
        auto result = try_receive();
        if (result.is_success() || result.is_closed()) {
            return ChannelAwaiter<ChannelResult<E>>(std::move(result));
        }
        
        // Slow path: suspend but resume with Result
        return ChannelAwaiter<ChannelResult<E>>(suspend_cancellable_coroutine<ChannelResult<E>>([this](CancellableContinuation<ChannelResult<E>>& cont) {
             // ... Similar logic but resume(Result::success(val)) or resume(Result::closed(cause))
             // Simplified for brevity in this step, calling receive() inside lambda? No wait.
             // We'll leave this unimplemented properly for now (sketch) or duplicate logic.
             // Duplication is safer for correctness.
             std::unique_lock<std::mutex> lock(mtx);

             // TODO: Implement receive_catching slow path
             // Need separate queue for catching receivers or unified queue with type tag.
             // Since ReceiveChannel interface separates E and ChannelResult<E>,
             // we technically need unified waiters that can handle providing E vs Result.
             // For now, throw not implemented.
             throw std::logic_error("receive_catching slow path not yet implemented");
        }));
    }

    ChannelResult<E> try_receive() override {
        std::lock_guard<std::mutex> lock(mtx);
        
        if (!buffer_.empty()) {
            E el = std::move(buffer_.front());
            buffer_.pop_front();
            
            while (!senders_.empty()) {
                auto& sender = senders_.front();
                if (sender.cont->try_resume()) {
                    buffer_.push_back(std::move(sender.element));
                    senders_.pop_front();
                    break;
                }
                senders_.pop_front();
            }
            return ChannelResult<E>::success(std::move(el));
        }
        
        if (closed_) return ChannelResult<E>::closed(close_cause_);
        
        while (!senders_.empty()) {
            auto& sender = senders_.front();
            if (sender.cont->try_resume()) {
                E el = std::move(sender.element);
                senders_.pop_front();
                return ChannelResult<E>::success(std::move(el));
            }
            senders_.pop_front();
        }

        return ChannelResult<E>::failure();
    }

    ChannelAwaiter<ChannelResult<E>> receive_with_timeout(long timeout) override {
        // TODO
        return ChannelAwaiter<ChannelResult<E>>(ChannelResult<E>::failure());
    }

    std::shared_ptr<ChannelIterator<E>> iterator() override {
        return nullptr; 
    }

    void cancel(std::exception_ptr cause = nullptr) override {
        close(cause ? cause : std::make_exception_ptr(std::runtime_error("Channel cancelled")));
    }

protected:
    struct Sender {
        E element;
        CancellableContinuation<void>* cont;
    };
    
    // We need a unified receiver type or logic. 
    // Standard receivers expect E. receive_catching expects ChannelResult<E>.
    // Using CancellableContinuation<E>* for now as primary queue. 
    // receive_catching is left as Todo in sketch.
    
    int capacity_;
    OnUndeliveredElement<E> on_undelivered_element_;
    bool closed_;
    std::exception_ptr close_cause_;
    
    mutable std::mutex mtx;
    std::deque<E> buffer_;
    std::deque<Sender> senders_;
    std::deque<CancellableContinuation<E>*> receivers_;
    // Missing receivers_catching_ queue for strict correctness
    
    std::vector<std::function<void(std::exception_ptr)>> close_handlers_;
};

} // channels
} // coroutines
} // kotlinx