#pragma once
#include "../core_fwd.hpp"
#include "BufferOverflow.hpp"
#include <memory>
#include <exception>
#include <functional>

namespace kotlinx {
namespace coroutines {
namespace channels {

/**
 * Handler for elements that were sent to a channel but were not delivered to the consumer.
 * This can happen when elements are dropped due to buffer overflow or when operations are cancelled.
 * The handler receives the undelivered element and optionally an exception that caused the failure.
 */
template <typename E>
using OnUndeliveredElement = std::function<void(const E&, std::exception_ptr)>;

/**
 * Indicates an attempt to send to a channel that was closed for sending.
 * This exception is thrown when trying to send to a closed channel without a cause.
 */
class ClosedSendChannelException : public std::runtime_error {
public:
    explicit ClosedSendChannelException(const std::string& message = "Channel was closed")
        : std::runtime_error(message) {}
};

/**
 * Indicates an attempt to receive from a channel that was closed for receiving.
 * This exception is thrown when trying to receive from a closed channel without a cause.
 */
class ClosedReceiveChannelException : public std::runtime_error {
public:
    explicit ClosedReceiveChannelException(const std::string& message = "Channel was closed")
        : std::runtime_error(message) {}
};

/**
 * A discriminated union representing a channel operation result.
 * It encapsulates the knowledge of whether the operation succeeded, failed with an option to retry,
 * or failed because the channel was closed.
 *
 * This matches the Kotlin ChannelResult<T> implementation with proper value semantics.
 */
template <typename T>
class ChannelResult {
private:
    // Internal storage using a union-like approach
    enum class ResultType { SUCCESS, FAILURE, CLOSED };

    ResultType type;
    union {
        T success_value;
        std::exception_ptr closed_cause;
    };

public:
    // Constructor for success
    ChannelResult(T value) : type(ResultType::SUCCESS), success_value(std::move(value)) {}

    // Constructor for failure
    ChannelResult() : type(ResultType::FAILURE) {}

    // Constructor for closed
    ChannelResult(std::exception_ptr cause) : type(ResultType::CLOSED), closed_cause(cause) {}

    // Copy constructor
    ChannelResult(const ChannelResult& other) : type(other.type) {
        switch (type) {
            case ResultType::SUCCESS:
                new (&success_value) T(other.success_value);
                break;
            case ResultType::FAILURE:
                break;
            case ResultType::CLOSED:
                new (&closed_cause) std::exception_ptr(other.closed_cause);
                break;
        }
    }

    // Move constructor
    ChannelResult(ChannelResult&& other) noexcept : type(other.type) {
        switch (type) {
            case ResultType::SUCCESS:
                new (&success_value) T(std::move(other.success_value));
                break;
            case ResultType::FAILURE:
                break;
            case ResultType::CLOSED:
                new (&closed_cause) std::exception_ptr(std::move(other.closed_cause));
                break;
        }
    }

    // Destructor
    ~ChannelResult() {
        switch (type) {
            case ResultType::SUCCESS:
                success_value.~T();
                break;
            case ResultType::CLOSED:
                closed_cause.~exception_ptr();
                break;
            case ResultType::FAILURE:
                break;
        }
    }

    /**
     * Returns `true` if the operation was successful.
     */
    bool is_success() const { return type == ResultType::SUCCESS; }

    /**
     * Returns `true` if the operation failed (either closed or empty/full).
     */
    bool is_failure() const { return type == ResultType::FAILURE || type == ResultType::CLOSED; }

    /**
     * Returns `true` if the operation failed because the channel was closed.
     */
    bool is_closed() const { return type == ResultType::CLOSED; }

    /**
     * Returns the value if successful, or `nullptr` otherwise.
     */
    T* get_or_null() const {
        return is_success() ? const_cast<T*>(&success_value) : nullptr;
    }

    /**
     * Returns the encapsulated value if the operation succeeded, or throws an exception if it failed.
     */
    T get_or_throw() const {
        if (is_success()) {
            return success_value;
        }
        if (is_closed()) {
            if (closed_cause) {
                std::rethrow_exception(closed_cause);
            }
            throw std::runtime_error("Trying to call 'getOrThrow' on a channel closed without a cause");
        }
        throw std::runtime_error("Trying to call 'getOrThrow' on a failed result of a non-closed channel");
    }

    /**
     * Returns the exception with which the channel was closed, or `nullptr` if the channel was not closed or was closed without a cause.
     */
    std::exception_ptr exception_or_null() const {
        return is_closed() ? closed_cause : nullptr;
    }

    // Factory methods
    static ChannelResult<T> success(T value) { return ChannelResult<T>(std::move(value)); }
    static ChannelResult<T> failure() { return ChannelResult<T>(); }
    static ChannelResult<T> closed(std::exception_ptr cause = nullptr) { return ChannelResult<T>(cause); }
};

// Void specialization
template <>
class ChannelResult<void> {
private:
    enum class ResultType { SUCCESS, FAILURE, CLOSED };
    ResultType type;
    std::exception_ptr closed_cause;

public:
    ChannelResult() : type(ResultType::SUCCESS) {}
    ChannelResult(ResultType t) : type(t) {}
    ChannelResult(std::exception_ptr cause) : type(ResultType::CLOSED), closed_cause(cause) {}

    bool is_success() const { return type == ResultType::SUCCESS; }
    bool is_failure() const { return type == ResultType::FAILURE || type == ResultType::CLOSED; }
    bool is_closed() const { return type == ResultType::CLOSED; }
    std::exception_ptr exception_or_null() const { return is_closed() ? closed_cause : nullptr; }

    static ChannelResult<void> success() { return ChannelResult<void>(ResultType::SUCCESS); }
    static ChannelResult<void> failure() { return ChannelResult<void>(ResultType::FAILURE); }
    static ChannelResult<void> closed(std::exception_ptr cause = nullptr) { return ChannelResult<void>(cause); }
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

    /**
     * Name of the JVM system property for the default channel capacity (64 by default).
     */
    static constexpr const char* DEFAULT_BUFFER_PROPERTY_NAME = "kotlinx.coroutines.channels.defaultBuffer";

    /**
     * Gets the default buffer capacity, either from system property or default value.
     */
    static int getDefaultBufferCapacity() {
        // TODO: Implement system property lookup
        // For now, return the default value
        return 64;
    }
};

/**
 * Creates a channel with the specified capacity, buffer overflow strategy, and onUndeliveredElement handler.
 */
template <typename E>
std::shared_ptr<Channel<E>> createChannel(
    int capacity = Channel<E>::RENDEZVOUS,
    BufferOverflow onBufferOverflow = BufferOverflow::SUSPEND,
    OnUndeliveredElement<E> onUndeliveredElement = nullptr
);

} // namespace channels
} // namespace coroutines
} // namespace kotlinx
