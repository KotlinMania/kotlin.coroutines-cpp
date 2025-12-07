#pragma once
#include "kotlinx/coroutines/core_fwd.hpp"
#include <memory>
#include <exception>
#include <functional>

namespace kotlinx {
namespace coroutines {
namespace channels {

/**
 * A discriminated union representing a channel operation result.
 * It encapsulates the knowledge of whether the operation succeeded, failed with an option to retry,
 * or failed because the channel was closed.
 */
template <typename T>
struct ChannelResult {
    T* value; // nullable, nullptr if failed
    bool failed;
    bool closed;
    std::exception_ptr exception;

    /**
     * Returns `true` if the operation was successful.
     */
    bool is_success() const { return !failed && !closed; }

    /**
     * Returns `true` if the operation failed (either closed or empty/full).
     */
    bool is_failure() const { return failed; }

    /**
     * Returns `true` if the operation failed because the channel was closed.
     */
    bool is_closed() const { return closed; }
    
    /**
     * Returns the value if successful, or `nullptr` otherwise.
     */
    T* get_or_null() const { return value; }

    /**
     * Returns the exception if the channel was closed with a cause, or `nullptr` otherwise.
     */
    std::exception_ptr exception_or_null() const { return exception; }

    // Helpers
    static ChannelResult<T> success(T* v) { return {v, false, false, nullptr}; }
    static ChannelResult<T> failure() { return {nullptr, true, false, nullptr}; }
    static ChannelResult<T> closed_result(std::exception_ptr e) { return {nullptr, true, true, e}; }
};

template <typename E>
struct ChannelIterator {
    /**
     * Returns `true` if the iterator has a next element, suspending if necessary.
     */
    virtual bool has_next() = 0; // suspend

    /**
     * Returns the next element from the channel.
     */
    virtual E next() = 0;

    virtual ~ChannelIterator() = default;
};

/**
 * Sender's interface to a [Channel].
 */
template <typename E>
struct SendChannel {
    virtual ~SendChannel() = default;

    /**
     * Returns `true` if this channel was closed by an invocation of [close] or its receiving side was [cancelled].
     */
    virtual bool is_closed_for_send() const = 0;

    /**
     * Sends the specified [element] to this channel.
     * This function suspends if the channel is full or does not have a receiver.
     */
    virtual void send(E element) = 0;

    /**
     * Attempts to add the specified [element] to this channel without waiting.
     * Returns a [ChannelResult] indicating success or failure.
     */
    virtual ChannelResult<void> try_send(E element) = 0;

    /**
     * Closes this channel so that subsequent attempts to [send] to it fail.
     * Returns `true` if the channel was closed by this invocation, `false` otherwise.
     */
    virtual bool close(std::exception_ptr cause = nullptr) = 0;

    /**
     * Registers a [handler] that is synchronously invoked once the channel is [closed].
     */
    virtual void invoke_on_close(std::function<void(std::exception_ptr)> handler) = 0;
};

/**
 * Receiver's interface to a [Channel].
 */
template <typename E>
struct ReceiveChannel {
    virtual ~ReceiveChannel() = default;

    /**
     * Returns `true` if the sending side of this channel was [closed] and all items were received.
     */
    virtual bool is_closed_for_receive() const = 0;

    /**
     * Returns `true` if the channel contains no elements and isn't closed for receive.
     */
    virtual bool is_empty() const = 0;

    /**
     * Retrieves an element, removing it from the channel.
     * Suspends if specific element is not available.
     */
    virtual E receive() = 0;
    
    /**
     * Retrieves an element, removing it from the channel.
     * Returns a [ChannelResult] on failure or close instead of throwing.
     */
    virtual ChannelResult<E> receive_catching() = 0;

    /**
     * Attempts to retrieve an element without waiting.
     */
    virtual ChannelResult<E> try_receive() = 0;

    /**
     * Returns a new iterator to receive elements from this channel.
     */
    virtual std::shared_ptr<ChannelIterator<E>> iterator() = 0;

    /**
     * Cancels the channel (clears buffer and closes it).
     */
    virtual void cancel(std::exception_ptr cause = nullptr) = 0;
};

/**
 * Channel is a communication primitive (conceptually similar to BlockingQueue).
 * It implements both [SendChannel] and [ReceiveChannel].
 */
template <typename E>
struct Channel : public SendChannel<E>, public ReceiveChannel<E> {
    // Factory constants
    static constexpr int UNLIMITED = 2147483647;
    static constexpr int RENDEZVOUS = 0;
    static constexpr int CONFLATED = -1;
    static constexpr int BUFFERED = -2;
    static constexpr int OPTIONAL_CHANNEL = -3;
};

} // namespace channels
} // namespace coroutines
} // namespace kotlinx
