#pragma once
// port-lint: source kotlinx-coroutines-core/common/src/channels/Channels.common.kt
// Transliterated from: kotlinx-coroutines-core/common/src/channels/Channels.common.kt
// Transliterated from:
// - kotlinx-coroutines-core/common/src/channels/Channels.common.kt
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
#include "kotlinx/coroutines/ContinuationImpl.hpp"
#include "kotlinx/coroutines/dsl/Suspend.hpp"

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
namespace internal {
// Transliterated from: kotlinx-coroutines-core/common/src/channels/Channels.common.kt:198-202
std::exception_ptr consumed_cancellation_cause(std::exception_ptr cause);
}

// Transliterated from: kotlinx-coroutines-core/common/src/channels/Channels.common.kt:198-202
template <typename E>
void cancel_consumed(ReceiveChannel<E>* channel, std::exception_ptr cause) {
    channel->cancel(internal::consumed_cancellation_cause(cause));
}

/**
 * Executes the block and then cancels the channel.
 *
 * It is guaranteed that, after invoking this operation, the channel will be cancelled,
 * so the operation is _terminal_.
 * If the block finishes with an exception, that exception will be used for cancelling
 * the channel and rethrown.
 *
 * This function is useful for building more complex terminal operators while ensuring
 * that producers stop sending new elements to the channel.
 *
 * Example:
 * ```kotlin
 * suspend fun <E> ReceiveChannel<E>.consumeFirst(): E =
 *     consume { return receive() }
 * ```
 *
 * consume() does not guarantee that new elements will not enter the channel after
 * block finishes executing, so some channel elements may be lost. Use the
 * on_undelivered_element parameter of a manually created Channel to define what
 * should happen with these elements during cancel().
 */
// Transliterated from: kotlinx-coroutines-core/common/src/channels/Channels.common.kt:90-103
template <typename E, typename R>
R consume(ReceiveChannel<E>* channel, std::function<R(ReceiveChannel<E>*)> block) {
    // NOTE(port): Keep finally outside the block's catch so a cancellation
    // failure supersedes the block result without invoking cancellation twice.
    R result = [&]() -> R {
        try {
            return block(channel);
        } catch (...) {
            cancel_consumed(channel, std::current_exception());
            throw;
        }
    }();
    cancel_consumed(channel, nullptr);
    return result;
}

// Transliterated from: kotlinx-coroutines-core/common/src/channels/Channels.common.kt:90-103
template <typename E>
void consume(ReceiveChannel<E>* channel, std::function<void(ReceiveChannel<E>*)> block) {
    try {
        block(channel);
    } catch (...) {
        cancel_consumed(channel, std::current_exception());
        throw;
    }
    cancel_consumed(channel, nullptr);
}

/**
 * Performs the given action for each received element and cancels the channel afterward.
 *
 * This function stops processing elements when either the channel is closed,
 * the coroutine in which the collection is performed gets cancelled and there are
 * no readily available elements in the channel's buffer, action fails with an
 * exception, or an early return from action happens.
 *
 * If the action finishes with an exception, that exception will be used for cancelling
 * the channel and rethrown. If the channel is closed with a cause, this cause will be
 * rethrown from consume_each().
 *
 * When the channel does not need to be closed after iterating over its elements,
 * a regular for loop should be used instead.
 *
 * The operation is _terminal_.
 * This function consumes all elements of the original ReceiveChannel.
 *
 * Pitfall: even though the name says "each", some elements could be left unprocessed
 * if they are added after this function decided to close the channel. Use the
 * on_undelivered_element parameter of the Channel constructor to handle these.
 */
// Transliterated from: kotlinx-coroutines-core/common/src/channels/Channels.common.kt:159-162
template <typename E>
void* consume_each(ReceiveChannel<E>* channel, std::function<void*(E, Continuation<void*>*)> action,
                   Continuation<void*>* completion) {
    // NOTE(port): The public element type requires a header-instantiated frame.
    // Use the existing Continuation ABI and LLVM-injected suspension markers.
    // The raw channel stays borrowed; callers retain it throughout suspension.
    // Transliterated from: kotlinx-coroutines-core/common/src/channels/Channels.common.kt:90-103,159-162
    class ConsumeEachFrame final : public ContinuationImpl {
    public:
        // Transliterated from: kotlinx-coroutines-core/common/src/channels/Channels.common.kt:159-162
        ConsumeEachFrame(ReceiveChannel<E>* channel, std::function<void*(E, Continuation<void*>*)> action,
                         Continuation<void*>* completion)
            : ContinuationImpl(kotlinx::coroutines::internal::retain_continuation(completion)),
              channel_(channel), action_(std::move(action)) {}

        void retain() { self_ref_ = shared_from_this(); }

        void release_intercepted() override {
            ContinuationImpl::release_intercepted();
            iterator_.reset();
            element_.reset();
            action_ = nullptr;
            self_ref_.reset();
        }

        // Transliterated from: kotlinx-coroutines-core/common/src/channels/Channels.common.kt:90-103,159-162
        void* invoke_suspend(Result<void*> result) override {
            try {
                coroutine_begin(this)
                iterator_ = channel_->iterator();
                while (true) {
                    coroutine_yield_value(this, result, iterator_->has_next(this), has_next_box_);
                    // NOTE(port): has_next's bool box belongs to this receiving call.
                    has_next_ = *static_cast<bool*>(has_next_box_);
                    delete static_cast<bool*>(has_next_box_);
                    has_next_box_ = nullptr;
                    if (!has_next_) break;
                    element_.emplace(iterator_->next());
                    // NOTE(port): Kotlin's inline action can contain suspension.
                    coroutine_yield(this, action_(std::move(*element_), this));
                    element_.reset();
                }
            } catch (...) {
                cancel_consumed(channel_, std::current_exception());
                throw;
            }
            cancel_consumed(channel_, nullptr);
            coroutine_end(this)
        }

    private:
        void* _label = nullptr;
        ReceiveChannel<E>* channel_;
        std::function<void*(E, Continuation<void*>*)> action_;
        std::optional<E> element_;
        std::unique_ptr<ChannelIterator<E>> iterator_;
        void* has_next_box_ = nullptr;
        bool has_next_ = false;
        std::shared_ptr<BaseContinuationImpl> self_ref_;
    };
    auto frame = std::make_shared<ConsumeEachFrame>(channel, std::move(action), completion);
    frame->retain();
    return frame->start(Result<void*>::success(nullptr));
}

// Transliterated from: kotlinx-coroutines-core/common/src/channels/Channels.common.kt:159-162
// NOTE(port): Ordinary nonsuspending C++ actions retain their callable interface.
template <typename E>
void* consume_each(ReceiveChannel<E>* channel, std::function<void(E)> action,
                   Continuation<void*>* completion) {
    std::function<void*(E, Continuation<void*>*)> lowered_action =
        [action = std::move(action)](E value, Continuation<void*>*) -> void* {
            action(std::move(value));
            return nullptr;
        };
    return consume_each<E>(channel, std::move(lowered_action), completion);
}

/**
 * Returns a vector containing all the elements sent to this channel, preserving their order.
 *
 * This function will attempt to receive elements and put them into the vector until
 * the channel is closed. Calling to_list() on channels that are not eventually closed
 * is always incorrect:
 * - It will suspend indefinitely if the channel is not closed, but no new elements arrive.
 * - If new elements do arrive and the channel is not eventually closed, to_list() will
 *   use more and more memory until exhausting it.
 *
 * If the channel is closed with a cause, to_list() will rethrow that cause.
 *
 * The operation is _terminal_.
 * This function consumes all elements of the original ReceiveChannel.
 */
// Transliterated from: kotlinx-coroutines-core/common/src/channels/Channels.common.kt:191-195
template <typename E>
void* to_list(ReceiveChannel<E>* channel, Continuation<void*>* completion) {
    // Transliterated from: kotlinx-coroutines-core/common/src/channels/Channels.common.kt:191-195
    class ToListFrame final : public ContinuationImpl {
    public:
        // Transliterated from: kotlinx-coroutines-core/common/src/channels/Channels.common.kt:191-195
        ToListFrame(ReceiveChannel<E>* channel, Continuation<void*>* completion)
            : ContinuationImpl(kotlinx::coroutines::internal::retain_continuation(completion)),
              channel_(channel) {}

        void retain() { self_ref_ = shared_from_this(); }

        void release_intercepted() override {
            ContinuationImpl::release_intercepted();
            values_.clear();
            self_ref_.reset();
        }

        // Transliterated from: kotlinx-coroutines-core/common/src/channels/Channels.common.kt:191-195
        void* invoke_suspend(Result<void*> result) override {
            coroutine_begin(this)
            coroutine_yield(this, consume_each<E>(channel_,
                [this](E value) { values_.push_back(std::move(value)); }, this));
            // NOTE(port): The caller owns the returned vector box and must delete it
            // after unboxing, on both immediate and resumed result paths.
            return new std::vector<E>(std::move(values_));
        }

    private:
        void* _label = nullptr;
        ReceiveChannel<E>* channel_;
        std::vector<E> values_;
        std::shared_ptr<BaseContinuationImpl> self_ref_;
    };
    auto frame = std::make_shared<ToListFrame>(channel, completion);
    frame->retain();
    return frame->start(Result<void*>::success(nullptr));
}

} // namespace channels
} // namespace coroutines
} // namespace kotlinx
