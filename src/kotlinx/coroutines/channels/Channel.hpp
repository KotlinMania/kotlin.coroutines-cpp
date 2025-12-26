#pragma once
/**
 * @file Channel.hpp
 * @brief Channel interfaces and types.
 *
 * Transliterated from: kotlinx-coroutines-core/common/src/channels/Channel.kt
 * Lines 1-1485 (complete file)
 *
 * This file contains the core channel interfaces: SendChannel, ReceiveChannel,
 * Channel, ChannelResult, and ChannelIterator.
 */

#include "kotlinx/coroutines/Continuation.hpp"
#include "kotlinx/coroutines/CancellableContinuation.hpp"
#include "kotlinx/coroutines/selects/Select.hpp"
#include "kotlinx/coroutines/channels/BufferOverflow.hpp"
#include "kotlinx/coroutines/internal/Symbol.hpp"
#include "kotlinx/coroutines/internal/SystemProps.hpp"
#include <memory>
#include <exception>
#include <functional>
#include <string>
#include <stdexcept>
#include <type_traits>

namespace kotlinx {
namespace coroutines {
namespace channels {

// Forward declarations
template <typename E> class SendChannel;
template <typename E> class ReceiveChannel;
template <typename E> class Channel;
template <typename E> class ChannelIterator;
template <typename T> class ChannelResult;
template <typename E> class BufferedChannel;
template <typename E> class ConflatedBufferedChannel;

// =============================================================================
// Channel capacity constants (namespace level for easy access)
// =============================================================================

/**
 * Line 1236-1255: Unlimited buffer capacity.
 */
constexpr int CHANNEL_UNLIMITED = 2147483647; // Int.MAX_VALUE

/**
 * Line 1257-1289: Zero buffer capacity (rendezvous channel).
 */
constexpr int CHANNEL_RENDEZVOUS = 0;

/**
 * Line 1291-1315: Single-element buffer with conflating behavior.
 */
constexpr int CHANNEL_CONFLATED = -1;

/**
 * Line 1317-1347: Default buffer capacity marker.
 */
constexpr int CHANNEL_BUFFERED = -2;

/**
 * Line 20-22: Handler for elements that were sent to a channel but were not
 * delivered to the consumer. This can happen when elements are dropped due to
 * buffer overflow or when operations are cancelled.
 */
template <typename E>
using OnUndeliveredElement = std::function<void(E)>;

// =============================================================================
// =============================================================================

/**
 * Line 1457-1470: Indicates an attempt to send to a channel that was
 * closed for sending.
 *
 * Indicates an attempt to [send][SendChannel.send] to a
 * [closed-for-sending][SendChannel.isClosedForSend] channel
 * that was [closed][SendChannel.close] without a cause.
 *
 * If a cause was provided, that cause is thrown from [send][SendChannel.send]
 * instead of this exception.
 * In particular, if the channel was closed because it was
 * [cancelled][ReceiveChannel.cancel],
 * this exception will never be thrown: either the `cause` of the cancellation
 * is thrown, or a new [CancellationException] gets constructed to be thrown
 * from [SendChannel.send].
 *
 * This exception is a subclass of [IllegalStateException], because the sender
 * should not attempt to send to a closed channel after it itself has
 * [closed][SendChannel.close] it, and indicates an error on the part of the
 * programmer.
 * Usually, this exception can be avoided altogether by restructuring the code.
 */
class ClosedSendChannelException : public std::logic_error {
public:
    explicit ClosedSendChannelException(const std::string& message)
        : std::logic_error(message) {}
};

// =============================================================================
// =============================================================================

/**
 * Line 1472-1484: Indicates an attempt to receive from a channel that was
 * closed for receiving.
 *
 * Indicates an attempt to [receive][ReceiveChannel.receive] from a
 * [closed-for-receiving][ReceiveChannel.isClosedForReceive] channel
 * that was [closed][SendChannel.close] without a cause.
 *
 * If a clause was provided, that clause is thrown from
 * [receive][ReceiveChannel.receive] instead of this exception.
 * In particular, if the channel was closed because it was
 * [cancelled][ReceiveChannel.cancel],
 * this exception will never be thrown: either the `cause` of the cancellation
 * is thrown, or a new [CancellationException] gets constructed to be thrown
 * from [ReceiveChannel.receive].
 *
 * This exception is a subclass of [NoSuchElementException] to be consistent
 * with plain collections.
 */
class ClosedReceiveChannelException : public std::runtime_error {
public:
    explicit ClosedReceiveChannelException(const std::string& message)
        : std::runtime_error(message) {}
};

inline const char* DEFAULT_CLOSE_MESSAGE = "Channel was closed";

// =============================================================================
// =============================================================================

/**
 * Line 788-811: A discriminated union representing a channel operation result.
 *
 * It encapsulates the knowledge of whether the operation succeeded, failed
 * with an option to retry, or failed because the channel was closed.
 *
 * If the operation was [successful][isSuccess], [T] is the result of the
 * operation: for example, for [ReceiveChannel.receiveCatching] and
 * [ReceiveChannel.tryReceive], it is the element received from the channel,
 * and for [Channel.trySend], it is [Unit], as the channel does not receive
 * anything in return for sending a channel.
 * This value can be retrieved with [getOrNull] or [getOrThrow].
 *
 * If the operation [failed][isFailure], it does not necessarily mean that the
 * channel itself is closed. For example, [ReceiveChannel.receiveCatching] and
 * [ReceiveChannel.tryReceive] can fail because the channel is empty, and
 * [Channel.trySend] can fail because the channel is full.
 *
 * If the operation [failed][isFailure] because the channel was closed for
 * that operation, [isClosed] returns `true`. The opposite is also true: if
 * [isClosed] returns `true`, then the channel is closed for that operation.
 * In this case, retrying the operation is meaningless: once closed, the
 * channel will remain closed. The [exceptionOrNull] function returns the
 * reason the channel was closed, if any was given.
 *
 * Manually obtaining a [ChannelResult] instance is not supported.
 * See the documentation for [ChannelResult]-returning functions for usage
 * examples.
 */
template <typename T>
class ChannelResult {
public:
    class Failed {
    public:
        virtual ~Failed() = default;
        virtual std::string to_string() const { return "Failed"; }
    };

    class Closed : public Failed {
    public:
        std::exception_ptr cause;

        explicit Closed(std::exception_ptr c = nullptr) : cause(c) {}

        bool operator==(const Closed& other) const {
            // Exception comparison by pointer (identity)
            return cause == other.cause;
        }

        std::string to_string() const override {
            return "Closed(" + std::string(cause ? "cause" : "null") + ")";
        }
    };

private:
    // Using variant-like storage
    enum class HolderType { VALUE, FAILED, CLOSED };
    HolderType holder_type_;

    union {
        T value_;
        Failed* failed_;
        Closed closed_;
    };

    // Private constructor for internal use
    explicit ChannelResult(HolderType type) : holder_type_(type) {
        if (type == HolderType::FAILED) {
            failed_ = get_failed_singleton();
        }
    }

    static Failed* get_failed_singleton() {
        static Failed instance;
        return &instance;
    }

public:
    // Construct success
    explicit ChannelResult(T value) : holder_type_(HolderType::VALUE), value_(std::move(value)) {}

    // Construct closed
    explicit ChannelResult(Closed closed) : holder_type_(HolderType::CLOSED), closed_(std::move(closed)) {}

    // Copy constructor
    ChannelResult(const ChannelResult& other) : holder_type_(other.holder_type_) {
        switch (holder_type_) {
            case HolderType::VALUE: new (&value_) T(other.value_); break;
            case HolderType::FAILED: failed_ = other.failed_; break;
            case HolderType::CLOSED: new (&closed_) Closed(other.closed_); break;
        }
    }

    // Move constructor
    ChannelResult(ChannelResult&& other) noexcept : holder_type_(other.holder_type_) {
        switch (holder_type_) {
            case HolderType::VALUE: new (&value_) T(std::move(other.value_)); break;
            case HolderType::FAILED: failed_ = other.failed_; break;
            case HolderType::CLOSED: new (&closed_) Closed(std::move(other.closed_)); break;
        }
    }

    // Copy assignment operator
    ChannelResult& operator=(const ChannelResult& other) {
        if (this != &other) {
            // Destroy current content
            this->~ChannelResult();
            // Copy construct in place
            holder_type_ = other.holder_type_;
            switch (holder_type_) {
                case HolderType::VALUE: new (&value_) T(other.value_); break;
                case HolderType::FAILED: failed_ = other.failed_; break;
                case HolderType::CLOSED: new (&closed_) Closed(other.closed_); break;
            }
        }
        return *this;
    }

    // Move assignment operator
    ChannelResult& operator=(ChannelResult&& other) noexcept {
        if (this != &other) {
            // Destroy current content
            this->~ChannelResult();
            // Move construct in place
            holder_type_ = other.holder_type_;
            switch (holder_type_) {
                case HolderType::VALUE: new (&value_) T(std::move(other.value_)); break;
                case HolderType::FAILED: failed_ = other.failed_; break;
                case HolderType::CLOSED: new (&closed_) Closed(std::move(other.closed_)); break;
            }
        }
        return *this;
    }

    ~ChannelResult() {
        switch (holder_type_) {
            case HolderType::VALUE: value_.~T(); break;
            case HolderType::FAILED: break;
            case HolderType::CLOSED: closed_.~Closed(); break;
        }
    }

    /**
     * Line 815-843: val isSuccess
     *
     * Whether the operation succeeded.
     */
    [[nodiscard]] bool is_success() const {
        return holder_type_ == HolderType::VALUE;
    }

    /**
     * Line 846-851: val isFailure
     *
     * Whether the operation failed.
     * A shorthand for `!isSuccess`.
     */
    [[nodiscard]] bool is_failure() const {
        return holder_type_ != HolderType::VALUE;
    }

    /**
     * Line 853-873: val isClosed
     *
     * Whether the operation failed because the channel was closed.
     */
    [[nodiscard]] bool is_closed() const {
        return holder_type_ == HolderType::CLOSED;
    }

    /**
     * Line 875-895: fun getOrNull(): T?
     *
     * Returns the encapsulated [T] if the operation succeeded, or `null`
     * if it failed.
     */
    T* get_or_null() {
        if (holder_type_ == HolderType::VALUE) {
            return &value_;
        }
        return nullptr;
    }

    const T* get_or_null() const {
        if (holder_type_ == HolderType::VALUE) {
            return &value_;
        }
        return nullptr;
    }

    /**
     * Line 899-920: fun getOrThrow(): T
     *
     * Returns the encapsulated [T] if the operation succeeded, or throws
     * the encapsulated exception if it failed.
     *
     * @throws std::logic_error if the operation failed, but the channel was
     *         not closed with a cause.
     */
    T get_or_throw() const {
        if (holder_type_ == HolderType::VALUE) {
            return value_;
        }
        if (holder_type_ == HolderType::CLOSED) {
            if (closed_.cause) {
                std::rethrow_exception(closed_.cause);
            }
            throw std::logic_error(
                "Trying to call 'getOrThrow' on a channel closed without a cause");
        }
        throw std::logic_error(
            "Trying to call 'getOrThrow' on a failed result of a non-closed channel");
    }

    /**
     * Line 922-940: fun exceptionOrNull(): Throwable?
     *
     * Returns the exception with which the channel was closed, or `null` if
     * the channel was not closed or was closed without a cause.
     */
    [[nodiscard]] std::exception_ptr exception_or_null() const {
        if (holder_type_ == HolderType::CLOSED) {
            return closed_.cause;
        }
        return nullptr;
    }

    /**
     * Line 972-976: override fun toString()
     */
    std::string to_string() const {
        switch (holder_type_) {
            case HolderType::CLOSED: return closed_.to_string();
            case HolderType::FAILED: return "Failed";
            default: return "Value(...)";
        }
    }


    /**
     * Line 959-961: fun <E> success(value: E): ChannelResult<E>
     */
    static ChannelResult<T> success(T value) {
        return ChannelResult<T>(std::move(value));
    }

    /**
     * Line 963-965: fun <E> failure(): ChannelResult<E>
     */
    static ChannelResult<T> failure() {
        return ChannelResult<T>(HolderType::FAILED);
    }

    /**
     * Line 967-969: fun <E> closed(cause: Throwable?): ChannelResult<E>
     */
    static ChannelResult<T> closed(std::exception_ptr cause = nullptr) {
        return ChannelResult<T>(Closed(cause));
    }
};

// =============================================================================
// Template specialization for ChannelResult<void>
// =============================================================================

/**
 * Specialization for void type - represents success/failure without a value.
 */
template <>
class ChannelResult<void> {
public:
    // Same nested types as the general template
    class Failed {
    public:
        std::string to_string() const { return "Failed"; }
    };

    class Closed {
    public:
        std::exception_ptr cause;

        Closed() : cause(nullptr) {}
        explicit Closed(std::exception_ptr c) : cause(c) {}

        std::string to_string() const {
            return "Closed(" + std::string(cause ? "cause" : "null") + ")";
        }
    };

private:
    enum class HolderType { VALUE, FAILED, CLOSED };
    HolderType holder_type_;
    Closed closed_;  // Only used when CLOSED

    explicit ChannelResult(HolderType type) : holder_type_(type) {}

public:
    // Construct success (void)
    ChannelResult() : holder_type_(HolderType::VALUE) {}

    // Construct closed
    explicit ChannelResult(Closed closed) : holder_type_(HolderType::CLOSED), closed_(std::move(closed)) {}

    // Copy/move constructors
    ChannelResult(const ChannelResult& other)
        : holder_type_(other.holder_type_), closed_(other.closed_) {}
    ChannelResult(ChannelResult&& other) noexcept
        : holder_type_(other.holder_type_), closed_(std::move(other.closed_)) {}

    // Copy assignment operator
    ChannelResult& operator=(const ChannelResult& other) {
        if (this != &other) {
            holder_type_ = other.holder_type_;
            closed_ = other.closed_;
        }
        return *this;
    }

    // Move assignment operator
    ChannelResult& operator=(ChannelResult&& other) noexcept {
        if (this != &other) {
            holder_type_ = other.holder_type_;
            closed_ = std::move(other.closed_);
        }
        return *this;
    }

    [[nodiscard]] bool is_success() const { return holder_type_ == HolderType::VALUE; }
    [[nodiscard]] bool is_failure() const { return holder_type_ != HolderType::VALUE; }
    [[nodiscard]] bool is_closed() const { return holder_type_ == HolderType::CLOSED; }

    [[nodiscard]] std::exception_ptr exception_or_null() const {
        if (holder_type_ == HolderType::CLOSED) {
            return closed_.cause;
        }
        return nullptr;
    }

    std::string to_string() const {
        switch (holder_type_) {
            case HolderType::CLOSED: return closed_.to_string();
            case HolderType::FAILED: return "Failed";
            default: return "Success";
        }
    }

    static ChannelResult<void> success() {
        return ChannelResult<void>();
    }

    static ChannelResult<void> failure() {
        return ChannelResult<void>(HolderType::FAILED);
    }

    static ChannelResult<void> closed(std::exception_ptr cause = nullptr) {
        return ChannelResult<void>(Closed(cause));
    }
};

// =============================================================================
// ChannelAwaiter - wraps a value or indicates suspension is needed
// =============================================================================

/**
 * Awaiter for channel operations that may need to suspend.
 * This is a temporary construct until full coroutine support is in place.
 */
template <typename T>
class ChannelAwaiter {
public:
    ChannelAwaiter() : has_value_(true), needs_suspend_(false) {}

    explicit ChannelAwaiter(T value)
        : has_value_(true), needs_suspend_(false), value_(std::move(value)) {}

    explicit ChannelAwaiter(bool needs_suspend)
        : has_value_(false), needs_suspend_(needs_suspend) {}

    [[nodiscard]] bool has_value() const { return has_value_; }
    [[nodiscard]] bool needs_suspend() const { return needs_suspend_; }

    T& value() {
        if (!has_value_) throw std::logic_error("ChannelAwaiter has no value");
        return value_;
    }

    const T& value() const {
        if (!has_value_) throw std::logic_error("ChannelAwaiter has no value");
        return value_;
    }

private:
    bool has_value_;
    bool needs_suspend_;
    T value_;
};

/**
 * Specialization for void - no value, just success/suspend indicator
 */
template <>
class ChannelAwaiter<void> {
public:
    ChannelAwaiter() : needs_suspend_(false) {}
    explicit ChannelAwaiter(bool needs_suspend) : needs_suspend_(needs_suspend) {}

    [[nodiscard]] bool has_value() const { return !needs_suspend_; }
    [[nodiscard]] bool needs_suspend() const { return needs_suspend_; }

private:
    bool needs_suspend_;
};

// =============================================================================
// =============================================================================

/**
 * Line 979-995: fun <T> ChannelResult<T>.getOrElse(onFailure: ...)
 *
 * Returns the encapsulated value if the operation succeeded, or the
 * result of [onFailure] function for [ChannelResult.exceptionOrNull]
 * otherwise.
 */
template <typename T, typename OnFailure>
T get_or_else(const ChannelResult<T>& result, OnFailure&& on_failure) {
    if (result.is_success()) {
        return *result.get_or_null();
    }
    return on_failure(result.exception_or_null());
}

/**
 * Line 997-1011: fun <T> ChannelResult<T>.onSuccess(action: ...)
 *
 * Performs the given [action] on the encapsulated value if the operation
 * succeeded. Returns the original `ChannelResult` unchanged.
 */
template <typename T, typename Action>
ChannelResult<T>& on_success(ChannelResult<T>& result, Action&& action) {
    if (result.is_success()) {
        action(*result.get_or_null());
    }
    return result;
}

/**
 * Line 1013-1028: fun <T> ChannelResult<T>.onFailure(action: ...)
 *
 * Performs the given [action] if the operation failed.
 * Returns the original `ChannelResult` unchanged.
 */
template <typename T, typename Action>
ChannelResult<T>& on_failure(ChannelResult<T>& result, Action&& action) {
    if (result.is_failure()) {
        action(result.exception_or_null());
    }
    return result;
}

/**
 * Line 1030-1049: fun <T> ChannelResult<T>.onClosed(action: ...)
 *
 * Performs the given [action] if the operation failed because the channel
 * was closed. Returns the original `ChannelResult` unchanged.
 */
template <typename T, typename Action>
ChannelResult<T>& on_closed(ChannelResult<T>& result, Action&& action) {
    if (result.is_closed()) {
        action(result.exception_or_null());
    }
    return result;
}

// =============================================================================
// =============================================================================

/**
 * Line 1051-1054: Iterator for a [ReceiveChannel].
 *
 * Instances of this interface are *not thread-safe* and shall not be used
 * from concurrent coroutines.
 */
template <typename E>
class ChannelIterator {
public:
    virtual ~ChannelIterator() = default;

    /**
     * Line 1056-1080: suspend operator fun hasNext(): Boolean
     *
     * Prepare an element for retrieval by the invocation of [next].
     *
     * - If the element that was retrieved by an earlier [hasNext] call was
     *   not yet consumed by [next], returns `true`.
     * - If the channel has an element available, returns `true` and removes
     *   it from the channel. This element will be returned by the subsequent
     *   invocation of [next].
     * - If the channel is [closed for receiving] without a cause, returns
     *   `false`.
     * - If the channel is closed with a cause, throws the original [close]
     *   cause exception.
     * - If the channel is not closed but does not contain an element,
     *   suspends until either an element is sent to the channel or the
     *   channel gets closed.
     *
     * This suspending function is cancellable.
     *
     * @param continuation The continuation to resume when complete.
     * @return COROUTINE_SUSPENDED or boxed bool result.
     */
    virtual void* has_next(Continuation<void*>* continuation) = 0;

    /**
     * Line 1096-1120: operator fun next(): E
     *
     * Retrieves the element removed from the channel by the preceding call
     * to [hasNext], or throws an [IllegalStateException] if [hasNext] was
     * not invoked.
     *
     * This method can only be used together with [hasNext]:
     * ```cpp
     * while (iterator.has_next()) {
     *     auto element = iterator.next();
     *     // ... handle the element ...
     * }
     * ```
     */
    virtual E next() = 0;
};

// =============================================================================
// =============================================================================

/**
 * Line 15-22: Sender's interface to a [Channel].
 *
 * Combined, [SendChannel] and [ReceiveChannel] define the complete [Channel]
 * interface.
 *
 * It is not expected that this interface will be implemented directly.
 * Instead, the existing [Channel] implementations can be used or delegated to.
 */
template <typename E>
class SendChannel {
public:
    virtual ~SendChannel() = default;

    /**
     * Line 24-76: val isClosedForSend: Boolean
     *
     * Returns `true` if this channel was closed by an invocation of [close]
     * or its receiving side was [cancelled][ReceiveChannel.cancel].
     * This means that calling [send] will result in an exception.
     *
     * Note that if this property returns `false`, it does not guarantee that
     * a subsequent call to [send] will succeed, as the channel can be
     * concurrently closed right after the check.
     *
     * @DelicateCoroutinesApi
     */
    virtual bool is_closed_for_send() const = 0;

    /**
     * Line 78-153: suspend fun send(element: E)
     *
     * Sends the specified [element] to this channel.
     *
     * This function suspends if it does not manage to pass the element to
     * the channel's buffer (or directly the receiving side if there's no
     * buffer), and it can be cancelled with or without having successfully
     * passed the element.
     *
     * If the channel is [closed][close], an exception is thrown.
     *
     * ## Suspending and cancellation
     *
     * This suspending function is cancellable: if the [Job] of the current
     * coroutine is cancelled while this suspending function is waiting,
     * this function immediately resumes with [CancellationException].
     * There is a **prompt cancellation guarantee**.
     *
     * ## Sending to a closed channel
     *
     * If a channel was [closed][close] before [send] was called and no cause
     * was specified, [ClosedSendChannelException] will be thrown.
     *
     * @param element The element to send.
     * @param continuation The continuation to resume when send completes.
     * @return COROUTINE_SUSPENDED or nullptr (Unit).
     */
    virtual void* send(E element, Continuation<void*>* continuation) = 0;

    /**
     * Line 155-195: val onSend: SelectClause2<E, SendChannel<E>>
     *
     * Clause for the [select] expression of the [send] suspending function.
     */
    virtual selects::SelectClause2<E, SendChannel<E>*>& on_send() = 0;

    /**
     * Line 197-226: fun trySend(element: E): ChannelResult<Unit>
     *
     * Attempts to add the specified [element] to this channel without waiting.
     *
     * [trySend] never suspends and never throws exceptions.
     * Instead, it returns a [ChannelResult] that encapsulates the result of
     * the operation.
     *
     * If this channel is currently full and cannot receive new elements at
     * the time or is [closed][close], this function returns a result that
     * indicates [a failure][ChannelResult.isFailure].
     */
    virtual ChannelResult<void> try_send(E element) = 0;

    /**
     * Line 228-263: fun close(cause: Throwable? = null): Boolean
     *
     * Closes this channel so that subsequent attempts to [send] to it fail.
     *
     * Returns `true` if the channel was not closed previously and the call
     * to this function closed it.
     * If the channel was already closed, this function does nothing and
     * returns `false`.
     *
     * The existing elements in the channel remain there, and likewise,
     * the calls to [send] that have suspended before [close] was called
     * will not be affected. Only subsequent calls will fail.
     *
     * @param cause Optional exception that caused the closure.
     * @return true if this call closed the channel.
     */
    virtual bool close(std::exception_ptr cause = nullptr) = 0;

    /**
     * Line 265-310: fun invokeOnClose(handler: (cause: Throwable?) -> Unit)
     *
     * Registers a [handler] that is synchronously invoked once the channel
     * is [closed][close] or the receiving side is [cancelled].
     *
     * Only one handler can be attached to a channel during its lifetime.
     * The `handler` is invoked when [isClosedForSend] starts to return `true`.
     * If the channel is closed already, the handler is invoked immediately.
     *
     * @throws std::logic_error if another handler was already registered.
     */
    virtual void invoke_on_close(std::function<void(std::exception_ptr)> handler) = 0;

    /**
     * Line 312-348: @Deprecated fun offer(element: E): Boolean
     *
     * Deprecated in favor of 'trySend' method.
     */
    [[deprecated("Deprecated in favor of 'trySend' method")]]
    bool offer(E element) {
        auto result = try_send(std::move(element));
        if (result.is_success()) return true;
        auto ex = result.exception_or_null();
        if (ex) std::rethrow_exception(ex);
        return false;
    }
};

// =============================================================================
// =============================================================================

/**
 * Line 351-355: Receiver's interface to a [Channel].
 *
 * Combined, [SendChannel] and [ReceiveChannel] define the complete [Channel]
 * interface.
 */
template <typename E>
class ReceiveChannel {
public:
    virtual ~ReceiveChannel() = default;

    /**
     * Line 357-391: val isClosedForReceive: Boolean
     *
     * Returns `true` if the sending side of this channel was [closed] and
     * all previously sent items were already received.
     *
     * @DelicateCoroutinesApi
     */
    virtual bool is_closed_for_receive() const = 0;

    /**
     * Line 393-416: val isEmpty: Boolean
     *
     * Returns `true` if the channel contains no elements and isn't
     * [closed for receive].
     *
     * @ExperimentalCoroutinesApi
     */
    virtual bool is_empty() const = 0;

    /**
     * Line 418-480: suspend fun receive(): E
     *
     * Retrieves an element, removing it from the channel.
     *
     * This function suspends if the channel is empty, waiting until an
     * element is available. If the channel is [closed for receive], an
     * exception is thrown.
     *
     * ## Suspending and cancellation
     *
     * This suspending function is cancellable.
     * There is a **prompt cancellation guarantee**.
     *
     * @param continuation The continuation to resume when element available.
     * @return COROUTINE_SUSPENDED or boxed element.
     */
    virtual void* receive(Continuation<void*>* continuation) = 0;

    /**
     * Line 482-514: val onReceive: SelectClause1<E>
     *
     * Clause for the [select] expression of the [receive] suspending function.
     */
    virtual selects::SelectClause1<E>& on_receive() = 0;

    /**
     * Line 516-591: suspend fun receiveCatching(): ChannelResult<E>
     *
     * Retrieves an element, removing it from the channel.
     *
     * A difference from [receive] is that this function encapsulates a
     * failure in its return value instead of throwing an exception.
     *
     * @param continuation The continuation to resume when complete.
     * @return COROUTINE_SUSPENDED or boxed ChannelResult<E>.
     */
    virtual void* receive_catching(Continuation<void*>* continuation) = 0;

    /**
     * Line 593-602: val onReceiveCatching: SelectClause1<ChannelResult<E>>
     *
     * Clause for the [select] expression of the [receiveCatching] function.
     */
    virtual selects::SelectClause1<ChannelResult<E>>& on_receive_catching() = 0;

    /**
     * Line 604-633: fun tryReceive(): ChannelResult<E>
     *
     * Attempts to retrieve an element without waiting, removing it from
     * the channel.
     *
     * - When the channel is non-empty, a successful result is returned.
     * - When the channel is empty, a failed result is returned.
     * - When the channel is closed for receive, a closed result is returned.
     */
    virtual ChannelResult<E> try_receive() = 0;

    /**
     * Line 635-659: operator fun iterator(): ChannelIterator<E>
     *
     * Returns a new iterator to receive elements from this channel using
     * a `for` loop.
     */
    virtual std::unique_ptr<ChannelIterator<E>> iterator() = 0;

    /**
     * Line 661-695: fun cancel(cause: CancellationException? = null)
     *
     * [Closes][SendChannel.close] the channel for new elements and removes
     * all existing ones.
     */
    virtual void cancel(std::exception_ptr cause = nullptr) = 0;

    /**
     * Line 697-701: @Deprecated fun cancel(): Unit
     *
     * Binary compatibility.
     */
    void cancel() { cancel(nullptr); }

    /**
     * Line 709-742: @Deprecated fun poll(): E?
     *
     * Deprecated in favor of 'tryReceive'.
     */
    [[deprecated("Deprecated in favor of 'tryReceive'")]]
    E* poll() {
        auto result = try_receive();
        if (result.is_success()) {
            // Caller must handle lifetime
            return result.get_or_null();
        }
        auto ex = result.exception_or_null();
        if (ex) std::rethrow_exception(ex);
        return nullptr;
    }
};

// =============================================================================
// =============================================================================

/**
 * Line 1123-1231: Channel
 *
 * Channel is a non-blocking primitive for communication between a sender
 * (via [SendChannel]) and a receiver (via [ReceiveChannel]).
 *
 * Conceptually, a channel is similar to `java.util.concurrent.BlockingQueue`,
 * but it has suspending operations instead of blocking ones and can be
 * [closed][SendChannel.close].
 *
 * ### Channel capacity
 *
 * Most ways to create a [Channel] allow specifying a capacity, which
 * determines how elements are buffered in the channel.
 *
 * ### Buffer overflow
 *
 * Some ways to create a [Channel] also expose a [BufferOverflow] parameter.
 *
 * ### Prompt cancellation guarantee
 *
 * All suspending functions with channels provide **prompt cancellation
 * guarantee**.
 *
 * ### Undelivered elements
 *
 * As a result of the prompt cancellation guarantee, when a closeable resource
 * is transferred via a channel, it can be lost if cancelled in parallel.
 * The `Channel()` factory function has the optional parameter
 * `onUndeliveredElement` to handle this.
 */
template <typename E>
class Channel : public virtual SendChannel<E>, public virtual ReceiveChannel<E> {
public:
    /**
     * Line 1232-1371: companion object Factory
     *
     * Constants for the channel factory function `Channel()`.
     */

    /**
     * Line 1236-1255: const val UNLIMITED: Int = Int.MAX_VALUE
     *
     * An unlimited buffer capacity.
     */
    static constexpr int UNLIMITED = 2147483647; // Int.MAX_VALUE

    /**
     * Line 1257-1289: const val RENDEZVOUS: Int = 0
     *
     * The zero buffer capacity (rendezvous channel).
     */
    static constexpr int RENDEZVOUS = 0;

    /**
     * Line 1291-1315: const val CONFLATED: Int = -1
     *
     * A single-element buffer with conflating behavior.
     */
    static constexpr int CONFLATED = -1;

    /**
     * Line 1317-1347: const val BUFFERED: Int = -2
     *
     * A channel capacity marker that is substituted by the default buffer
     * capacity.
     */
    static constexpr int BUFFERED = -2;

    static constexpr int OPTIONAL_CHANNEL = -3;

    /**
     * Line 1352-1365: const val DEFAULT_BUFFER_PROPERTY_NAME: String
     *
     * Name of the system property for the default channel capacity.
     */
    static constexpr const char* DEFAULT_BUFFER_PROPERTY_NAME =
        "kotlinx.coroutines.channels.defaultBuffer";

    /**
     * Line 1367-1369: internal val CHANNEL_DEFAULT_CAPACITY
     *
     * The default channel capacity (64 by default).
     */
    static int channel_default_capacity() {
        static int value = internal::system_prop_int(
            DEFAULT_BUFFER_PROPERTY_NAME, 64, 1, UNLIMITED - 1);
        return value;
    }
};

// =============================================================================
// =============================================================================

/**
 * Line 1373-1456: fun <E> Channel(...)
 *
 * Creates a channel. See the [Channel] interface documentation for details.
 *
 * @param capacity either a positive channel capacity or one of the constants
 *        defined in [Channel.Factory].
 * @param on_buffer_overflow configures an action on buffer overflow.
 * @param on_undelivered_element a function called when element was sent but
 *        was not delivered to the consumer.
 * @throws std::invalid_argument when [capacity] < -2
 */
// Declaration - implementation in Channels.hpp
template <typename E>
std::shared_ptr<Channel<E>> create_channel(
    int capacity = Channel<E>::RENDEZVOUS,
    BufferOverflow on_buffer_overflow = BufferOverflow::SUSPEND,
    OnUndeliveredElement<E> on_undelivered_element = nullptr
);

} // namespace channels
} // namespace coroutines
} // namespace kotlinx
