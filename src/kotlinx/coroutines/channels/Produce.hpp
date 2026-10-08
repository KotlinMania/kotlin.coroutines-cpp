#pragma once
// port-lint: source channels/Produce.kt
/**
 * @file Produce.hpp
 * @brief Producer coroutine scope and produce builder
 *
 * Transliterated from: kotlinx-coroutines-core/common/src/channels/Produce.kt
 */

#include "kotlinx/coroutines/channels/Channel.hpp"
#include "kotlinx/coroutines/channels/Channels.hpp"
#include "kotlinx/coroutines/channels/ProducerScope.hpp"
#include "kotlinx/coroutines/channels/ChannelCoroutine.hpp"
#include "kotlinx/coroutines/CoroutineStart.hpp"
#include "kotlinx/coroutines/channels/BufferOverflow.hpp"
#include "kotlinx/coroutines/CancellableContinuationImpl.hpp"
#include "kotlinx/coroutines/context_impl.hpp"
#include "kotlinx/coroutines/Dispatchers.hpp"
#include <functional>
#include <memory>

namespace kotlinx {
namespace coroutines {
namespace channels {

/**
 * Internal coroutine that implements ProducerScope.
 *
 * Transliterated from:
 * private class ProducerCoroutine<E>(
 *     parentContext: CoroutineContext, channel: Channel<E>
 * ) : ChannelCoroutine<E>(parentContext, channel, true, active = true), ProducerScope<E>
 */
// Transliterated from: kotlinx-coroutines-core/common/src/channels/Produce.kt:285-299
// NOTE(port): Generic channel bindings require this private source type in the header.
template <typename E>
class ProducerCoroutine : public ChannelCoroutine<E>, public ProducerScope<E> {
public:
    // Transliterated from: kotlinx-coroutines-core/common/src/channels/Produce.kt:285-287
    ProducerCoroutine(
        std::shared_ptr<CoroutineContext> parent_context,
        std::shared_ptr<Channel<E>> channel
    ) : ChannelCoroutine<E>(std::move(parent_context), std::move(channel), true, true) {}

    virtual ~ProducerCoroutine() = default;

    // Transliterated from: kotlinx-coroutines-core/common/src/channels/Produce.kt:288-289
    bool is_active() const override { return ChannelCoroutine<E>::is_active(); }

    /**
     * Called when the coroutine completes normally.
     * Closes the channel without a cause.
     */
    // Transliterated from: kotlinx-coroutines-core/common/src/channels/Produce.kt:291-293
    void on_completed(Unit value) override {
        (void)value;
        ChannelCoroutine<E>::_channel->close(nullptr);
    }

    /**
     * Called when the coroutine is cancelled.
     * Closes the channel with the cancellation cause.
     * If the channel was already closed and the exception wasn't handled,
     * delegates to the coroutine exception handler.
     */
    // Transliterated from: kotlinx-coroutines-core/common/src/channels/Produce.kt:295-298
    void on_cancelled(std::exception_ptr cause, bool handled) override {
        bool processed = ChannelCoroutine<E>::_channel->close(cause);
        if (!processed && !handled) {
            // Upstream: if the channel was already closed and the producer's exception
            // wasn't handled by another consumer, delegate to the context's
            // CoroutineExceptionHandler. The C++ port routes the same call here.
            handle_coroutine_exception(*this->get_context(), cause);
        }
    }

    // ProducerScope overrides (delegating to ChannelCoroutine -> Channel)
    bool is_closed_for_send() const override { return ChannelCoroutine<E>::is_closed_for_send(); }

    void* send(E element, Continuation<void*>* continuation) override {
        return ChannelCoroutine<E>::send(std::move(element), continuation);
    }

    ChannelResult<void> try_send(E element) override { return ChannelCoroutine<E>::try_send(std::move(element)); }

    bool close(std::exception_ptr cause = nullptr) override { return ChannelCoroutine<E>::close(cause); }

    void invoke_on_close(std::function<void(std::exception_ptr)> handler) override {
        ChannelCoroutine<E>::invoke_on_close(handler);
    }

    /**
     * A reference to the channel this coroutine sends elements to.
     * It is provided for convenience, so that the code in the coroutine can refer
     * to the channel as `channel` as opposed to `this`.
     * All the SendChannel functions on this interface delegate to
     * the channel instance returned by this property.
     */
    SendChannel<E>* get_channel() override { return ChannelCoroutine<E>::_channel.get(); }
};

/**
 * Launches a new coroutine to produce a stream of values by sending them to a channel
 * and returns a reference to the coroutine as a ReceiveChannel. This resulting
 * object can be used to receive elements produced by this coroutine.
 *
 * The scope of the coroutine contains the ProducerScope interface, which implements
 * both CoroutineScope and SendChannel, so that the coroutine can invoke send() directly.
 *
 * The kind of the resulting channel depends on the specified `capacity` parameter.
 * See the Channel interface documentation for details.
 * By default, an unbuffered channel is created.
 * If an invalid capacity value is specified, an exception is thrown.
 *
 * ## Behavior on termination
 *
 * The channel is closed when the coroutine completes.
 *
 * The running coroutine is cancelled when the channel is cancelled.
 *
 * If this coroutine finishes with an exception, it will close the channel with that exception
 * as the cause, so after receiving all the existing elements, all further attempts to receive
 * from it will throw the exception with which the coroutine finished.
 *
 * ## Coroutine context
 *
 * The coroutine context is inherited from the CoroutineScope. Additional context elements
 * can be specified with the context argument. If the context does not have any dispatcher
 * or other ContinuationInterceptor, then Dispatchers::Default is used. The parent job is
 * inherited from the CoroutineScope as well, but it can also be overridden with a
 * corresponding context element.
 *
 * ## Undelivered elements
 *
 * Some values that produce creates may be lost. There is no way to recover these lost elements.
 * If this is unsuitable, please create a Channel manually and pass the onUndeliveredElement
 * callback to the constructor.
 *
 * @note This is an experimental api. Behaviour of producers that work as children in a parent
 *       scope with respect to cancellation and error handling may change in the future.
 *
 * @param scope The CoroutineScope to launch the producer in
 * @param context Additional coroutine context elements (default: EmptyCoroutineContext)
 * @param capacity Channel buffer capacity (default: 0 for rendezvous)
 * @param on_buffer_overflow Behavior on buffer overflow (default: SUSPEND)
 * @param start Coroutine start option (default: DEFAULT)
 * @param block The producer block that sends elements to the channel
 * @return A ReceiveChannel to receive the produced elements
 *
 */
// Transliterated from: kotlinx-coroutines-core/common/src/channels/Produce.kt:269-283
template <typename E>
std::shared_ptr<ReceiveChannel<E>> produce(
    CoroutineScope* scope, std::shared_ptr<CoroutineContext> context,
    int capacity, BufferOverflow on_buffer_overflow, CoroutineStart start,
    std::function<void(std::exception_ptr)> on_completion,
    std::function<void*(ProducerScope<E>*, std::shared_ptr<Continuation<void*>>)> block) {
    auto channel = create_channel<E>(capacity, on_buffer_overflow);
    auto new_context = new_coroutine_context(
        scope, context ? std::move(context) : EmptyCoroutineContext::instance());
    auto coroutine = std::make_shared<ProducerCoroutine<E>>(new_context, channel);
    if (on_completion) coroutine->invoke_on_completion(std::move(on_completion));
    coroutine->start(start, static_cast<ProducerScope<E>*>(coroutine.get()), std::move(block));
    return coroutine;
}

// Transliterated from: kotlinx-coroutines-core/common/src/channels/Produce.kt:269-283
// NOTE(port): C++ trailing blocks cannot follow defaulted parameters; this
// forwarding overload supplies Kotlin's omitted onCompletion argument.
template <typename E>
std::shared_ptr<ReceiveChannel<E>> produce(
    CoroutineScope* scope, std::shared_ptr<CoroutineContext> context,
    int capacity, BufferOverflow on_buffer_overflow, CoroutineStart start,
    std::function<void*(ProducerScope<E>*, std::shared_ptr<Continuation<void*>>)> block) {
    return produce<E>(scope, std::move(context), capacity, on_buffer_overflow,
                      start, nullptr, std::move(block));
}

// Transliterated from: kotlinx-coroutines-core/common/src/channels/Produce.kt:259-266
template <typename E>
std::shared_ptr<ReceiveChannel<E>> produce(
    CoroutineScope* scope, std::shared_ptr<CoroutineContext> context,
    int capacity, CoroutineStart start,
    std::function<void(std::exception_ptr)> on_completion,
    std::function<void*(ProducerScope<E>*, std::shared_ptr<Continuation<void*>>)> block) {
    return produce<E>(scope, std::move(context), capacity, BufferOverflow::SUSPEND,
                      start, std::move(on_completion), std::move(block));
}

// Transliterated from: kotlinx-coroutines-core/common/src/channels/Produce.kt:239-244
// NOTE(port): Place the block before default arguments to express Kotlin's
// trailing-lambda call with omitted context and capacity in ordinary C++.
template <typename E>
std::shared_ptr<ReceiveChannel<E>> produce(
    CoroutineScope* scope,
    std::function<void*(ProducerScope<E>*, std::shared_ptr<Continuation<void*>>)> block,
    std::shared_ptr<CoroutineContext> context = EmptyCoroutineContext::instance(),
    int capacity = Channel<E>::RENDEZVOUS) {
    return produce<E>(scope, std::move(context), capacity, BufferOverflow::SUSPEND,
                      CoroutineStart::DEFAULT, nullptr, std::move(block));
}

// Transliterated from: kotlinx-coroutines-core/common/src/channels/Produce.kt:269-283
// NOTE(port): An ordinary nonsuspending C++ block enters the same builder.
template <typename E>
std::shared_ptr<ReceiveChannel<E>> produce(
    CoroutineScope* scope,
    std::shared_ptr<CoroutineContext> context,
    int capacity,
    BufferOverflow on_buffer_overflow,
    CoroutineStart start,
    std::function<void(ProducerScope<E>*)> block) {
    std::function<void*(ProducerScope<E>*, std::shared_ptr<Continuation<void*>>)> suspend_block =
        [block = std::move(block)](ProducerScope<E>* receiver,
                                  std::shared_ptr<Continuation<void*>>) -> void* {
            block(receiver);
            return nullptr;
        };
    return produce<E>(scope, std::move(context), capacity, on_buffer_overflow,
                      start, nullptr, std::move(suspend_block));
}

/**
 * Suspends the current coroutine until the channel is either closed or cancelled.
 *
 * The given block will be executed unconditionally before this function returns.
 * `await_close([&]{ cleanup(); })` is a convenient shorthand for the often useful form
 * `try { await_close(); } finally { cleanup(); }`.
 *
 * This function can only be invoked directly inside the same coroutine that is its receiver.
 * Specifying the receiver of await_close explicitly is most probably a mistake.
 *
 * This suspending function is cancellable: if the Job of the current coroutine is cancelled
 * while this suspending function is waiting, this function immediately resumes with
 * CancellationException. There is a prompt cancellation guarantee: even if this function
 * is ready to return, but was cancelled while suspended, CancellationException will be thrown.
 *
 * Internally, await_close is implemented using SendChannel::invoke_on_close.
 * Currently, every channel can have at most one invoke_on_close handler.
 * This means that calling await_close several times in a row or combining it with other
 * invoke_on_close invocations is prohibited.
 *
 * @throws std::logic_error if invoked from outside the ProducerScope (by leaking `this`
 *         outside the producer coroutine).
 * @throws std::logic_error if this channel already has an invoke_on_close handler registered.
 *
 * Transliterated from:
 * public suspend fun ProducerScope<*>.awaitClose(block: () -> Unit = {})
 */
namespace internal {
// Transliterated from: kotlinx-coroutines-core/common/src/channels/Produce.kt:60-71
// NOTE(port): The generic receiver supplies its actual Job and close registration;
// the concrete suspended try/finally lives in Produce.cpp.
void* await_close_erased(Job* receiver,
    std::function<void(std::function<void(std::exception_ptr)>)> register_close,
    std::function<void()> block, std::shared_ptr<Continuation<void*>> continuation);
}

// Transliterated from: kotlinx-coroutines-core/common/src/channels/Produce.kt:60-71
template <typename E>
void* await_close(ProducerScope<E>* scope, std::function<void()> block,
                  Continuation<void*>* continuation) {
    auto* job = dynamic_cast<JobSupport*>(scope);
    auto owner = job ? job->weak_from_this().lock() : nullptr;
    return internal::await_close_erased(dynamic_cast<Job*>(scope),
        [scope, owner = std::move(owner)](std::function<void(std::exception_ptr)> handler) {
            scope->invoke_on_close(std::move(handler));
        }, std::move(block), kotlinx::coroutines::internal::retain_continuation(continuation));
}

// Transliterated from: kotlinx-coroutines-core/common/src/channels/Produce.kt:60-71
template <typename E>
void* await_close(ProducerScope<E>* scope, Continuation<void*>* continuation) {
    return await_close<E>(scope, [] {}, continuation);
}

} // namespace channels
} // namespace coroutines
} // namespace kotlinx
