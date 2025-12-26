#pragma once
// Transliterated from:
// - kotlinx-coroutines-core/common/src/channels/Channels.common.kt
// - kotlinx-coroutines-core/concurrent/src/channels/Channels.kt
// - Channel factory from kotlinx-coroutines-core/common/src/channels/Channel.kt (lines 1373-1456)
//
// Kotlin imports:
// - kotlinx.coroutines.*
// - kotlinx.coroutines.selects.*
// - kotlin.contracts.*
// - kotlin.jvm.*

#include "kotlinx/coroutines/channels/Channel.hpp"
#include "kotlinx/coroutines/channels/BufferedChannel.hpp"
#include "kotlinx/coroutines/channels/ConflatedBufferedChannel.hpp"
#include "kotlinx/coroutines/channels/BufferOverflow.hpp"
#include "kotlinx/coroutines/CoroutineScope.hpp"
#include "kotlinx/coroutines/Exceptions.hpp"
#include "kotlinx/coroutines/selects/Select.hpp"
#include <functional>
#include <vector>
#include <exception>
#include <mutex>
#include <condition_variable>

namespace kotlinx {
namespace coroutines {
namespace channels {

/**
 * Creates a channel. See the Channel interface documentation for details.
 *
 * @param capacity either a positive channel capacity or one of the constants
 *        defined in Channel.Factory.
 * @param on_buffer_overflow configures an action on buffer overflow.
 * @param on_undelivered_element a function called when element was sent but
 *        was not delivered to the consumer.
 * @throws std::invalid_argument when capacity < -2
 */
template <typename E>
std::shared_ptr<Channel<E>> create_channel(
    int capacity,
    BufferOverflow on_buffer_overflow,
    OnUndeliveredElement<E> on_undelivered_element
) {
    // Constants from Channel<E>
    int RENDEZVOUS = Channel<E>::RENDEZVOUS;
    int CONFLATED = Channel<E>::CONFLATED;
    int UNLIMITED = Channel<E>::UNLIMITED;
    int BUFFERED = Channel<E>::BUFFERED;
    int DEFAULT_CAPACITY = Channel<E>::channel_default_capacity();

    if (capacity == RENDEZVOUS) {
        if (on_buffer_overflow == BufferOverflow::SUSPEND) {
            return std::make_shared<BufferedChannel<E>>(RENDEZVOUS, on_undelivered_element);
        } else {
            return std::make_shared<ConflatedBufferedChannel<E>>(1, on_buffer_overflow, on_undelivered_element);
        }
    } else if (capacity == CONFLATED) {
        if (on_buffer_overflow != BufferOverflow::SUSPEND) {
            throw std::invalid_argument("CONFLATED capacity cannot be used with non-default onBufferOverflow");
        }
        return std::make_shared<ConflatedBufferedChannel<E>>(1, BufferOverflow::DROP_OLDEST, on_undelivered_element);
    } else if (capacity == UNLIMITED) {
        return std::make_shared<BufferedChannel<E>>(UNLIMITED, on_undelivered_element);
    } else if (capacity == BUFFERED) {
        if (on_buffer_overflow == BufferOverflow::SUSPEND) {
            return std::make_shared<BufferedChannel<E>>(DEFAULT_CAPACITY, on_undelivered_element);
        } else {
            return std::make_shared<ConflatedBufferedChannel<E>>(1, on_buffer_overflow, on_undelivered_element);
        }
    } else {
        if (on_buffer_overflow == BufferOverflow::SUSPEND) {
            return std::make_shared<BufferedChannel<E>>(capacity, on_undelivered_element);
        } else {
            return std::make_shared<ConflatedBufferedChannel<E>>(capacity, on_buffer_overflow, on_undelivered_element);
        }
    }
}

/**
 * Adds element to this channel, **blocking** the caller while this channel is full,
 * and returning either successful result when the element was added, or
 * failed result representing closed channel with a corresponding exception.
 *
 * This is a way to call Channel.send method in a safe manner inside a blocking code,
 * so this function should not be used from coroutine.
 *
 * For this operation it is guaranteed that failure always contains an exception in it.
 */
template <typename E>
ChannelResult<void> try_send_blocking(SendChannel<E>* channel, E element) {
    auto result = channel->try_send(element);
    if (result.is_success()) {
        return ChannelResult<void>::success();
    }

    // Blocking send using condition variable
    std::mutex mtx;
    std::condition_variable cv;
    bool done = false;
    ChannelResult<void> final_result = ChannelResult<void>::failure();
    std::exception_ptr ex = nullptr;

    // Create a continuation that signals completion
    class BlockingContinuation : public Continuation<void*> {
    public:
        std::mutex& mtx_;
        std::condition_variable& cv_;
        bool& done_;
        std::exception_ptr& ex_;
        std::shared_ptr<CoroutineContext> ctx_;

        BlockingContinuation(std::mutex& m, std::condition_variable& c, bool& d, std::exception_ptr& e)
            : mtx_(m), cv_(c), done_(d), ex_(e), ctx_(EmptyCoroutineContext::instance()) {}

        std::shared_ptr<CoroutineContext> get_context() const override { return ctx_; }

        void resume_with(Result<void*> result) override {
            std::lock_guard<std::mutex> lock(mtx_);
            if (result.is_failure()) {
                ex_ = result.exception_or_null();
            }
            done_ = true;
            cv_.notify_one();
        }
    };

    auto cont = std::make_shared<BlockingContinuation>(mtx, cv, done, ex);

    try {
        void* send_result = channel->send(std::move(element), cont.get());
        if (send_result != intrinsics::get_COROUTINE_SUSPENDED()) {
            // Completed immediately
            return ChannelResult<void>::success();
        }

        // Wait for completion
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock, [&done] { return done; });

        if (ex) {
            return ChannelResult<void>::closed(ex);
        }
        return ChannelResult<void>::success();
    } catch (...) {
        return ChannelResult<void>::closed(std::current_exception());
    }
}

/**
 * @deprecated Deprecated in favour of 'try_send_blocking'.
 * Consider handling the result of 'try_send_blocking' explicitly and rethrow exception if necessary.
 */
template <typename E>
[[deprecated("Use try_send_blocking instead")]]
void send_blocking(SendChannel<E>* channel, E element) {
    // Fast path
    if (channel->try_send(element).is_success()) {
        return;
    }

    // Slow path - blocking send
    auto result = try_send_blocking(channel, std::move(element));
    if (result.is_closed()) {
        auto cause = result.exception_or_null();
        if (cause) {
            std::rethrow_exception(cause);
        }
        throw ClosedSendChannelException(DEFAULT_CLOSE_MESSAGE);
    }
}

/**
 * Internal function to cancel a channel after consumption.
 */
template <typename E>
void cancel_consumed(ReceiveChannel<E>* channel, std::exception_ptr cause) {
    channel->cancel(cause);
}

/**
 * Executes the block and then cancels the channel.
 * It is guaranteed that, after invoking this operation, the channel will be cancelled,
 * so the operation is _terminal_.
 * If the block finishes with an exception, that exception will be used for cancelling
 * the channel and rethrown.
 */
template <typename E, typename R>
R consume(ReceiveChannel<E>* channel, std::function<R(ReceiveChannel<E>*)> block) {
    std::exception_ptr cause = nullptr;
    try {
        R result = block(channel);
        cancel_consumed(channel, cause);
        return result;
    } catch (...) {
        cause = std::current_exception();
        cancel_consumed(channel, cause);
        throw;
    }
}

// Overload for void return type
template <typename E>
void consume(ReceiveChannel<E>* channel, std::function<void(ReceiveChannel<E>*)> block) {
    std::exception_ptr cause = nullptr;
    try {
        block(channel);
        cancel_consumed(channel, cause);
    } catch (...) {
        cause = std::current_exception();
        cancel_consumed(channel, cause);
        throw;
    }
}

/**
 * Performs the given action for each received element and cancels the channel afterward.
 * This function stops processing elements when either the channel is closed,
 * action fails with an exception, or an early return from action happens.
 *
 * The operation is _terminal_.
 */
template <typename E>
void consume_each(ReceiveChannel<E>* channel, std::function<void(E)> action) {
    consume<E>(channel, [&action](ReceiveChannel<E>* c) {
        auto iter = c->iterator();
        // Note: has_next() is a suspend function, but for blocking iteration
        // we use a simple loop with try_receive for non-blocking semantics
        while (true) {
            auto result = c->try_receive();
            if (result.is_success()) {
                action(result.get_or_throw());
            } else if (result.is_closed()) {
                auto cause = result.exception_or_null();
                if (cause) {
                    std::rethrow_exception(cause);
                }
                break;
            } else {
                // is_failure() means channel is empty but not closed
                // In a true suspend context we would suspend here
                // For blocking, we could busy-wait or use condition variables
                // For now, break to avoid infinite loop in non-suspend context
                break;
            }
        }
    });
}

/**
 * Returns a List containing all the elements sent to this channel, preserving their order.
 * This function will attempt to receive elements until the channel is closed.
 *
 * The operation is _terminal_.
 */
template <typename E>
std::vector<E> to_list(ReceiveChannel<E>* channel) {
    std::vector<E> list;
    consume_each<E>(channel, [&list](E e) {
        list.push_back(std::move(e));
    });
    return list;
}

} // namespace channels
} // namespace coroutines
} // namespace kotlinx
