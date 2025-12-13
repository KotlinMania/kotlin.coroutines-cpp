#pragma once
/**
 * @file Channels.hpp
 * @brief Flow-Channel integration utilities.
 *
 * Transliterated from: kotlinx-coroutines-core/common/src/flow/Channels.kt
 *
 * NOTE: This header intentionally contains templates so they can be instantiated
 *       for any T (the previous `.cpp`-only templates only worked for explicitly
 *       instantiated types).
 *
 * TODO(semantics): This currently uses blocking/busy-wait in places where Kotlin
 *                  would suspend (e.g., waiting for channel elements).
 */

#include "kotlinx/coroutines/Continuation.hpp"
#include "kotlinx/coroutines/channels/Channel.hpp"
#include "kotlinx/coroutines/flow/FlowCollector.hpp"
#include "kotlinx/coroutines/intrinsics/Intrinsics.hpp"
#include <exception>
#include <thread>

namespace kotlinx::coroutines::flow {

    /**
 * Emits all elements from the given channel to this flow collector and cancels (consumes)
 * the channel afterwards.
 *
 * Kotlin: suspend fun <T> FlowCollector<T>.emitAll(channel: ReceiveChannel<T>)
 */
    template <typename T>
    inline void* emit_all(
        FlowCollector<T>* collector,
        ::kotlinx::coroutines::channels::ReceiveChannel<T>* channel,
        Continuation<void*>* cont
    ) {
        return emit_all_impl(collector, channel, /*consume=*/true, cont);
    }

    /**
 * Internal implementation with configurable "consume" behavior.
 *
 * Kotlin: private suspend fun <T> FlowCollector<T>.emitAllImpl(channel: ReceiveChannel<T>, consume: Boolean)
 */
    template <typename T>
    inline void* emit_all_impl(
        FlowCollector<T>* collector,
        ::kotlinx::coroutines::channels::ReceiveChannel<T>* channel,
        bool consume,
        Continuation<void*>* cont
    ) {
        std::exception_ptr cause = nullptr;
        try {
            while (true) {
                auto result = channel->try_receive();
                if (result.is_closed()) {
                    break;
                }
                if (result.is_success()) {
                    void* emitted = collector->emit(result.get_or_throw(), cont);
                    if (emitted == intrinsics::get_COROUTINE_SUSPENDED()) {
                        return emitted;
                    }
                    continue;
                }

                // Kotlin would suspend until there is an element or the channel is closed.
                // Our current runtime doesn't have that yet, so we block/yield.
                // TODO(suspend-plugin): rewrite into a proper suspend loop once ReceiveChannel is suspend-aware.
                std::this_thread::yield();
            }
        } catch (...) {
            cause = std::current_exception();
            throw;
        }

        if (consume) {
            channel->cancel(cause);
        }

        return nullptr;
    }

} // namespace kotlinx::coroutines::flow
