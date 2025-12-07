// Transliterated from: reactive/kotlinx-coroutines-reactor/src/ReactorFlow.cpp

// TODO: #include <kotlinx/coroutines/coroutines.hpp>
// TODO: #include <kotlinx/coroutines/flow.hpp>
// TODO: #include <kotlinx/coroutines/reactive.hpp>
// TODO: #include <org/reactivestreams/publisher.hpp>
// TODO: #include <reactor/core/core_subscriber.hpp>
// TODO: #include <reactor/core/publisher.hpp>
// TODO: #include <kotlin/coroutines/continuation.hpp>

namespace kotlinx { namespace coroutines { namespace reactor {

/**
 * Converts the given flow to a cold flux.
 * The original flow is cancelled when the flux subscriber is disposed.
 *
 * This function is integrated with [ReactorContext], see its documentation for additional details.
 *
 * An optional [context] can be specified to control the execution context of calls to [Subscriber] methods.
 * You can set a [CoroutineDispatcher] to confine them to a specific thread and/or various [ThreadContextElement] to
 * inject additional context into the caller thread. By default, the [Unconfined][Dispatchers.Unconfined] dispatcher
 * is used, so calls are performed from an arbitrary thread.
 */
// @JvmOverloads // binary compatibility
template<typename T>
Flux<T> as_flux(Flow<T>* flow, CoroutineContext context = EmptyCoroutineContext) {
    return FlowAsFlux<T>(flow, Dispatchers::Unconfined + context);
}

template<typename T>
class FlowAsFlux : public Flux<T> {
private:
    Flow<T>* flow;
    CoroutineContext context;

public:
    FlowAsFlux(Flow<T>* flow_param, CoroutineContext context_param)
        : flow(flow_param), context(context_param) {
    }

    void subscribe(CoreSubscriber<T>* subscriber) override {
        bool has_context = !subscriber->current_context().is_empty();
        Flow<T>* source = has_context
            ? flow->flow_on(subscriber->current_context().as_coroutine_context())
            : flow;
        subscriber->on_subscribe(new FlowSubscription<T>(source, subscriber, context));
    }
};

} } } // namespace kotlinx::coroutines::reactor

// TODO: Semantic implementation tasks:
// 1. Implement Flow<T> type
// 2. Implement Flux<T> base class
// 3. Implement CoreSubscriber<T> interface
// 4. Implement FlowSubscription<T> class
// 5. Implement flow_on() method for Flow
// 6. Implement Dispatchers::Unconfined
// 7. Implement operator+ for CoroutineContext
// 8. Implement EmptyCoroutineContext
// 9. Implement is_empty() for Context
// 10. Implement as_coroutine_context() for Context
// 11. Handle JvmOverloads annotation effect
