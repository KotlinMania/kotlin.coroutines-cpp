#pragma once
/**
 * Transliterated from: kotlinx-coroutines-core/common/src/channels/ChannelCoroutine.kt
 *
 * Kotlin file header (translated):
 *   package kotlinx.coroutines.channels
 */

#include "kotlinx/coroutines/AbstractCoroutine.hpp"
#include "kotlinx/coroutines/CoroutineContext.hpp"
#include "kotlinx/coroutines/Exceptions.hpp"
#include "kotlinx/coroutines/Unit.hpp"
#include "kotlinx/coroutines/channels/Channel.hpp"

#include <exception>
#include <functional>
#include <memory>
#include <utility>

namespace kotlinx::coroutines::channels {

/**
 * Upstream:
 *   internal open class ChannelCoroutine<E>(
 *       parentContext: CoroutineContext,
 *       protected val _channel: Channel<E>,
 *       initParentJob: Boolean,
 *       active: Boolean
 *   ) : AbstractCoroutine<Unit>(parentContext, initParentJob, active), Channel<E> by _channel { ... }
 *
 * Kotlin's `by` delegation forwards every interface member to `_channel`. C++ has no
 * delegation keyword, so each `Channel<E>` member is forwarded explicitly below.
 */
template <typename E>
class ChannelCoroutine : public AbstractCoroutine<Unit>, public Channel<E> {
public:
    ChannelCoroutine(
        std::shared_ptr<CoroutineContext> parent_context,
        std::shared_ptr<Channel<E>> channel,
        bool init_parent_job,
        bool active)
        : AbstractCoroutine<Unit>(std::move(parent_context), init_parent_job, active),
          _channel(std::move(channel)) {}

    ~ChannelCoroutine() override = default;

    /** Upstream: val channel: Channel<E> get() = this */
    Channel<E>* channel() { return this; }

    /**
     * Upstream:
     *   @Deprecated(level = HIDDEN, ...) override fun cancel() { cancelInternal(defaultCancellationException()) }
     */
    void cancel() override {
        cancel_internal(default_cancellation_exception());
    }

    /**
     * Upstream:
     *   final override fun cancel(cause: Throwable?): Boolean {
     *       cancelInternal(defaultCancellationException())
     *       return true
     *   }
     */
    bool cancel(std::exception_ptr /*cause*/) override {
        cancel_internal(default_cancellation_exception());
        return true;
    }

    /**
     * Upstream:
     *   final override fun cancel(cause: CancellationException?) {
     *       if (isCancelled) return
     *       cancelInternal(cause ?: defaultCancellationException())
     *   }
     *
     * The C++ port models `CancellationException?` as `std::exception_ptr` with nullptr as the
     * sentinel for the upstream `null` branch (which falls back to the default cancellation
     * exception).
     */
    void cancel_cancellation(std::exception_ptr cause) {
        if (this->is_cancelled()) return;
        cancel_internal(cause ? cause : default_cancellation_exception());
    }

    /**
     * Upstream:
     *   override fun cancelInternal(cause: Throwable) {
     *       val exception = cause.toCancellationException()
     *       _channel.cancel(exception)  // cancel the channel
     *       cancelCoroutine(exception)  // cancel the job
     *   }
     */
    void cancel_internal(std::exception_ptr cause) override {
        auto exception = to_cancellation_exception(cause);
        _channel->cancel(exception);
        AbstractCoroutine<Unit>::cancel_coroutine(exception);
    }

    // ------------------------------------------------------------------------
    // Channel<E> delegation (Kotlin `by _channel`)
    // ------------------------------------------------------------------------

    bool is_closed_for_send() const override { return _channel->is_closed_for_send(); }

    void* send(E element, Continuation<void*>* continuation) override {
        return _channel->send(std::move(element), continuation);
    }

    ChannelResult<void> try_send(E element) override {
        return _channel->try_send(std::move(element));
    }

    bool close(std::exception_ptr cause = nullptr) override { return _channel->close(cause); }

    void invoke_on_close(std::function<void(std::exception_ptr)> handler) override {
        _channel->invoke_on_close(std::move(handler));
    }

    bool is_closed_for_receive() const override { return _channel->is_closed_for_receive(); }
    bool is_empty() const override { return _channel->is_empty(); }

    void* receive(Continuation<void*>* continuation) override {
        return _channel->receive(continuation);
    }

    void* receive_catching(Continuation<void*>* continuation) override {
        return _channel->receive_catching(continuation);
    }

    ChannelResult<E> try_receive() override { return _channel->try_receive(); }

    selects::SelectClause2<E, SendChannel<E>*>& on_send() override { return _channel->on_send(); }
    selects::SelectClause1<E>& on_receive() override { return _channel->on_receive(); }
    selects::SelectClause1<ChannelResult<E>>& on_receive_catching() override {
        return _channel->on_receive_catching();
    }

    std::unique_ptr<ChannelIterator<E>> iterator() override { return _channel->iterator(); }

protected:
    /** Upstream: protected val _channel: Channel<E> */
    std::shared_ptr<Channel<E>> _channel;
};

} // namespace kotlinx::coroutines::channels
