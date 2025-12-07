// Transliterated from: reactive/kotlinx-coroutines-reactor/src/Flux.cpp

// TODO: #include <kotlinx/coroutines/coroutines.hpp>
// TODO: #include <kotlinx/coroutines/channels.hpp>
// TODO: #include <kotlinx/coroutines/reactive.hpp>
// TODO: #include <org/reactivestreams/publisher.hpp>
// TODO: #include <reactor/core/publisher.hpp>
// TODO: #include <reactor/util/context.hpp>
// TODO: #include <kotlin/coroutines/continuation.hpp>

namespace kotlinx { namespace coroutines { namespace reactor {

/**
 * Creates a cold reactive [Flux] that runs the given [block] in a coroutine.
 * Every time the returned flux is subscribed, it starts a new coroutine in the specified [context].
 * The coroutine emits ([Subscriber.onNext]) values with [send][ProducerScope.send], completes ([Subscriber.onComplete])
 * when the coroutine completes, or, in case the coroutine throws an exception or the channel is closed,
 * emits the error ([Subscriber.onError]) and closes the channel with the cause.
 * Unsubscribing cancels the running coroutine.
 *
 * Invocations of [send][ProducerScope.send] are suspended appropriately when subscribers apply back-pressure and to
 * ensure that [onNext][Subscriber.onNext] is not invoked concurrently.
 *
 * **Note: This is an experimental api.** Behaviour of publishers that work as children in a parent scope with respect
 *        to cancellation and error handling may change in the future.
 *
 * @throws IllegalArgumentException if the provided [context] contains a [Job] instance.
 */
template<typename T>
// @BuilderInference
// TODO: implement coroutine suspension
Flux<T> flux(
    CoroutineContext context = EmptyCoroutineContext,
    std::function<void(ProducerScope<T>&)> block
) {
    if (context[Job{}] != nullptr) {
        throw std::invalid_argument("Flux context cannot contain job in it. "
            "Its lifecycle should be managed via Disposable handle. Had " + context.to_string());
    }
    return Flux<T>::from(reactor_publish(GlobalScope, context, block));
}

template<typename T>
// @BuilderInference
// TODO: implement coroutine suspension
Publisher<T> reactor_publish(
    CoroutineScope* scope,
    CoroutineContext context = EmptyCoroutineContext,
    std::function<void(ProducerScope<T>&)> block
) {
    return Publisher<T>([scope, context, block](Subscriber<T>* subscriber) {
        CoreSubscriber<T>* core_subscriber = dynamic_cast<CoreSubscriber<T>*>(subscriber);
        if (core_subscriber == nullptr) {
            subscriber->reject(std::invalid_argument("Subscriber is not an instance of CoreSubscriber, context can not be extracted."));
            return;
        }
        Context current_context = core_subscriber->current_context();
        ReactorContext reactor_context = context.extend_reactor_context(current_context);
        CoroutineContext new_context = scope->new_coroutine_context(context + reactor_context);
        PublisherCoroutine<T>* coroutine = new PublisherCoroutine<T>(new_context, core_subscriber, kReactorHandler);
        core_subscriber->on_subscribe(coroutine); // do it first (before starting coroutine), to avoid unnecessary suspensions
        coroutine->start(CoroutineStart::kDefault, coroutine, block);
    });
}

const auto kReactorHandler = [](Throwable* cause, CoroutineContext ctx) -> void {
    CancellationException* cancellation = dynamic_cast<CancellationException*>(cause);
    if (cancellation == nullptr) {
        try {
            ReactorContext* reactor_ctx = ctx[ReactorContext{}];
            Context empty_context = Context::empty();
            Operators::on_operator_error(cause, reactor_ctx != nullptr ? reactor_ctx->context : empty_context);
        } catch (std::exception& e) {
            cause->add_suppressed(e);
            handle_coroutine_exception(ctx, cause);
        }
    }
};

/** The proper way to reject the subscriber, according to
 * [the reactive spec](https://github.com/reactive-streams/reactive-streams-jvm/blob/v1.0.3/README.md#1.9)
 */
template<typename T>
void reject(Subscriber<T>* subscriber, Throwable* t) {
    if (subscriber == nullptr) {
        throw std::runtime_error("The subscriber can not be null");
    }
    subscriber->on_subscribe(new class : public Subscription {
        void request(int64_t n) override {
            // intentionally left blank
        }
        void cancel() override {
            // intentionally left blank
        }
    });
    subscriber->on_error(t);
}

/**
 * @suppress
 */
// @Deprecated
//     message = "CoroutineScope.flux is deprecated in favour of top-level flux",
//     level = DeprecationLevel.HIDDEN,
//     replaceWith = ReplaceWith("flux(context, block)")
// Since 1.3.0, will be error in 1.3.1 and hidden in 1.4.0. Binary compatibility with Spring
template<typename T>
// @BuilderInference
// TODO: implement coroutine suspension
Flux<T> flux(
    CoroutineScope* scope,
    CoroutineContext context = EmptyCoroutineContext,
    std::function<void(ProducerScope<T>&)> block
) {
    return Flux<T>::from(reactor_publish(scope, context, block));
}

} } } // namespace kotlinx::coroutines::reactor

// TODO: Semantic implementation tasks:
// 1. Implement Flux<T> type and from() static method
// 2. Implement Publisher<T> type and subscription mechanism
// 3. Implement Subscriber<T> and CoreSubscriber<T> interfaces
// 4. Implement ProducerScope<T> and send() method
// 5. Implement CoroutineScope and GlobalScope
// 6. Implement CoroutineContext with operator[] and operator+
// 7. Implement Job context element
// 8. Implement CoroutineStart enum with kDefault
// 9. Implement PublisherCoroutine class
// 10. Implement Context type from Reactor
// 11. Implement ReactorContext wrapper
// 12. Implement extend_reactor_context() method
// 13. Implement new_coroutine_context() method
// 14. Implement Throwable and CancellationException types
// 15. Implement Operators::on_operator_error()
// 16. Implement handle_coroutine_exception()
// 17. Implement Subscription interface
// 18. Convert dynamic casting to appropriate C++ patterns
// 19. Implement lambda to function pointer conversions
// 20. Handle BuilderInference annotation effect
