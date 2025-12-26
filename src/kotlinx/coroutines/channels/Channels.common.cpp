/**
 * @file Channels.common.cpp
 * @brief Common channel operations and utilities
 *
 * Transliterated from: kotlinx-coroutines-core/common/src/channels/Channels.common.kt
 *
 * This file provides common channel operations that are platform-independent.
 * The main implementations are in Channels.hpp as templates.
 */

#include "kotlinx/coroutines/channels/Channels.hpp"
#include "kotlinx/coroutines/CoroutineScope.hpp"
#include "kotlinx/coroutines/Exceptions.hpp"

namespace kotlinx {
namespace coroutines {
namespace channels {

/**
 * Default message for channel close operations.
 *
 * Transliterated from:
 * internal const val DEFAULT_CLOSE_MESSAGE = "Channel was closed"
 */
// Note: DEFAULT_CLOSE_MESSAGE is defined in Channels.hpp

/**
 * Internal function to cancel a channel after consumption.
 *
 * Transliterated from:
 * @PublishedApi
 * internal fun ReceiveChannel<*>.cancelConsumed(cause: Throwable?) {
 *     cancel(cause?.let {
 *         it as? CancellationException ?: CancellationException("Channel was consumed, consumer had failed", it)
 *     })
 * }
 */
// Note: cancel_consumed<E> is implemented as a template in Channels.hpp

/**
 * Executes the block and then cancels the channel.
 *
 * It is guaranteed that, after invoking this operation, the channel will be cancelled,
 * so the operation is _terminal_.
 * If the block finishes with an exception, that exception will be used for cancelling
 * the channel and rethrown.
 *
 * This function is useful for building more complex terminal operators while ensuring
 * that the producers stop sending new elements to the channel.
 *
 * consume() does not guarantee that new elements will not enter the channel after
 * block finishes executing, so some channel elements may be lost.
 * Use the on_undelivered_element parameter of a manually created Channel to define what
 * should happen with these elements during cancel().
 *
 * Transliterated from:
 * public inline fun <E, R> ReceiveChannel<E>.consume(block: ReceiveChannel<E>.() -> R): R
 */
// Note: consume<E, R> is implemented as a template in Channels.hpp

/**
 * Performs the given action for each received element and cancels the channel afterward.
 *
 * This function stops processing elements when either:
 * - The channel is closed
 * - The coroutine in which the collection is performed gets cancelled
 *   and there are no readily available elements in the channel's buffer
 * - action fails with an exception
 * - An early return from action happens
 *
 * If the action finishes with an exception, that exception will be used for
 * cancelling the channel and rethrown. If the channel is closed with a cause,
 * this cause will be rethrown from consume_each.
 *
 * When the channel does not need to be closed after iterating over its elements,
 * a regular for loop should be used instead.
 *
 * The operation is _terminal_.
 * This function consumes the elements of the original ReceiveChannel.
 *
 * This function is useful in cases when this channel is only expected to have
 * a single consumer that decides when the producer may stop.
 *
 * **Pitfall**: even though the name says "each", some elements could be left
 * unprocessed if they are added after this function decided to close the channel.
 * In this case, the elements will simply be lost. If the elements of the channel
 * are resources that must be closed (like file handles, sockets, etc.),
 * an on_undelivered_element must be passed to the Channel on construction.
 * It will be called for each element left in the channel at the point of cancellation.
 *
 * Transliterated from:
 * public suspend inline fun <E> ReceiveChannel<E>.consumeEach(action: (E) -> Unit): Unit =
 *     consume { for (e in this) action(e) }
 */
// Note: consume_each<E> is implemented as a template in Channels.hpp

/**
 * Returns a List containing all the elements sent to this channel, preserving their order.
 *
 * This function will attempt to receive elements and put them into the list until the
 * channel is closed.
 *
 * Calling to_list on channels that are not eventually closed is always incorrect:
 * - It will suspend indefinitely if the channel is not closed, but no new elements arrive.
 * - If new elements do arrive and the channel is not eventually closed, to_list will
 *   use more and more memory until exhausting it.
 *
 * If the channel is closed with a cause, to_list will rethrow that cause.
 *
 * The operation is _terminal_.
 * This function consumes all elements of the original ReceiveChannel.
 *
 * Transliterated from:
 * public suspend fun <E> ReceiveChannel<E>.toList(): List<E> = buildList {
 *     consumeEach { add(it) }
 * }
 */
// Note: to_list<E> is implemented as a template in Channels.hpp

} // namespace channels
} // namespace coroutines
} // namespace kotlinx
