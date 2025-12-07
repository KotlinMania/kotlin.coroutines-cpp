// Transliterated from: reactive/kotlinx-coroutines-reactor/src/ReactorContext.cpp

// TODO: #include <kotlin/coroutines/continuation.hpp>
// TODO: #include <kotlinx/coroutines/reactive.hpp>
// TODO: #include <reactor/util/context.hpp>

namespace kotlinx { namespace coroutines { namespace reactor {

/**
 * Wraps Reactor's [Context] into a [CoroutineContext] element for seamless integration between
 * Reactor and kotlinx.coroutines.
 * [Context.asCoroutineContext] puts Reactor's [Context] elements into a [CoroutineContext],
 * which can be used to propagate the information about Reactor's [Context] through coroutines.
 *
 * This context element is implicitly propagated through subscribers' context by all Reactive integrations,
 * such as [mono], [flux], [Publisher.asFlow][asFlow], [Flow.asPublisher][asPublisher] and [Flow.asFlux][asFlux].
 * Functions that subscribe to a reactive stream
 * (e.g. [Publisher.awaitFirst][kotlinx.coroutines.reactive.awaitFirst]), too, propagate [ReactorContext]
 * to the subscriber's [Context].
 **
 * ### Examples of Reactive context integration.
 *
 * #### Propagating ReactorContext to Reactor's Context
 * ```
 * val flux = myDatabaseService.getUsers()
 *     .contextWrite { ctx -> println(ctx); ctx }
 * flux.awaitFirst() // Will print "null"
 *
 * // Now add ReactorContext
 * withContext(Context.of("answer", "42").asCoroutineContext()) {
 *     flux.awaitFirst() // Will print "Context{'key'='value'}"
 * }
 * ```
 *
 * #### Propagating subscriber's Context to ReactorContext:
 * ```
 * val flow = flow {
 *     println("Reactor context in Flow: " + currentCoroutineContext()[ReactorContext])
 * }
 * // No context
 * flow.asFlux()
 *     .subscribe() // Will print 'Reactor context in Flow: null'
 * // Add subscriber's context
 * flow.asFlux()
 *     .contextWrite { ctx -> ctx.put("answer", 42) }
 *     .subscribe() // Will print "Reactor context in Flow: Context{'answer'=42}"
 * ```
 */
class ReactorContext : public AbstractCoroutineContextElement {
public:
    Context context;

    // `Context.of` is zero-cost if the argument is a `Context`
    ReactorContext(ContextView context_view) : AbstractCoroutineContextElement(Key), context(Context::of(context_view)) {
    }

    ReactorContext(Context ctx) : AbstractCoroutineContextElement(Key), context(ctx) {
    }

    class KeyType : public CoroutineContext::Key<ReactorContext> {};
    static KeyType Key;

    std::string to_string() const override {
        return context.to_string();
    }
};

ReactorContext::KeyType ReactorContext::Key;

/**
 * Wraps the given [ContextView] into [ReactorContext], so it can be added to the coroutine's context
 * and later used via `coroutineContext[ReactorContext]`.
 */
ReactorContext as_coroutine_context(ContextView* context_view) {
    return ReactorContext(*context_view);
}

/** @suppress */
// @Deprecated("The more general version for ContextView should be used instead", level = DeprecationLevel.HIDDEN)
ReactorContext as_coroutine_context(Context* context) {
    return context->read_only().as_coroutine_context(); // `readOnly()` is zero-cost.
}

/**
 * Updates the Reactor context in this [CoroutineContext], adding (or possibly replacing) some values.
 */
CoroutineContext extend_reactor_context(CoroutineContext coroutine_context, ContextView extensions) {
    ReactorContext* existing = coroutine_context[ReactorContext::Key];
    if (existing != nullptr) {
        return existing->context.put_all(extensions).as_coroutine_context();
    } else {
        return extensions.as_coroutine_context();
    }
}

} } } // namespace kotlinx::coroutines::reactor

// TODO: Semantic implementation tasks:
// 1. Implement AbstractCoroutineContextElement base class
// 2. Implement CoroutineContext and CoroutineContext::Key<T>
// 3. Implement Context and ContextView from Reactor
// 4. Implement Context::of() static method
// 5. Implement Context::empty() static method
// 6. Implement Context::put_all() method
// 7. Implement ContextView::read_only() method
// 8. Implement to_string() for Context
// 9. Implement static Key member pattern
// 10. Handle operator[] for CoroutineContext
