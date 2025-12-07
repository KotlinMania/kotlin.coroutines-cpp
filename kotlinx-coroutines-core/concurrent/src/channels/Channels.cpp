// Transliterated from Kotlin to C++
// Original: kotlinx-coroutines-core/concurrent/src/channels/Channels.kt
//
// TODO: Map @file:JvmMultifileClass and @file:JvmName annotations (JVM-specific, likely ignore in C++)
// TODO: Implement SendChannel<E> template class
// TODO: Implement ChannelResult<T> template class with isSuccess, success(), closed() methods
// TODO: Implement runBlocking coroutine builder
// TODO: Implement runCatching utility (Result type)
// TODO: Handle suspend function semantics for send()
// TODO: Map @Deprecated annotation to [[deprecated]] or comments
// TODO: Map DeprecationLevel.HIDDEN to appropriate C++ deprecation mechanism
// TODO: kotlin.jvm.* imports are JVM-specific, can be ignored

// @file:JvmMultifileClass
// @file:JvmName("ChannelsKt")

namespace kotlinx {
namespace coroutines {
namespace channels {

/**
 * Adds [element] to this channel, **blocking** the caller while this channel is full,
 * and returning either [successful][ChannelResult.isSuccess] result when the element was added, or
 * failed result representing closed channel with a corresponding exception.
 *
 * This is a way to call [Channel.send] method in a safe manner inside a blocking code using [runBlocking] and catching,
 * so this function should not be used from coroutine.
 *
 * Example of usage:
 *
 * ```
 * // From callback API
 * channel.trySendBlocking(element)
 *     .onSuccess { /* request next element or debug log */ }
 *     .onFailure { t: Throwable? -> /* throw or log */ }
 * ```
 *
 * For this operation it is guaranteed that [failure][ChannelResult.failed] always contains an exception in it.
 *
 * @throws `InterruptedException` on JVM if the current thread is interrupted during the blocking send operation.
 */
template<typename E>
ChannelResult<void> try_send_blocking(SendChannel<E>* channel, const E& element) {
    /*
     * Sent successfully -- bail out.
     * But failure may indicate either that the channel is full or that
     * it is close. Go to slow path on failure to simplify the successful path and
     * to materialize default exception.
     */
    auto result = channel->try_send(element);
    if (result.is_success()) {
        return ChannelResult<void>::success();
    }
    // TODO: runBlocking is a coroutine builder that blocks the current thread
    // TODO: Implement runCatching and Result<T> type
    return run_blocking([&]() {
        // TODO: suspend function call to send()
        auto r = run_catching([&]() { channel->send(element); });
        if (r.is_success()) {
            return ChannelResult<void>::success();
        } else {
            return ChannelResult<void>::closed(r.exception_or_null());
        }
    });
}

/** @suppress */
// @Deprecated(
//     level = DeprecationLevel.HIDDEN,
//     message = "Deprecated in the favour of 'trySendBlocking'. " +
//         "Consider handling the result of 'trySendBlocking' explicitly and rethrow exception if necessary",
//     replaceWith = ReplaceWith("trySendBlocking(element)")
// ) // WARNING in 1.5.0, ERROR in 1.6.0
template<typename E>
[[deprecated("Deprecated in the favour of 'trySendBlocking'. Consider handling the result of 'trySendBlocking' explicitly and rethrow exception if necessary")]]
void send_blocking(SendChannel<E>* channel, const E& element) {
    // fast path
    if (channel->try_send(element).is_success()) {
        return;
    }
    // slow path
    // TODO: runBlocking is a coroutine builder that blocks the current thread
    run_blocking([&]() {
        // TODO: suspend function call
        channel->send(element);
    });
}

} // namespace channels
} // namespace coroutines
} // namespace kotlinx
