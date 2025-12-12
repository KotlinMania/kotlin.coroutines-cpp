#pragma once
// Kotlin source: kotlinx-coroutines-core/common/src/channels/Channel.kt
#include "kotlinx/coroutines/CancellableContinuation.hpp"
#include "kotlinx/coroutines/selects/Select.hpp"
#include "kotlinx/coroutines/channels/BufferOverflow.hpp"
#include "kotlinx/coroutines/Exceptions.hpp"
#include <memory>
#include <exception>
#include <functional>
#include <string>
#include <optional>

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
 * 
 * This exception is thrown when attempting to send an element to a channel
 * that has been closed. The channel may have been closed normally or due
 * to an error condition.
 */
class ClosedSendChannelException : public std::runtime_error {
public:
    /**
     * Constructs a ClosedSendChannelException with the specified message.
     * @param message The error message describing the closure.
     */
    explicit ClosedSendChannelException(const std::string& message = "Channel was closed");
};

/**
 * Indicates an attempt to receive from a channel that was closed for receiving.
 */
class ClosedReceiveChannelException : public std::runtime_error {
public:
    explicit ClosedReceiveChannelException(const std::string& message = "Channel was closed");
};

/**
 * A discriminated union representing a channel operation result.
 * 
 * ChannelResult provides a type-safe way to handle the three possible outcomes
 * of channel operations: success with a value, failure (no immediate result),
 * or closed channel. This is particularly useful for non-blocking operations
 * like try_send() and try_receive().
 * 
 * @tparam T The type of value contained in a successful result.
 */
template <typename T>
class ChannelResult {
private:
    enum class ResultType { SUCCESS, FAILURE, CLOSED };

    ResultType type;
    union {
        T success_value;
        std::exception_ptr closed_cause;
    };

public:
    ChannelResult(T value) : type(ResultType::SUCCESS), success_value(std::move(value)) {}
    ChannelResult() : type(ResultType::FAILURE) {}
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

    bool is_success() const { return type == ResultType::SUCCESS; }
    bool is_failure() const { return type == ResultType::FAILURE || type == ResultType::CLOSED; }
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

    std::exception_ptr exception_or_null() const { return is_closed() ? closed_cause : nullptr; }

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

// --- Awaiter Infrastructure ---
// TODO: Rewrite to use Kotlin-style coroutines (Continuation<void*>*) instead of C++20 coroutines.
// The suspend functions should follow the pattern:
//   void* send(E element, Continuation<void*>* continuation);
//   void* receive(Continuation<void*>* continuation);
// For now, using placeholder that compiles but doesn't support actual suspension.

template <typename T>
struct ChannelAwaiter {
    std::unique_ptr<T> fast_val;
    bool suspended_ = false;

    // Fast path constructor - immediate value
    ChannelAwaiter(T val) : fast_val(std::make_unique<T>(std::move(val))), suspended_(false) {}

    // Slow path constructor - needs suspension (placeholder)
    // TODO: NEEDS SUSPEND instead of suspendvariable - refer to Channel.kt
    ChannelAwaiter(bool needs_suspend /*, std::function<void(CancellableContinuation<T>&)> block*/)
       : suspended_(needs_suspend) {}

    bool has_value() const { return fast_val != nullptr; }
    bool needs_suspend() const { return suspended_; }

    T get_or_throw() const {
        if (fast_val) return *fast_val;
        throw std::runtime_error("ChannelAwaiter: no value available - suspension required");
    }
};

// Void specialization
template <>
struct ChannelAwaiter<void> {
    bool ready;
    bool suspended_ = false;

    ChannelAwaiter() : ready(true), suspended_(false) {}
    ChannelAwaiter(bool needs_suspend) : ready(!needs_suspend), suspended_(needs_suspend) {}

    bool has_value() const { return ready; }
    bool needs_suspend() const { return suspended_; }
};

template <typename E>
struct ChannelIterator {
    virtual ChannelAwaiter<bool> has_next() = 0; // suspend
    virtual E next() = 0;
    virtual ~ChannelIterator() = default;
};

/**
 * Interface for the sending side of a channel.
 * 
 * SendChannel provides operations to send elements to a channel, check if the
 * channel is closed for sending, and close the channel. It supports both
 * suspending and non-suspending send operations.
 * 
 * @tparam E The type of elements that can be sent through this channel.
 */
template <typename E>
struct SendChannel {
    virtual ~SendChannel() = default;
    
    /**
     * Checks if this channel is closed for sending.
     * When true, no more elements can be sent and send operations will fail.
     * @return true if the channel is closed for sending.
     */
    virtual bool is_closed_for_send() const = 0;
    
    /**
     * Sends an element to the channel, suspending if the channel is full.
     * This is a suspending function that will resume when the element is sent
     * or the channel is closed.
     * 
     * @param element The element to send.
     * @return ChannelAwaiter that handles the suspension semantics.
     * @throws ClosedSendChannelException if the channel is closed.
     */
    virtual ChannelAwaiter<void> send(E element) = 0; // suspend
    
    /**
     * Attempts to send an element without suspending.
     * This is a non-blocking operation that immediately returns the result.
     * 
     * @param element The element to send.
     * @return ChannelResult indicating success, failure, or closed state.
     */
    virtual ChannelResult<void> try_send(E element) = 0;
    
    /**
     * Closes the channel for sending.
     * After closing, no more elements can be sent, but existing elements
     * in the buffer can still be received.
     * 
     * @param cause Optional exception that caused the closure.
     * @return true if this call closed the channel, false if already closed.
     */
    virtual bool close(std::exception_ptr cause = nullptr) = 0;
    
    /**
     * Registers a handler to be called when the channel is closed.
     * The handler is called immediately if the channel is already closed.
     * 
     * @param handler Function to call on channel closure.
     */
    virtual void invoke_on_close(std::function<void(std::exception_ptr)> handler) = 0;

    /**
     * Select clause that becomes selected when sending an element to this channel.
     * Mirrors Kotlin's `SendChannel.onSend`.
     *
     * NOTE: Returns a shared_ptr because SelectClause2 is abstract; callers
     *       should keep the returned clause alive for the duration of select.
     */
    virtual std::shared_ptr<selects::SelectClause2<E, SendChannel<E>>> on_send() = 0;
};

template <typename E>
struct ReceiveChannel {
    virtual ~ReceiveChannel() = default;
    virtual bool is_closed_for_receive() const = 0;
    virtual bool is_empty() const = 0;
    virtual ChannelAwaiter<E> receive() = 0; // suspend
    virtual ChannelAwaiter<ChannelResult<E>> receive_catching() = 0; // suspend
    virtual ChannelResult<E> try_receive() = 0;
    virtual ChannelAwaiter<ChannelResult<E>> receive_with_timeout(long timeout_millis) = 0; // suspend with timeout
    virtual std::shared_ptr<ChannelIterator<E>> iterator() = 0;

    virtual void cancel(std::exception_ptr cause = nullptr) = 0;

    /**
     * Select clause that becomes selected when an element is available to receive.
     * Mirrors Kotlin's `ReceiveChannel.onReceive`.
     *
     * NOTE: Returns a shared_ptr because SelectClause1 is abstract; callers
     *       should keep the returned clause alive for the duration of select.
     */
    virtual std::shared_ptr<selects::SelectClause1<E>> on_receive() = 0;
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
 * Creates a channel with the specified capacity.
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
