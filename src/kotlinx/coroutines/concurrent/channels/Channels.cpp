/**
 * Transliterated from: kotlinx-coroutines-core/concurrent/src/channels/Channels.kt
 *
 * Kotlin file header (translated):
 *   package kotlinx.coroutines.channels
 *
 * Upstream:
 *   public fun <E> SendChannel<E>.trySendBlocking(element: E): ChannelResult<Unit> =
 *       trySend(element).onClosed { ... }.onFailure {
 *           runBlocking { sendCatching(element) }
 *       }
 *
 * Blocking send/receive entry points for callback APIs that cannot use suspend
 * functions. The C++ port routes the fallback through `run_blocking([&] { ... })`, which
 * drives the suspend body to completion on the calling thread.
 */

#include "kotlinx/coroutines/Builders.hpp"
#include "kotlinx/coroutines/channels/Channel.hpp"

#include <utility>

namespace kotlinx::coroutines::channels {

/**
 * Upstream:
 *   public fun <E> SendChannel<E>.trySendBlocking(element: E): ChannelResult<Unit>
 */
template <typename E>
ChannelResult<Unit> try_send_blocking(SendChannel<E>* channel, E element) {
    auto immediate = channel->try_send(element);
    if (immediate.is_success() || immediate.is_closed()) {
        return immediate;
    }
    return run_blocking([channel, element = std::move(element)]() mutable {
        return channel->send_catching(std::move(element), nullptr);
    });
}

} // namespace kotlinx::coroutines::channels
