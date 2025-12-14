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

// ============================================================================
// Line 12: internal const val DEFAULT_CLOSE_MESSAGE = "Channel was closed"
// Already defined in Channel.hpp
// ============================================================================

// ============================================================================
// Channel factory function (from Channel.kt lines 1373-1456)
// Implementation here because it requires concrete BufferedChannel types.
// ============================================================================

/**
 * Line 1373-1456 of Channel.kt: fun <E> Channel(...)
 *
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

    // Line 1430-1452: when (capacity) { ... }
    if (capacity == RENDEZVOUS) {
        // Line 1431-1436
        if (on_buffer_overflow == BufferOverflow::SUSPEND) {
            return std::make_shared<BufferedChannel<E>>(RENDEZVOUS, on_undelivered_element);
        } else {
            return std::make_shared<ConflatedBufferedChannel<E>>(1, on_buffer_overflow, on_undelivered_element);
        }
    } else if (capacity == CONFLATED) {
        // Line 1437-1441
        if (on_buffer_overflow != BufferOverflow::SUSPEND) {
            throw std::invalid_argument("CONFLATED capacity cannot be used with non-default onBufferOverflow");
        }
        return std::make_shared<ConflatedBufferedChannel<E>>(1, BufferOverflow::DROP_OLDEST, on_undelivered_element);
    } else if (capacity == UNLIMITED) {
        // Line 1443
        return std::make_shared<BufferedChannel<E>>(UNLIMITED, on_undelivered_element);
    } else if (capacity == BUFFERED) {
        // Line 1444-1447
        if (on_buffer_overflow == BufferOverflow::SUSPEND) {
            return std::make_shared<BufferedChannel<E>>(DEFAULT_CAPACITY, on_undelivered_element);
        } else {
            return std::make_shared<ConflatedBufferedChannel<E>>(1, on_buffer_overflow, on_undelivered_element);
        }
    } else {
        // Line 1448-1451
        if (on_buffer_overflow == BufferOverflow::SUSPEND) {
            return std::make_shared<BufferedChannel<E>>(capacity, on_undelivered_element);
        } else {
            return std::make_shared<ConflatedBufferedChannel<E>>(capacity, on_buffer_overflow, on_undelivered_element);
        }
    }
}

// ============================================================================
// Channels.kt (concurrent/src) - Blocking operations
// ============================================================================

/**
 * Line 9-43 of Channels.kt: fun <E> SendChannel<E>.trySendBlocking(element: E): ChannelResult<Unit>
 *
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
    // Line 37: trySend(element).onSuccess { return ChannelResult.success(Unit) }
    auto result = channel->try_send(element);
    if (result.is_success()) {
        return ChannelResult<void>::success();
    }

    // Line 38-42: return runBlocking { ... }
    // In C++, we implement blocking send using a condition variable
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
 * Line 45-60 of Channels.kt: fun <E> SendChannel<E>.sendBlocking(element: E)
 *
 * @deprecated Deprecated in favour of 'try_send_blocking'.
 * Consider handling the result of 'try_send_blocking' explicitly and rethrow exception if necessary.
 */
template <typename E>
[[deprecated("Use try_send_blocking instead")]]
void send_blocking(SendChannel<E>* channel, E element) {
    // Line 53-55: fast path
    if (channel->try_send(element).is_success()) {
        return;
    }

    // Line 56-59: slow path - blocking send
    auto result = try_send_blocking(channel, std::move(element));
    if (result.is_closed()) {
        auto cause = result.exception_or_null();
        if (cause) {
            std::rethrow_exception(cause);
        }
        throw ClosedSendChannelException(DEFAULT_CLOSE_MESSAGE);
    }
}

// ============================================================================
// Channels.common.kt - Channel consumption utilities
// ============================================================================

/**
 * Line 197-202: internal fun ReceiveChannel<*>.cancelConsumed(cause: Throwable?)
 */
template <typename E>
void cancel_consumed(ReceiveChannel<E>* channel, std::exception_ptr cause) {
    // Line 199-201: cancel with cause wrapped in CancellationException if needed
    // In C++ we just pass the cause directly for now
    channel->cancel(cause);
}

/**
 * Line 90-103: inline fun <E, R> ReceiveChannel<E>.consume(block: ReceiveChannel<E>.() -> R): R
 *
 * Executes the block and then cancels the channel.
 * It is guaranteed that, after invoking this operation, the channel will be cancelled,
 * so the operation is _terminal_.
 * If the block finishes with an exception, that exception will be used for cancelling
 * the channel and rethrown.
 */
template <typename E, typename R>
R consume(ReceiveChannel<E>* channel, std::function<R(ReceiveChannel<E>*)> block) {
    // Line 94: var cause: Throwable? = null
    std::exception_ptr cause = nullptr;
    try {
        // Line 96: return block()
        R result = block(channel);
        // Line 101: cancelConsumed(cause)
        cancel_consumed(channel, cause);
        return result;
    } catch (...) {
        // Line 97-99: catch (e: Throwable) { cause = e; throw e }
        cause = std::current_exception();
        // Line 101: cancelConsumed(cause) in finally
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
 * Line 159-162: suspend inline fun <E> ReceiveChannel<E>.consumeEach(action: (E) -> Unit): Unit
 *
 * Performs the given action for each received element and cancels the channel afterward.
 * This function stops processing elements when either the channel is closed,
 * action fails with an exception, or an early return from action happens.
 *
 * The operation is _terminal_.
 */
template <typename E>
void consume_each(ReceiveChannel<E>* channel, std::function<void(E)> action) {
    // Line 160-162: consume { for (e in this) action(e) }
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
 * Line 191-195: suspend fun <E> ReceiveChannel<E>.toList(): List<E>
 *
 * Returns a List containing all the elements sent to this channel, preserving their order.
 * This function will attempt to receive elements until the channel is closed.
 *
 * The operation is _terminal_.
 */
template <typename E>
std::vector<E> to_list(ReceiveChannel<E>* channel) {
    // Line 192-195: buildList { consumeEach { add(it) } }
    std::vector<E> list;
    consume_each<E>(channel, [&list](E e) {
        list.push_back(std::move(e));
    });
    return list;
}

// ============================================================================
// Deprecated functions (HIDDEN level - not transliterated)
// ============================================================================

// Line 28-37: receiveOrNull - deprecated, not transliterated (HIDDEN)
// Line 42-49: onReceiveOrNull - deprecated, not transliterated (HIDDEN)

} // namespace channels
} // namespace coroutines
} // namespace kotlinx
