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
#include "kotlinx/coroutines/CoroutineScope.hpp"

namespace kotlinx {
namespace coroutines {
namespace channels {

using kotlinx::coroutines::Continuation;
using kotlinx::coroutines::CancellationException;
using kotlinx::coroutines::CoroutineScope;
namespace selects = kotlinx::coroutines::selects;

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
 * An unlimited buffer capacity.
 *
 * `Channel(CHANNEL_UNLIMITED)` creates a channel with an unlimited buffer,
 * which never suspends the sender. The total amount of elements that can be
 * sent to the channel is limited only by the available memory.
 *
 * If [BufferOverflow] is specified for the channel, it is completely ignored,
 * as the channel never suspends the sender.
 *
 * ```cpp
 * auto channel = create_channel<int>(CHANNEL_UNLIMITED);
 * for (int i = 0; i < 1000; ++i) {
 *    channel->try_send(i);
 * }
 * for (int i = 0; i < 1000; ++i) {
 *    assert(channel->try_receive().get_or_null() == i);
 * }
 * ```
 *
 * Transliterated from:
 * public const val UNLIMITED: Int = Int.MAX_VALUE
 */
constexpr int CHANNEL_UNLIMITED = 2147483647; // Int.MAX_VALUE

/**
 * The zero buffer capacity (rendezvous channel).
 *
 * For the default [BufferOverflow] value of [BufferOverflow::SUSPEND],
 * `Channel(CHANNEL_RENDEZVOUS)` creates a channel without a buffer.
 * An element is transferred from the sender to the receiver only when [send]
 * and [receive] invocations meet in time (that is, they _rendezvous_),
 * so [send] suspends until another coroutine invokes [receive],
 * and [receive] suspends until another coroutine invokes [send].
 *
 * ```cpp
 * auto channel = create_channel<int>(CHANNEL_RENDEZVOUS);
 * assert(channel->try_send(5).is_failure()); // sending fails: no receiver waiting
 * // launch receiver...
 * assert(channel->try_send(3).is_success()); // sending succeeds: receiver waiting
 * ```
 *
 * If a different [BufferOverflow] is specified,
 * `Channel(CHANNEL_RENDEZVOUS)` creates a channel with a buffer of size 1.
 *
 * Transliterated from:
 * public const val RENDEZVOUS: Int = 0
 */
constexpr int CHANNEL_RENDEZVOUS = 0;

/**
 * A single-element buffer with conflating behavior.
 *
 * Specifying [CHANNEL_CONFLATED] as the capacity is equivalent to creating
 * a channel with a buffer of size 1 and a [BufferOverflow] strategy of
 * [BufferOverflow::DROP_OLDEST]:
 * `create_channel<T>(1, BufferOverflow::DROP_OLDEST)`.
 *
 * Such a channel buffers at most one element and conflates all subsequent
 * `send` and `try_send` invocations so that the receiver always gets the
 * last element sent, **losing** the previously sent elements.
 * See the "Undelivered elements" section in the [Channel] documentation.
 * [Sending][send] to this channel never suspends, and [try_send] always
 * succeeds.
 *
 * ```cpp
 * auto channel = create_channel<int>(CHANNEL_CONFLATED);
 * channel->try_send(1);
 * channel->try_send(2);
 * channel->try_send(3);
 * assert(*channel->try_receive().get_or_null() == 3);
 * ```
 *
 * Specifying a [BufferOverflow] other than [BufferOverflow::SUSPEND] is not
 * allowed with [CHANNEL_CONFLATED], and an exception is thrown if such a
 * combination is used.
 *
 * Transliterated from:
 * public const val CONFLATED: Int = -1
 */
constexpr int CHANNEL_CONFLATED = -1;

/**
 * A channel capacity marker that is substituted by the default buffer capacity.
 *
 * When passed as a parameter to the `create_channel()` function, the default
 * buffer capacity is used. For [BufferOverflow::SUSPEND] (the default buffer
 * overflow strategy), the default capacity is 64.
 *
 * ```cpp
 * auto channel = create_channel<int>(CHANNEL_BUFFERED);
 * for (int i = 0; i < 100; ++i) {
 *     channel->try_send(i);
 * }
 * channel->close();
 * // With default capacity 64, only first 64 elements are buffered
 * ```
 *
 * If a different [BufferOverflow] is specified, `Channel(CHANNEL_BUFFERED)`
 * creates a channel with a buffer of size 1.
 *
 * Transliterated from:
 * public const val BUFFERED: Int = -2
 */
constexpr int CHANNEL_BUFFERED = -2;

/**
 * Optional capacity for ChannelFlow.
 */
constexpr int CHANNEL_OPTIONAL = -3;

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
// ChannelIterator Interface
// =============================================================================

/**
 * Iterator for a [ReceiveChannel].
 * Instances of this interface are *not thread-safe* and shall not be used
 * from concurrent coroutines.
 *
 * Transliterated from:
 * public interface ChannelIterator<out E>
 */
template <typename E>
class ChannelIterator {
public:
    virtual ~ChannelIterator() = default;

    /**
     * Prepare an element for retrieval by the invocation of [next].
     *
     * - If the element that was retrieved by an earlier [has_next] call was
     *   not yet consumed by [next], returns `true`.
     * - If the channel has an element available, returns `true` and removes
     *   it from the channel. This element will be returned by the subsequent
     *   invocation of [next].
     * - If the channel is [closed for receiving][ReceiveChannel::is_closed_for_receive]
     *   without a cause, returns `false`.
     * - If the channel is closed with a cause, throws the original
     *   [close][SendChannel::close] cause exception.
     * - If the channel is not closed but does not contain an element,
     *   suspends until either an element is sent to the channel or the
     *   channel gets closed.
     *
     * This suspending function is cancellable: if the [Job] of the current
     * coroutine is cancelled while this suspending function is waiting,
     * this function immediately resumes with [CancellationException].
     * There is a **prompt cancellation guarantee**: even if [has_next]
     * retrieves the element from the channel during its operation, but was
     * cancelled while suspended, [CancellationException] will be thrown.
     * See [suspend_cancellable_coroutine] for low-level details.
     *
     * Because of the prompt cancellation guarantee, some values retrieved
     * from the channel can become lost.
     * See the "Undelivered elements" section in the [Channel] documentation
     * for details on handling undelivered elements.
     *
     * Note that this function does not check for cancellation when it is not
     * suspended, that is, if the next element is immediately available.
     * Use [ensure_active] or [CoroutineScope::is_active] to periodically
     * check for cancellation in tight loops if needed.
     *
     * @param continuation The continuation to resume when complete.
     * @return COROUTINE_SUSPENDED or boxed bool result.
     *
     * Transliterated from:
     * public suspend operator fun hasNext(): Boolean
     */
    virtual void* has_next(Continuation<void*>* continuation) = 0;

    /**
     * Retrieves the element removed from the channel by the preceding call
     * to [has_next], or throws an [std::logic_error] if [has_next] was not
     * invoked.
     *
     * This method can only be used together with [has_next]:
     * ```cpp
     * auto iter = channel->iterator();
     * while (iter->has_next(cont)) {
     *     auto element = iter->next();
     *     // ... handle the element ...
     * }
     * ```
     *
     * A more idiomatic way to iterate over a channel is to use a `for` loop
     * (when full range-based for support is available):
     * ```cpp
     * for (auto element : *channel) {
     *    // ... handle the element ...
     * }
     * ```
     *
     * This method never throws if [has_next] returned `true`.
     * If [has_next] threw the cause with which the channel was closed, this
     * method will rethrow the same exception.
     * If [has_next] returned `false` because the channel was closed without
     * a cause, this method throws a [ClosedReceiveChannelException].
     *
     * Transliterated from:
     * public operator fun next(): E
     */
    virtual E next() = 0;
};

// =============================================================================
// SendChannel Interface
// =============================================================================

/**
 * Sender's interface to a [Channel].
 *
 * Combined, [SendChannel] and [ReceiveChannel] define the complete [Channel]
 * interface.
 *
 * It is not expected that this interface will be implemented directly.
 * Instead, the existing [Channel] implementations can be used or delegated to.
 *
 * Transliterated from:
 * public interface SendChannel<in E>
 */
template <typename E>
class SendChannel {
public:
    virtual ~SendChannel() = default;

    /**
     * Returns `true` if this channel was closed by an invocation of [close]
     * or its receiving side was [cancelled][ReceiveChannel::cancel].
     * This means that calling [send] will result in an exception.
     *
     * Note that if this property returns `false`, it does not guarantee that
     * a subsequent call to [send] will succeed, as the channel can be
     * concurrently closed right after the check.
     * For such scenarios, [try_send] is the more robust solution: it attempts
     * to send the element and returns a result that says whether the channel
     * was closed, and if not, whether sending a value was successful.
     *
     * ```cpp
     * // DANGER! THIS CHECK IS NOT RELIABLE!
     * if (!channel->is_closed_for_send()) {
     *     channel->send(element, cont); // can still fail!
     * } else {
     *     std::cout << "Cannot send: the channel is closed\n";
     * }
     * // DO THIS INSTEAD:
     * on_closed(channel->try_send(element), [](auto) {
     *     std::cout << "Cannot send: the channel is closed\n";
     * });
     * ```
     *
     * The primary intended usage of this property is skipping some portions
     * of code that should not be executed if the channel is already known
     * to be closed, or for assertions and diagnostics.
     *
     * @DelicateCoroutinesApi
     *
     * Transliterated from:
     * public val isClosedForSend: Boolean
     */
    virtual bool is_closed_for_send() const = 0;

    /**
     * Sends the specified [element] to this channel.
     *
     * This function suspends if it does not manage to pass the element to
     * the channel's buffer (or directly the receiving side if there's no
     * buffer), and it can be cancelled with or without having successfully
     * passed the element.
     * See the "Suspending and cancellation" section below for details.
     * If the channel is [closed][close], an exception is thrown (see below).
     *
     * ```cpp
     * auto channel = create_channel<int>();
     * launch([&channel]() {
     *     assert(channel->receive(cont) == 5);
     * });
     * channel->send(5, cont); // suspends until 5 is received
     * ```
     *
     * ## Suspending and cancellation
     *
     * If the [BufferOverflow] strategy of this channel is [BufferOverflow::SUSPEND],
     * this function may suspend.
     * The exact scenarios differ depending on the channel's capacity:
     * - If the channel is [rendezvous][CHANNEL_RENDEZVOUS],
     *   the sender will be suspended until the receiver calls [receive].
     * - If the channel is [unlimited][CHANNEL_UNLIMITED] or [conflated][CHANNEL_CONFLATED],
     *   the sender will never be suspended even with [BufferOverflow::SUSPEND].
     * - If the channel is buffered, the sender will be suspended until the
     *   buffer has free space.
     *
     * This suspending function is cancellable: if the [Job] of the current
     * coroutine is cancelled while this suspending function is waiting,
     * this function immediately resumes with [CancellationException].
     * There is a **prompt cancellation guarantee**: even if [send] managed
     * to send the element, but was cancelled while suspended,
     * [CancellationException] will be thrown.
     *
     * ## Sending to a closed channel
     *
     * If a channel was [closed][close] before [send] was called and no cause
     * was specified, a [ClosedSendChannelException] will be thrown from [send].
     * If a channel was [closed][close] with a cause before [send] was called,
     * then [send] will rethrow the same exception that was passed to [close].
     *
     * In both cases, it is guaranteed that the element was not delivered to
     * the consumer, and the `on_undelivered_element` callback will be called.
     *
     * ## Related
     *
     * This function can be used in [select] invocations with the [on_send] clause.
     * Use [try_send] to try sending to this channel without waiting and throwing.
     *
     * @param element The element to send.
     * @param continuation The continuation to resume when send completes.
     * @return COROUTINE_SUSPENDED or nullptr (Unit).
     *
     * Transliterated from:
     * public suspend fun send(element: E)
     */
    virtual void* send(E element, Continuation<void*>* continuation) = 0;

    /**
     * Clause for the [select] expression of the [send] suspending function
     * that selects when the element that is specified as the parameter is
     * sent to the channel.
     * When the clause is selected, the reference to this channel is passed
     * into the corresponding block.
     *
     * The [select] invocation fails with an exception if the channel
     * [is closed for send][is_closed_for_send] before the [select] suspends
     * (see the "Sending to a closed channel" section of [send]).
     *
     * Like [send], [on_send] obeys the rules of prompt cancellation:
     * [select] may finish with a [CancellationException] even if the element
     * was successfully sent.
     *
     * Transliterated from:
     * public val onSend: SelectClause2<E, SendChannel<E>>
     */
    virtual selects::SelectClause2<E, SendChannel<E>*>& on_send() = 0;

    /**
     * Attempts to add the specified [element] to this channel without waiting.
     *
     * [try_send] never suspends and never throws exceptions.
     * Instead, it returns a [ChannelResult] that encapsulates the result of
     * the operation.
     * This makes it different from [send], which can suspend and throw.
     *
     * If this channel is currently full and cannot receive new elements at
     * the time or is [closed][close], this function returns a result that
     * indicates [a failure][ChannelResult::is_failure].
     * In this case, it is guaranteed that the element was not delivered to
     * the consumer and the `on_undelivered_element` callback, if one is
     * provided during the [Channel]'s construction, does *not* get called.
     *
     * [try_send] can be used as a non-`suspend` alternative to [send] in
     * cases where it's known beforehand that the channel's buffer cannot
     * overflow.
     *
     * ```cpp
     * struct Coordinates { int x, y; };
     * // A channel for a single subscriber that stores the latest update.
     * auto mouse_updates = create_channel<Coordinates>(CHANNEL_CONFLATED);
     * // Notifies the subscriber about the new mouse position.
     * void move_mouse(Coordinates coords) {
     *     auto result = mouse_updates->try_send(coords);
     *     if (result.is_closed()) {
     *         throw std::runtime_error("Mouse position no longer processed");
     *     }
     * }
     * ```
     *
     * Transliterated from:
     * public fun trySend(element: E): ChannelResult<Unit>
     */
    virtual ChannelResult<void> try_send(E element) = 0;

    /**
     * Closes this channel so that subsequent attempts to [send] to it fail.
     *
     * Returns `true` if the channel was not closed previously and the call
     * to this function closed it.
     * If the channel was already closed, this function does nothing and
     * returns `false`.
     *
     * The existing elements in the channel remain there, and likewise,
     * the calls to [send] and [on_send] that have suspended before [close]
     * was called will not be affected.
     * Only the subsequent calls to [send], [try_send], or [on_send] will fail.
     * [is_closed_for_send] will start returning `true` immediately after this
     * function is called.
     *
     * Once all the existing elements are received, the channel will be
     * considered closed for `receive` as well.
     * This means that [receive][ReceiveChannel::receive] will also start
     * throwing exceptions.
     * At that point, [is_closed_for_receive][ReceiveChannel::is_closed_for_receive]
     * will start returning `true`.
     *
     * If the [cause] is non-null, it will be thrown from all the subsequent
     * attempts to [send] to this channel, as well as from all the attempts
     * to [receive][ReceiveChannel::receive] from the channel after no elements
     * remain.
     *
     * If the [cause] is null, the channel is considered to have completed
     * normally. All subsequent calls to [send] will throw a
     * [ClosedSendChannelException], whereas calling
     * [receive][ReceiveChannel::receive] will throw a
     * [ClosedReceiveChannelException] after there are no more elements.
     *
     * ```cpp
     * auto channel = create_channel<int>();
     * channel->try_send(1);
     * channel->close();
     * try {
     *     channel->send(2, cont);
     *     throw std::logic_error("unreachable");
     * } catch (const ClosedSendChannelException& e) {
     *     // expected
     * }
     * ```
     *
     * @param cause Optional exception that caused the closure.
     * @return true if this call closed the channel.
     *
     * Transliterated from:
     * public fun close(cause: Throwable? = null): Boolean
     */
    virtual bool close(std::exception_ptr cause = nullptr) = 0;

    /**
     * Registers a [handler] that is synchronously invoked once the channel is
     * [closed][close] or the receiving side of this channel is
     * [cancelled][ReceiveChannel::cancel].
     * Only one handler can be attached to a channel during its lifetime.
     * The `handler` is invoked when [is_closed_for_send] starts to return `true`.
     * If the channel is closed already, the handler is invoked immediately.
     *
     * The meaning of `cause` that is passed to the handler:
     * - `nullptr` if the channel was [closed][close] normally with `cause = nullptr`.
     * - Instance of [CancellationException] if the channel was
     *   [cancelled][ReceiveChannel::cancel] normally without the corresponding
     *   argument.
     * - The cause of `close` or `cancel` otherwise.
     *
     * ### Execution context and exception safety
     *
     * The [handler] is executed as part of the closing or cancelling operation,
     * and only after the channel reaches its final state.
     * This means that if the handler throws an exception or hangs,
     * the channel will still be successfully closed or cancelled.
     * Unhandled exceptions from [handler] are propagated to the closing or
     * cancelling operation's caller.
     *
     * Example of usage:
     * ```cpp
     * auto events = create_channel<Event>(CHANNEL_UNLIMITED);
     * callback_api.register_callback([&events](Event event) {
     *     on_closed(events->try_send(event), [](auto) {
     *         // channel is already closed, but callback hasn't stopped yet
     *     });
     * });
     *
     * auto ui_updater = launch(Dispatchers::Main, [&events]() {
     *     events->consume([](Event e) { // handle events 
     *     });
     * });
     * // Stop the callback after the channel is closed or cancelled
     * events->invoke_on_close([&callback_api](auto) {
     *     callback_api.stop();
     * });
     * ```
     *
     * @throws std::logic_error if another handler was already registered.
     *
     * Transliterated from:
     * public fun invokeOnClose(handler: (cause: Throwable?) -> Unit)
     */
    virtual void invoke_on_close(std::function<void(std::exception_ptr)> handler) = 0;

    /**
     * **Deprecated** offer method.
     *
     * This method was deprecated in the favour of [try_send].
     * It has proven itself as the most error-prone method in Channel API:
     *
     * - `bool` return type creates the false sense of security, implying that
     *   `false` is returned instead of throwing an exception.
     * - It was used mostly from non-suspending APIs where CancellationException
     *   triggered internal failures in the application.
     * - Due to signature and explicit `if (ch.offer(...))` checks it was easy
     *   to oversee such error during code review.
     * - Its name was not aligned with the rest of the API.
     *
     * Transliterated from:
     * @Deprecated public fun offer(element: E): Boolean
     */
    [[deprecated("Deprecated in favor of 'try_send' method")]]
    bool offer(E element) {
        auto result = try_send(std::move(element));
        if (result.is_success()) return true;
        auto ex = result.exception_or_null();
        if (ex) std::rethrow_exception(ex);
        return false;
    }
};

// =============================================================================
// ReceiveChannel Interface
// =============================================================================

/**
 * Receiver's interface to a [Channel].
 *
 * Combined, [SendChannel] and [ReceiveChannel] define the complete [Channel]
 * interface.
 *
 * Transliterated from:
 * public interface ReceiveChannel<out E>
 */
template <typename E>
class ReceiveChannel {
public:
    virtual ~ReceiveChannel() = default;

    /**
     * Returns `true` if the sending side of this channel was
     * [closed][SendChannel::close] and all previously sent items were already
     * received (which also happens for [cancelled][cancel] channels).
     *
     * Note that if this property returns `false`, it does not guarantee that
     * a subsequent call to [receive] will succeed, as the channel can be
     * concurrently cancelled or closed right after the check.
     * For such scenarios, [receive_catching] is the more robust solution:
     * if the channel is closed, instead of throwing an exception,
     * [receive_catching] returns a result that allows querying it.
     *
     * ```cpp
     * // DANGER! THIS CHECK IS NOT RELIABLE!
     * if (!channel->is_closed_for_receive()) {
     *     channel->receive(cont); // can still fail!
     * } else {
     *     std::cout << "Cannot receive: the channel is closed\n";
     * }
     * // DO THIS INSTEAD:
     * on_closed(channel->receive_catching(cont), [](auto) {
     *     std::cout << "Cannot receive: the channel is closed\n";
     * });
     * ```
     *
     * The primary intended usage of this property is for assertions and
     * diagnostics to verify the expected state of the channel.
     * Using it in production code is discouraged.
     *
     * @DelicateCoroutinesApi
     *
     * Transliterated from:
     * public val isClosedForReceive: Boolean
     */
    virtual bool is_closed_for_receive() const = 0;

    /**
     * Returns `true` if the channel contains no elements and isn't
     * [closed for receive][is_closed_for_receive].
     *
     * If [is_empty] returns `true`, it means that calling [receive] at exactly
     * the same moment would suspend.
     * However, calling [receive] immediately after checking [is_empty] may or
     * may not suspend, as new elements could have been added or removed or
     * the channel could have been closed for receive between the two
     * invocations.
     * Consider using [try_receive] in cases when suspensions are undesirable:
     *
     * ```cpp
     * // DANGER! THIS CHECK IS NOT RELIABLE!
     * while (!channel->is_empty()) {
     *     // can still suspend if other `receive` happens in parallel!
     *     auto element = channel->receive(cont);
     *     std::cout << element << "\n";
     * }
     * // DO THIS INSTEAD:
     * while (true) {
     *     auto* element = channel->try_receive().get_or_null();
     *     if (!element) break;
     *     std::cout << *element << "\n";
     * }
     * ```
     *
     * @ExperimentalCoroutinesApi
     *
     * Transliterated from:
     * public val isEmpty: Boolean
     */
    virtual bool is_empty() const = 0;

    /**
     * Retrieves an element, removing it from the channel.
     *
     * This function suspends if the channel is empty, waiting until an
     * element is available.
     * If the channel is [closed for receive][is_closed_for_receive], an
     * exception is thrown (see below).
     *
     * ```cpp
     * auto channel = create_channel<int>();
     * launch([&channel]() {
     *     auto element = channel->receive(cont); // suspends until 5 available
     *     assert(element == 5);
     * });
     * channel->send(5, cont);
     * ```
     *
     * ## Suspending and cancellation
     *
     * This suspending function is cancellable: if the [Job] of the current
     * coroutine is cancelled while this suspending function is waiting,
     * this function immediately resumes with [CancellationException].
     * There is a **prompt cancellation guarantee**: even if [receive] managed
     * to retrieve the element from the channel, but was cancelled while
     * suspended, [CancellationException] will be thrown, and, if the channel
     * has an `on_undelivered_element` callback installed, the retrieved
     * element will be passed to it.
     *
     * Note that this function does not check for cancellation when it manages
     * to immediately receive an element without suspending.
     * Use [ensure_active] or [CoroutineScope::is_active] to periodically check
     * for cancellation in tight loops if needed.
     *
     * ## Receiving from a closed channel
     *
     * - Attempting to [receive] from a [closed][SendChannel::close] channel
     *   while there are still some elements will successfully retrieve an
     *   element from the channel.
     * - When a channel is [closed][SendChannel::close] and there are no
     *   elements remaining, the channel becomes [closed for receive].
     *   After that, [receive] will rethrow the same exception that was passed
     *   to [SendChannel::close], or [ClosedReceiveChannelException] if none
     *   was given.
     *
     * ## Related
     *
     * This function can be used in [select] invocations with the [on_receive]
     * clause.
     * Use [try_receive] to try receiving from this channel without waiting
     * and throwing.
     * Use [receive_catching] to receive from this channel without throwing.
     *
     * @param continuation The continuation to resume when element available.
     * @return COROUTINE_SUSPENDED or boxed element.
     *
     * Transliterated from:
     * public suspend fun receive(): E
     */
    virtual void* receive(Continuation<void*>* continuation) = 0;

    /**
     * Clause for the [select] expression of the [receive] suspending function
     * that selects with the element received from the channel.
     *
     * The [select] invocation fails with an exception if the channel
     * [is closed for receive][is_closed_for_receive] at any point, even if
     * other [select] clauses could still work.
     *
     * Example:
     * ```cpp
     * struct ScreenSize { int width, height; };
     * struct MouseClick { int x, y; };
     * auto screen_resizes = create_channel<ScreenSize>(CHANNEL_CONFLATED);
     * auto mouse_clicks = create_channel<MouseClick>(CHANNEL_CONFLATED);
     *
     * launch(Dispatchers::Main, [&]() {
     *     while (true) {
     *         select([&](auto& selector) {
     *             screen_resizes->on_receive().invoke(selector, [](ScreenSize s) {
     *                 // update the UI to the new screen size
     *             });
     *             mouse_clicks->on_receive().invoke(selector, [](MouseClick c) {
     *                 // react to a mouse click
     *             });
     *         });
     *     }
     * });
     * ```
     *
     * Like [receive], [on_receive] obeys the rules of prompt cancellation:
     * [select] may finish with a [CancellationException] even if an element
     * was successfully retrieved, in which case the `on_undelivered_element`
     * callback will be called.
     *
     * Transliterated from:
     * public val onReceive: SelectClause1<E>
     */
    virtual selects::SelectClause1<E>& on_receive() = 0;

    /**
     * Retrieves an element, removing it from the channel.
     *
     * A difference from [receive] is that this function encapsulates a failure
     * in its return value instead of throwing an exception.
     * However, it will still throw [CancellationException] if the coroutine
     * calling [receive_catching] is cancelled.
     *
     * It is guaranteed that the only way this function can return a
     * [failed][ChannelResult::is_failure] result is when the channel is
     * [closed for receive][is_closed_for_receive], so
     * [ChannelResult::is_closed] is also true.
     *
     * This function suspends if the channel is empty, waiting until an element
     * is available or the channel becomes closed.
     *
     * ```cpp
     * auto channel = create_channel<int>();
     * launch([&channel]() {
     *     while (true) {
     *         auto result = channel->receive_catching(cont); // suspends
     *         auto* element = result.get_or_null();
     *         if (!element) break; // the channel is closed
     *         assert(*element == 5);
     *     }
     * });
     * channel->try_send(5);
     * ```
     *
     * ## Suspending and cancellation
     *
     * This suspending function is cancellable.
     * There is a **prompt cancellation guarantee**: even if [receive_catching]
     * managed to retrieve the element from the channel, but was cancelled
     * while suspended, [CancellationException] will be thrown, and, if the
     * channel has an `on_undelivered_element` callback installed, the retrieved
     * element will be passed to it.
     *
     * ## Receiving from a closed channel
     *
     * - Attempting to [receive_catching] from a [closed][SendChannel::close]
     *   channel while there are still some elements will successfully retrieve
     *   an element from the channel.
     * - When a channel is [closed][SendChannel::close] and there are no
     *   elements remaining, the channel becomes [closed for receive].
     *   After that, [receive_catching] will return a result with
     *   [ChannelResult::is_closed] set.
     *   [ChannelResult::exception_or_null] will be the exact exception that
     *   was passed to [SendChannel::close], or `nullptr` if none was given.
     *
     * ## Related
     *
     * This function can be used in [select] invocations with the
     * [on_receive_catching] clause.
     * Use [try_receive] to try receiving from this channel without waiting
     * and throwing.
     * Use [receive] to receive from this channel and throw exceptions on error.
     *
     * @param continuation The continuation to resume when complete.
     * @return COROUTINE_SUSPENDED or boxed ChannelResult<E>.
     *
     * Transliterated from:
     * public suspend fun receiveCatching(): ChannelResult<E>
     */
    virtual void* receive_catching(Continuation<void*>* continuation) = 0;

    /**
     * Clause for the [select] expression of the [receive_catching] suspending
     * function that selects with a [ChannelResult] when an element is retrieved
     * or the channel gets closed.
     *
     * Like [receive_catching], [on_receive_catching] obeys the rules of prompt
     * cancellation: [select] may finish with a [CancellationException] even if
     * an element was successfully retrieved, in which case the
     * `on_undelivered_element` callback will be called.
     *
     * Transliterated from:
     * public val onReceiveCatching: SelectClause1<ChannelResult<E>>
     */
    virtual selects::SelectClause1<ChannelResult<E>>& on_receive_catching() = 0;

    /**
     * Attempts to retrieve an element without waiting, removing it from
     * the channel.
     *
     * - When the channel is non-empty, a [successful][ChannelResult::is_success]
     *   result is returned, and [ChannelResult::get_or_null] returns the
     *   retrieved element.
     * - When the channel is empty, a [failed][ChannelResult::is_failure] result
     *   is returned.
     * - When the channel is already [closed for receive][is_closed_for_receive],
     *   returns the ["channel is closed"][ChannelResult::is_closed] result.
     *   If the channel was [closed][SendChannel::close] with a cause (for
     *   example, [cancelled][cancel]), [ChannelResult::exception_or_null]
     *   contains the cause.
     *
     * This function is useful when implementing on-demand allocation of
     * resources to be stored in the channel:
     *
     * ```cpp
     * auto resource_pool = create_channel<Resource>(max_resources);
     *
     * template<typename Block>
     * void with_resource(Block&& block) {
     *     auto result = resource_pool->try_receive();
     *     Resource* resource = result.get_or_null();
     *     if (!resource) {
     *         resource = try_create_new_resource();
     *     }
     *     if (!resource) {
     *         resource = resource_pool->receive(cont); // actually wait
     *     }
     *     try {
     *         block(*resource);
     *     } catch (...) {
     *         resource_pool->try_send(*resource);
     *         throw;
     *     }
     *     resource_pool->try_send(*resource);
     * }
     * ```
     *
     * Transliterated from:
     * public fun tryReceive(): ChannelResult<E>
     */
    virtual ChannelResult<E> try_receive() = 0;

    /**
     * Returns a new iterator to receive elements from this channel using
     * a `for` loop.
     * Iteration completes normally when the channel
     * [is closed for receive][is_closed_for_receive] without a cause and
     * throws the exception passed to [close][SendChannel::close] if there
     * was one.
     *
     * Instances of [ChannelIterator] are not thread-safe and shall not be
     * used from concurrent coroutines.
     *
     * Example:
     * ```cpp
     * auto channel = produce<int>([](auto& scope) {
     *     for (int i = 0; i < 1000; ++i) {
     *         scope.send(i);
     *     }
     * });
     * auto iter = channel->iterator();
     * while (iter->has_next(cont)) {
     *     std::cout << iter->next() << "\n";
     * }
     * ```
     *
     * Note that if an early return happens from the loop, the channel does
     * not get cancelled.
     * To forbid sending new elements after the iteration is completed, use
     * [consume_each] or call [cancel] manually.
     *
     * Transliterated from:
     * public operator fun iterator(): ChannelIterator<E>
     */
    virtual std::unique_ptr<ChannelIterator<E>> iterator() = 0;

    /**
     * [Closes][SendChannel::close] the channel for new elements and removes
     * all existing ones.
     *
     * A [cause] can be used to specify an error message or to provide other
     * details on the cancellation reason for debugging purposes.
     * If the cause is not specified, then an instance of [CancellationException]
     * with a default message is created to [close][SendChannel::close] the
     * channel.
     *
     * If the channel was already [closed][SendChannel::close], [cancel] only
     * has the effect of removing all elements from the channel.
     *
     * Immediately after the invocation of this function,
     * [is_closed_for_receive] and, on the [SendChannel] side,
     * [is_closed_for_send][SendChannel::is_closed_for_send] start returning
     * `true`.
     * Any attempt to send to or receive from this channel will lead to a
     * [CancellationException].
     * This also applies to the existing senders and receivers that are
     * suspended at the time of the call: they will be resumed with a
     * [CancellationException] immediately after [cancel] is called.
     *
     * If the channel has an `on_undelivered_element` callback installed,
     * this function will invoke it for each of the elements still in the
     * channel, since these elements will be inaccessible otherwise.
     * If the callback is not installed, these elements will simply be removed
     * from the channel for garbage collection.
     *
     * ```cpp
     * auto channel = create_channel<int>();
     * channel->try_send(1);
     * channel->try_send(2);
     * channel->cancel();
     * channel->try_send(3); // returns ChannelResult::is_closed
     * // for (auto element : channel) -> prints nothing
     * ```
     *
     * [consume] and [consume_each] are convenient shorthands for cancelling
     * the channel after the single consumer has finished processing.
     *
     * Transliterated from:
     * public fun cancel(cause: CancellationException? = null)
     */
    virtual void cancel(std::exception_ptr cause = nullptr) = 0;

    /**
     * Binary compatibility overload.
     *
     * Transliterated from:
     * @Deprecated fun cancel(): Unit
     */
    void cancel() { cancel(nullptr); }

    /**
     * **Deprecated** poll method.
     *
     * This method was deprecated in the favour of [try_receive].
     * It has proven itself as error-prone method in Channel API:
     *
     * - Nullable return type creates the false sense of security, implying
     *   that `nullptr` is returned instead of throwing an exception.
     * - It was used mostly from non-suspending APIs where CancellationException
     *   triggered internal failures in the application.
     * - Its name was not aligned with the rest of the API.
     *
     * ### Replacement note
     *
     * The replacement `try_receive().get_or_null()` is a default that ignores
     * all close exceptions and proceeds with `nullptr`, while `poll` throws
     * an exception if the channel was closed with an exception.
     *
     * Transliterated from:
     * @Deprecated public fun poll(): E?
     */
    [[deprecated("Deprecated in favor of 'try_receive'")]]
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
// Channel Interface
// =============================================================================

/**
 * Channel is a non-blocking primitive for communication between a sender
 * (via [SendChannel]) and a receiver (via [ReceiveChannel]).
 * Conceptually, a channel is similar to `std::deque` or a blocking queue,
 * but it has suspending operations instead of blocking ones and can be
 * [closed][SendChannel::close].
 *
 * ### Channel capacity
 *
 * Most ways to create a [Channel] (in particular, the `create_channel()`
 * factory function) allow specifying a capacity, which determines how
 * elements are buffered in the channel.
 * There are several predefined constants for the capacity that have special
 * behavior:
 *
 * - [Channel::RENDEZVOUS] (or 0) creates a _rendezvous_ channel, which does
 *   not have a buffer at all.
 *   Instead, the sender and the receiver must rendezvous (meet):
 *   [SendChannel::send] suspends until another coroutine invokes
 *   [ReceiveChannel::receive], and vice versa.
 * - [Channel::CONFLATED] creates a buffer for a single element and
 *   automatically changes the [buffer overflow strategy][BufferOverflow] to
 *   [BufferOverflow::DROP_OLDEST].
 * - [Channel::UNLIMITED] creates a channel with an unlimited buffer, which
 *   never suspends the sender.
 * - [Channel::BUFFERED] creates a channel with a buffer whose size depends
 *   on the [buffer overflow strategy][BufferOverflow].
 *
 * See each constant's documentation for more details.
 *
 * If the capacity is positive but less than [Channel::UNLIMITED], the channel
 * has a buffer with the specified capacity.
 * It is safe to construct a channel with a large buffer, as memory is only
 * allocated gradually as elements are added.
 *
 * Constructing a channel with a negative capacity not equal to a predefined
 * constant is not allowed and throws an exception.
 *
 * ### Buffer overflow
 *
 * Some ways to create a [Channel] also expose a [BufferOverflow] parameter
 * (by convention, `on_buffer_overflow`), which does not affect the receiver
 * but determines the behavior of the sender when the buffer is full.
 * The options include [suspending][BufferOverflow::SUSPEND] until there is
 * space in the buffer, [dropping the oldest element][BufferOverflow::DROP_OLDEST]
 * to make room for the new one, or
 * [dropping the element to be sent][BufferOverflow::DROP_LATEST].
 * See the [BufferOverflow] documentation.
 *
 * By convention, the default value for [BufferOverflow] whenever it cannot
 * be configured is [BufferOverflow::SUSPEND].
 *
 * See the [Channel::RENDEZVOUS], [Channel::CONFLATED], and [Channel::UNLIMITED]
 * documentation for a description of how they interact with the
 * [BufferOverflow] parameter.
 *
 * ### Prompt cancellation guarantee
 *
 * All suspending functions with channels provide **prompt cancellation
 * guarantee**.
 * If the job was cancelled while send or receive function was suspended,
 * it will not resume successfully, even if it already changed the channel's
 * state, but throws a [CancellationException].
 * With a single-threaded [dispatcher][CoroutineDispatcher] like
 * [Dispatchers::Main], this gives a guarantee that the coroutine promptly
 * reacts to the cancellation of its [Job] and does not resume its execution.
 *
 * > **Prompt cancellation guarantee** for channel operations was added in
 * > `kotlinx.coroutines` version `1.4.0` and has replaced the channel-specific
 * > atomic cancellation that was not consistent with other suspending functions.
 * > The low-level mechanics of prompt cancellation are explained in the
 * > [suspend_cancellable_coroutine] documentation.
 *
 * ### Undelivered elements
 *
 * As a result of the prompt cancellation guarantee, when a closeable resource
 * (like an open file or a handle to another native resource) is transferred
 * via a channel, it can be successfully extracted from the channel, but still
 * be lost if the receiving operation is cancelled in parallel.
 *
 * The `create_channel()` factory function has the optional parameter
 * `on_undelivered_element`.
 * When that parameter is set, the corresponding function is called once for
 * each element that was sent to the channel with the call to the
 * [send][SendChannel::send] function but failed to be delivered, which can
 * happen in the following cases:
 *
 * - When an element is dropped due to the limited buffer capacity.
 *   This can happen when the overflow strategy is [BufferOverflow::DROP_LATEST]
 *   or [BufferOverflow::DROP_OLDEST].
 * - When the sending operations like [send][SendChannel::send] or
 *   [on_send][SendChannel::on_send] throw an exception because it was
 *   cancelled before it had a chance to actually send the element or because
 *   the channel was [closed][SendChannel::close] or
 *   [cancelled][ReceiveChannel::cancel].
 * - When the receiving operations like [receive][ReceiveChannel::receive],
 *   [on_receive][ReceiveChannel::on_receive], or
 *   [has_next][ChannelIterator::has_next] throw an exception after retrieving
 *   the element from the channel because of being cancelled before the code
 *   following them had a chance to resume.
 * - When the channel was [cancelled][ReceiveChannel::cancel], in which case
 *   `on_undelivered_element` is called on every remaining element in the
 *   channel's buffer.
 *
 * Note that `on_undelivered_element` is called synchronously in an arbitrary
 * context.
 * It should be fast, non-blocking, and should not throw exceptions.
 * Any exception thrown by `on_undelivered_element` is wrapped into an internal
 * runtime exception which is either rethrown from the caller method or handed
 * off to the exception handler in the current context.
 *
 * A typical usage for `on_undelivered_element` is to close a resource that is
 * being transferred via the channel.
 * The following code pattern guarantees that opened resources are closed even
 * if the producer, the consumer, and/or the channel are cancelled.
 * Resources are never lost.
 *
 * ```cpp
 * // Create a channel with an on_undelivered_element block that closes a resource
 * auto channel = create_channel<Resource>(
 *     CHANNEL_RENDEZVOUS,
 *     BufferOverflow::SUSPEND,
 *     [](Resource& resource) { resource.close(); }
 * );
 *
 * // Producer code
 * auto resource_to_send = open_resource();
 * channel->send(std::move(resource_to_send), cont);
 *
 * // Consumer code
 * Resource resource_received = channel->receive(cont);
 * try {
 *     // work with received resource
 * } finally {
 *     resource_received.close();
 * }
 * ```
 *
 * > Note that if any work happens between `open_resource()` and
 * > `channel->send(...)`, it is your responsibility to ensure that resource
 * > gets closed in case this additional code fails.
 *
 * Transliterated from:
 * public interface Channel<E> : SendChannel<E>, ReceiveChannel<E>
 */
template <typename E>
class Channel : public virtual SendChannel<E>, public virtual ReceiveChannel<E> {
public:
    /**
     * Constants for the channel factory function `create_channel()`.
     *
     * Transliterated from:
     * public companion object Factory
     */

    /**
     * An unlimited buffer capacity.
     *
     * `create_channel<E>(UNLIMITED)` creates a channel with an unlimited
     * buffer, which never suspends the sender.
     * The total amount of elements that can be sent to the channel is limited
     * only by the available memory.
     *
     * If [BufferOverflow] is specified for the channel, it is completely
     * ignored, as the channel never suspends the sender.
     *
     * Transliterated from:
     * public const val UNLIMITED: Int = Int.MAX_VALUE
     */
    static constexpr int UNLIMITED = 2147483647; // Int.MAX_VALUE

    /**
     * The zero buffer capacity (rendezvous channel).
     *
     * For the default [BufferOverflow] value of [BufferOverflow::SUSPEND],
     * `create_channel<E>(RENDEZVOUS)` creates a channel without a buffer.
     * An element is transferred from the sender to the receiver only when
     * [send] and [receive] invocations meet in time (that is, they _rendezvous_),
     * so [send] suspends until another coroutine invokes [receive],
     * and [receive] suspends until another coroutine invokes [send].
     *
     * If a different [BufferOverflow] is specified,
     * `create_channel<E>(RENDEZVOUS)` creates a channel with a buffer of size 1.
     *
     * Transliterated from:
     * public const val RENDEZVOUS: Int = 0
     */
    static constexpr int RENDEZVOUS = 0;

    /**
     * A single-element buffer with conflating behavior.
     *
     * Specifying [CONFLATED] as the capacity in the `create_channel()`
     * function is equivalent to creating a channel with a buffer of size 1
     * and a [BufferOverflow] strategy of [BufferOverflow::DROP_OLDEST]:
     * `create_channel<E>(1, BufferOverflow::DROP_OLDEST)`.
     *
     * Such a channel buffers at most one element and conflates all subsequent
     * `send` and `try_send` invocations so that the receiver always gets the
     * last element sent, **losing** the previously sent elements.
     * See the "Undelivered elements" section above.
     *
     * [Sending][send] to this channel never suspends, and [try_send] always
     * succeeds.
     *
     * Specifying a [BufferOverflow] other than [BufferOverflow::SUSPEND] is
     * not allowed with [CONFLATED], and an exception is thrown if such a
     * combination is used.
     *
     * Transliterated from:
     * public const val CONFLATED: Int = -1
     */
    static constexpr int CONFLATED = -1;

    /**
     * A channel capacity marker that is substituted by the default buffer
     * capacity.
     *
     * When passed as a parameter to the `create_channel()` function, the
     * default buffer capacity is used.
     * For [BufferOverflow::SUSPEND] (the default buffer overflow strategy),
     * the default capacity is 64.
     *
     * If a different [BufferOverflow] is specified,
     * `create_channel<E>(BUFFERED)` creates a channel with a buffer of size 1.
     *
     * Transliterated from:
     * public const val BUFFERED: Int = -2
     */
    static constexpr int BUFFERED = -2;

    /**
     * Internal marker for optional channel (used in flow operators).
     */
    static constexpr int OPTIONAL_CHANNEL = -3;

    /**
     * Name of the system property for the default channel capacity
     * (64 by default).
     *
     * Setting this property affects the default channel capacity for channel
     * constructors, channel-backed coroutines and flow operators that imply
     * channel usage, including ones defined in 3rd-party libraries.
     *
     * Usage of this property is highly discouraged and is intended to be used
     * as a last-ditch effort as an immediate measure for hot fixes and
     * duct-taping.
     *
     * @DelicateCoroutinesApi
     *
     * Transliterated from:
     * public const val DEFAULT_BUFFER_PROPERTY_NAME: String
     */
    static constexpr const char* DEFAULT_BUFFER_PROPERTY_NAME =
        "kotlinx.coroutines.channels.defaultBuffer";

    /**
     * The default channel capacity (64 by default).
     *
     * Transliterated from:
     * internal val CHANNEL_DEFAULT_CAPACITY
     */
    static int channel_default_capacity() {
        static int value = internal::system_prop_int(
            DEFAULT_BUFFER_PROPERTY_NAME, 64, 1, UNLIMITED - 1);
        return value;
    }
};

// =============================================================================
// Channel Factory Function
// =============================================================================

/**
 * Creates a channel. See the [Channel] interface documentation for details.
 *
 * This function is the most flexible way to create a channel.
 * It allows specifying the channel's capacity, buffer overflow strategy, and
 * an optional function to call to handle undelivered elements.
 *
 * ```cpp
 * std::unordered_set<int> allocated_resources;
 * // An autocloseable resource that must be closed when no longer needed
 * class Resource {
 * public:
 *     int id;
 *     Resource(int id) : id(id) { allocated_resources.insert(id); }
 *     void close() { allocated_resources.erase(id); }
 * };
 *
 * // A channel with a 15-element buffer that drops the oldest element on
 * // buffer overflow and closes the elements that were not delivered to
 * // the consumer
 * auto channel = create_channel<Resource>(
 *     15,
 *     BufferOverflow::DROP_OLDEST,
 *     [](Resource& element) { element.close(); }
 * );
 *
 * // A sender's view of the channel
 * SendChannel<Resource>* send_channel = channel.get();
 * for (int i = 0; i < 100; ++i) {
 *     send_channel->try_send(Resource(i));
 * }
 * send_channel->close();
 *
 * // A receiver's view of the channel
 * ReceiveChannel<Resource>* receive_channel = channel.get();
 * auto received_resources = to_list(receive_channel);
 * // Check that the last 15 sent resources were received
 * assert(received_resources.size() == 15);
 *
 * // Close the resources that were successfully received
 * for (auto& r : received_resources) {
 *     r.close();
 * }
 * // The dropped resources were closed by the channel itself
 * assert(allocated_resources.empty());
 * ```
 *
 * For a full explanation of every parameter and their interaction, see the
 * [Channel] interface documentation.
 *
 * @param capacity either a positive channel capacity or one of the constants
 *        defined in [Channel::Factory].
 *        See the "Channel capacity" section in the [Channel] documentation.
 * @param on_buffer_overflow configures an action on buffer overflow.
 *        See the "Buffer overflow" section in the [Channel] documentation.
 * @param on_undelivered_element a function that is called when element was
 *        sent but was not delivered to the consumer.
 *        See the "Undelivered elements" section in the [Channel] documentation.
 * @throws std::invalid_argument when [capacity] < -2
 *
 * Transliterated from:
 * public fun <E> Channel(
 *     capacity: Int = RENDEZVOUS,
 *     onBufferOverflow: BufferOverflow = BufferOverflow.SUSPEND,
 *     onUndeliveredElement: ((E) -> Unit)? = null
 * ): Channel<E>
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
