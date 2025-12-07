// Transliterated from: integration/kotlinx-coroutines-slf4j/src/MDCContext.kt

// TODO: #include equivalent
// import kotlinx.coroutines.*
// import org.slf4j.MDC
// import kotlin.coroutines.AbstractCoroutineContextElement
// import kotlin.coroutines.CoroutineContext

namespace kotlinx {
namespace coroutines {
namespace slf4j {

/**
 * The value of [MDC] context map.
 * See [MDC.getCopyOfContextMap].
 */
using MDCContextMap = std::map<std::string, std::string>*;

/**
 * [MDC] context element for [CoroutineContext].
 *
 * Example:
 *
 * ```
 * MDC.put("kotlin", "rocks") // Put a value into the MDC context
 *
 * launch(MDCContext()) {
 *     logger.info { "..." }   // The MDC context contains the mapping here
 * }
 * ```
 *
 * Note that you cannot update MDC context from inside the coroutine simply
 * using [MDC.put]. These updates are going to be lost on the next suspension and
 * reinstalled to the MDC context that was captured or explicitly specified in
 * [contextMap] when this object was created on the next resumption.
 *
 * For example, the following code will not work as expected:
 *
 * ```
 * launch(MDCContext()) {
 *     MDC.put("key", "value") // This update will be lost
 *     delay(100)
 *     println(MDC.get("key")) // This will print null
 * }
 * ```
 *
 * Instead, you should use [withContext] to capture the updated MDC context:
 *
 * ```
 * launch(MDCContext()) {
 *     MDC.put("key", "value") // This update will be captured
 *     withContext(MDCContext()) {
 *         delay(100)
 *         println(MDC.get("key")) // This will print "value"
 *     }
 * }
 * ```
 *
 * There is no way to implicitly propagate MDC context updates from inside the coroutine to the outer scope.
 * You have to capture the updated MDC context and restore it explicitly. For example:
 *
 * ```
 * MDC.put("a", "b")
 * val contextMap = withContext(MDCContext()) {
 *     MDC.put("key", "value")
 *     withContext(MDCContext()) {
 *         MDC.put("key2", "value2")
 *         withContext(MDCContext()) {
 *             yield()
 *             MDC.getCopyOfContextMap()
 *         }
 *     }
 * }
 * // contextMap contains: {"a"="b", "key"="value", "key2"="value2"}
 * MDC.setContextMap(contextMap)
 * ```
 *
 * @param contextMap the value of [MDC] context map.
 * Default value is the copy of the current thread's context map that is acquired via
 * [MDC.getCopyOfContextMap].
 */
class MDCContext : public ThreadContextElement<MDCContextMap>,
                   public AbstractCoroutineContextElement {
public:
    /**
     * Key of [MDCContext] in [CoroutineContext].
     */
    class Key : public CoroutineContext::Key<MDCContext> {
    public:
        static Key& instance() {
            static Key inst;
            return inst;
        }
    };

    /**
     * The value of [MDC] context map.
     */
    // @Suppress("MemberVisibilityCanBePrivate")
    const MDCContextMap context_map;

    MDCContext(MDCContextMap context_map = MDC::get_copy_of_context_map())
        : AbstractCoroutineContextElement(Key::instance()),
          context_map(context_map) {}

    /** @suppress */
    MDCContextMap update_thread_context(CoroutineContext& context) override {
        auto old_state = MDC::get_copy_of_context_map();
        set_current(context_map);
        return old_state;
    }

    /** @suppress */
    void restore_thread_context(CoroutineContext& context, MDCContextMap old_state) override {
        set_current(old_state);
    }

private:
    void set_current(MDCContextMap context_map) {
        if (context_map == nullptr) {
            MDC::clear();
        } else {
            MDC::set_context_map(*context_map);
        }
    }
};

} // namespace slf4j
} // namespace coroutines
} // namespace kotlinx

// TODO: Semantic implementation tasks:
// 1. Implement ThreadContextElement interface
// 2. Implement AbstractCoroutineContextElement base class
// 3. Implement MDC (SLF4J Mapped Diagnostic Context) integration
// 4. Implement CoroutineContext::Key mechanism
// 5. Implement updateThreadContext/restoreThreadContext
// 6. Handle nullable map pointer semantics
// 7. Implement companion object as singleton Key
// 8. Set up proper SLF4J dependency
