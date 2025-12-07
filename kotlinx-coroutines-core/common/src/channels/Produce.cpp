// Transliterated from Kotlin to C++
// Original: kotlinx-coroutines-core/common/src/channels/Produce.kt
//
// TODO: Implement coroutine semantics (suspend functions, CoroutineScope, etc.)
// TODO: Map Kotlin annotations (@BuilderInference, @ExperimentalCoroutinesApi, @InternalCoroutinesApi)
// TODO: Implement Kotlin lambda closures and capture
// TODO: Map Kotlin extension functions
// TODO: Implement suspendCancellableCoroutine

#include <memory>
#include <functional>
#include <exception>

namespace kotlinx {
namespace coroutines {
namespace channels {

// Forward declarations
template<typename E> class SendChannel;
template<typename E> class ReceiveChannel;
template<typename E> class Channel;
class CoroutineScope;
class CoroutineContext;
class CoroutineStart;
class Job;
class CompletionHandler;
class BufferOverflow;

/**
 * Scope for the [produce][CoroutineScope.produce], [callbackFlow] and [channelFlow] builders.
 */
template<typename E>
class ProducerScope : public CoroutineScope, public SendChannel<E> {
public:
    /**
     * A reference to the channel this coroutine [sends][send] elements to.
     * It is provided for convenience, so that the code in the coroutine can refer
     * to the channel as `channel` as opposed to `this`.
     * All the [SendChannel] functions on this interface delegate to
     * the channel instance returned by this property.
     */
    virtual SendChannel<E>* channel() = 0;
};

/**
 * Suspends the current coroutine until the channel is either
 * [closed][SendChannel.close] or [cancelled][ReceiveChannel.cancel].
 *
 * The given [block] will be executed unconditionally before this function returns.
 * `awaitClose { cleanup() }` is a convenient shorthand for the often useful form
 * `try { awaitClose() } finally { cleanup() }`.
 */
// suspend
template<typename T>
void await_close(ProducerScope<T>* scope, std::function<void()> block = [](){}) {
    // TODO: check(kotlin.coroutines.coroutineContext[Job] === this) { "awaitClose() can only be invoked from the producer context" }
    // TODO: try {
    // TODO:     suspendCancellableCoroutine<Unit> { cont ->
    // TODO:         invokeOnClose {
    // TODO:             cont.resume(Unit)
    // TODO:         }
    // TODO:     }
    // TODO: } finally {
    // TODO:     block()
    // TODO: }
    throw std::runtime_error("Not implemented: await_close");
}

/**
 * Launches a new coroutine to produce a stream of values by sending them to a channel
 * and returns a reference to the coroutine as a [ReceiveChannel].
 */
// @ExperimentalCoroutinesApi
template<typename E>
ReceiveChannel<E>* produce(
    CoroutineScope* scope,
    CoroutineContext* context, // = EmptyCoroutineContext
    int capacity, // = Channel.RENDEZVOUS
    std::function<void(ProducerScope<E>*)> block // @BuilderInference suspend
) {
    // TODO: return produce(context, capacity, BufferOverflow.SUSPEND, CoroutineStart.DEFAULT, onCompletion = null, block = block)
    throw std::runtime_error("Not implemented: produce");
}

/**
 * **This is an internal API and should not be used from general code.**
 * The `onCompletion` parameter will be redesigned.
 */
// @InternalCoroutinesApi
template<typename E>
ReceiveChannel<E>* produce_internal(
    CoroutineScope* scope,
    CoroutineContext* context, // = EmptyCoroutineContext
    int capacity, // = 0
    CoroutineStart start, // = CoroutineStart.DEFAULT
    CompletionHandler* on_completion, // = null
    std::function<void(ProducerScope<E>*)> block // @BuilderInference suspend
) {
    // TODO: return produce(context, capacity, BufferOverflow.SUSPEND, start, onCompletion, block)
    throw std::runtime_error("Not implemented: produce_internal");
}

// Internal version of produce that is maximally flexible, but is not exposed through public API (too many params)
template<typename E>
ReceiveChannel<E>* produce_impl(
    CoroutineScope* scope,
    CoroutineContext* context, // = EmptyCoroutineContext
    int capacity, // = 0
    BufferOverflow on_buffer_overflow, // = BufferOverflow.SUSPEND
    CoroutineStart start, // = CoroutineStart.DEFAULT
    CompletionHandler* on_completion, // = null
    std::function<void(ProducerScope<E>*)> block // @BuilderInference suspend
) {
    // TODO: val channel = Channel<E>(capacity, onBufferOverflow)
    // TODO: val newContext = newCoroutineContext(context)
    // TODO: val coroutine = ProducerCoroutine(newContext, channel)
    // TODO: if (onCompletion != null) coroutine.invokeOnCompletion(handler = onCompletion)
    // TODO: coroutine.start(start, coroutine, block)
    // TODO: return coroutine
    throw std::runtime_error("Not implemented: produce_impl");
}

template<typename E>
class ProducerCoroutine : public ChannelCoroutine<E>, public ProducerScope<E> {
public:
    ProducerCoroutine(
        CoroutineContext* parent_context,
        Channel<E>* channel
    ) : ChannelCoroutine<E>(parent_context, channel, true, true) {
    }

    bool is_active() const override {
        // TODO: return super.isActive
        return false;
    }

    void on_completed(void* value) override {
        // TODO: _channel.close()
    }

    void on_cancelled(std::exception_ptr cause, bool handled) override {
        // TODO: val processed = _channel.close(cause)
        // TODO: if (!processed && !handled) handleCoroutineException(context, cause)
    }
};

// TODO: Implement ChannelCoroutine base class
// TODO: Implement SendChannel, ReceiveChannel, Channel interfaces

} // namespace channels
} // namespace coroutines
} // namespace kotlinx
