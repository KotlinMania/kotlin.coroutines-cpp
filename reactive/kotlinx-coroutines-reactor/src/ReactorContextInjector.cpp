// Transliterated from: reactive/kotlinx-coroutines-reactor/src/ReactorContextInjector.cpp

// TODO: #include <kotlinx/coroutines/reactive.hpp>
// TODO: #include <org/reactivestreams/publisher.hpp>
// TODO: #include <reactor/core/publisher.hpp>
// TODO: #include <reactor/util/context.hpp>
// TODO: #include <kotlin/coroutines/continuation.hpp>

namespace kotlinx { namespace coroutines { namespace reactor {

class ReactorContextInjector : public ContextInjector {
public:
    /**
     * Injects all values from the [ReactorContext] entry of the given coroutine context
     * into the downstream [Context] of Reactor's [Publisher] instances of [Mono] or [Flux].
     */
    template<typename T>
    Publisher<T>* inject_coroutine_context(Publisher<T>* publisher, CoroutineContext coroutine_context) override {
        ReactorContext* reactor_context = coroutine_context[ReactorContext::Key];
        if (reactor_context == nullptr) {
            return publisher;
        }

        Mono<T>* mono = dynamic_cast<Mono<T>*>(publisher);
        if (mono != nullptr) {
            return mono->context_write(reactor_context->context);
        }

        Flux<T>* flux = dynamic_cast<Flux<T>*>(publisher);
        if (flux != nullptr) {
            return flux->context_write(reactor_context->context);
        }

        return publisher;
    }
};

} } } // namespace kotlinx::coroutines::reactor

// TODO: Semantic implementation tasks:
// 1. Implement ContextInjector interface
// 2. Implement Publisher<T> base class
// 3. Implement Mono<T> and Flux<T> as Publisher<T> subclasses
// 4. Implement context_write() method for Mono and Flux
// 5. Implement ReactorContext access from CoroutineContext
// 6. Handle dynamic_cast pattern or use visitor pattern
// 7. Implement template override mechanism
