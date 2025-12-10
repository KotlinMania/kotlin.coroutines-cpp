#pragma once
#include "BufferOverflow.hpp"
#include <memory>
#include <exception>
#include <functional>
#include <string>

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

    ChannelResult(const ChannelResult& other) : type(other.type) {
        switch (type) {
            case ResultType::SUCCESS: new (&success_value) T(other.success_value); break;
            case ResultType::FAILURE: break;
            case ResultType::CLOSED: new (&closed_cause) std::exception_ptr(other.closed_cause); break;
        }
    }

    ChannelResult(ChannelResult&& other) noexcept : type(other.type) {
        switch (type) {
            case ResultType::SUCCESS: new (&success_value) T(std::move(other.success_value)); break;
            case ResultType::FAILURE: break;
            case ResultType::CLOSED: new (&closed_cause) std::exception_ptr(std::move(other.closed_cause)); break;
        }
    }

    ~ChannelResult() {
        switch (type) {
            case ResultType::SUCCESS: success_value.~T(); break;
            case ResultType::CLOSED: closed_cause.~exception_ptr(); break;
            case ResultType::FAILURE: break;
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

    T* get_or_null() const { return is_success() ? const_cast<T*>(&success_value) : nullptr; }

    T get_or_throw() const {
        if (is_success()) return success_value;
        if (is_closed()) {
            if (closed_cause) std::rethrow_exception(closed_cause);
            throw std::runtime_error("Channel closed without cause");
        }
        throw std::runtime_error("Channel operation failed");
    }

    /**
 * Returns the exception with which the channel was closed, or `nullptr` if the channel was not closed or was closed without a cause.
 */
    std::exception_ptr exception_or_null() const { return is_closed() ? closed_cause : nullptr; }

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
    virtual bool has_next() = 0; // suspend
    virtual E next() = 0;
    virtual ~ChannelIterator() = default;
};

template <typename E>
struct SendChannel {
    virtual ~SendChannel() = default;
    virtual bool is_closed_for_send() const = 0;
    virtual void send(E element) = 0; // suspend
    virtual ChannelResult<void> try_send(E element) = 0;
    virtual bool close(std::exception_ptr cause = nullptr) = 0;
    virtual void invoke_on_close(std::function<void(std::exception_ptr)> handler) = 0;
};

template <typename E>
struct ReceiveChannel {
    virtual ~ReceiveChannel() = default;
    virtual bool is_closed_for_receive() const = 0;
    virtual bool is_empty() const = 0;
    virtual E receive() = 0; // suspend
    virtual ChannelResult<E> receive_catching() = 0; // suspend
    virtual ChannelResult<E> try_receive() = 0;
    virtual std::shared_ptr<ChannelIterator<E>> iterator() = 0;
    virtual void cancel(std::exception_ptr cause = nullptr) = 0;
};

template <typename E>
struct Channel : public SendChannel<E>, public ReceiveChannel<E> {
    static constexpr int UNLIMITED = 2147483647;
    static constexpr int RENDEZVOUS = 0;
    static constexpr int CONFLATED = -1;
    static constexpr int BUFFERED = -2;
    static constexpr int OPTIONAL_CHANNEL = -3;
    static constexpr const char* DEFAULT_BUFFER_PROPERTY_NAME = "kotlinx.coroutines.channels.defaultBuffer";

    static int getDefaultBufferCapacity() { return 64; }
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
