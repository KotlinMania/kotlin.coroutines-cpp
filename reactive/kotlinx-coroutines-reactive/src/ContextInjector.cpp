// Transliterated from: reactive/kotlinx-coroutines-reactive/src/ContextInjector.kt
// TODO: Implement semantic correctness for context injection

namespace kotlinx {
namespace coroutines {
namespace reactive {

// TODO: #include <kotlinx/coroutines/InternalCoroutinesApi.hpp>
// TODO: #include <org/reactivestreams/Publisher.hpp>
// TODO: #include <kotlin/coroutines/CoroutineContext.hpp>

/** @suppress */
// @InternalCoroutinesApi
class ContextInjector {
public:
    /**
     * Injects `ReactorContext` element from the given context into the `SubscriberContext` of the publisher.
     * This API used as an indirection layer between `reactive` and `reactor` modules.
     */
    template<typename T>
    virtual Publisher<T> inject_coroutine_context(Publisher<T>& publisher, const CoroutineContext& coroutine_context) = 0;

    virtual ~ContextInjector() = default;
};

} // namespace reactive
} // namespace coroutines
} // namespace kotlinx

/*
 * TODO List for semantic implementation:
 *
 * 1. Implement CoroutineContext type
 * 2. Implement Publisher interface
 * 3. Implement service loader mechanism for ContextInjector
 * 4. Add reactor module integration
 */
