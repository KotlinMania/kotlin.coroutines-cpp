// Original Kotlin package: kotlinx.coroutines.debug.junit5
// Line-by-line C++ transliteration from Kotlin
//
// TODO: JUnit5 types: InvocationInterceptor, ExtensionContext, ReflectiveInvocationContext, RegisterExtension
// TODO: AnnotationSupport - JUnit5 platform utility
// TODO: AtomicBoolean - Java atomic, use std::atomic<bool>
// TODO: Optional - Java Optional, use std::optional
// TODO: Method, Constructor - Java reflection types, use C++ RTTI or type_info
// TODO: @JvmField, @JvmOverloads - Kotlin JVM annotations
// TODO: internal class/constructor - visibility modifiers
// TODO: companion object - static factory methods
// TODO: synchronized - Java synchronization, use std::lock_guard

#include <atomic>
#include <optional>
#include <string>
#include <mutex>
#include <stdexcept>

namespace kotlinx {
namespace coroutines {
namespace debug {
namespace junit5 {

// TODO: internal class
class CoroutinesTimeoutException : public std::exception {
public:
    CoroutinesTimeoutException(long timeout_ms)
        : timeout_ms_(timeout_ms)
        , message_("test timed out after " + std::to_string(timeout_ms) + " ms")
    {
    }

    const char* what() const noexcept override {
        return message_.c_str();
    }

    long timeout_ms() const { return timeout_ms_; }

private:
    long timeout_ms_;
    std::string message_;
};

// This JUnit5 extension allows running test, test factory, test template, and lifecycle methods in a separate thread,
// failing them after the provided time limit and interrupting the thread.
//
// Additionally, it installs DebugProbes and dumps all coroutines at the moment of the timeout. It also cancels
// coroutines on timeout if cancelOnTimeout set to `true`.
// enableCoroutineCreationStackTraces controls the corresponding DebugProbes.enableCreationStackTraces property
// and can be optionally enabled if the creation stack traces are necessary.
//
// Beware that if several tests that use this extension set enableCoroutineCreationStackTraces to different values and
// execute in parallel, the behavior is ill-defined. In order to avoid conflicts between different instances of this
// extension when using JUnit5 in parallel, use ResourceLock with resource name `coroutines timeout` on tests that use
// it. Note that the tests annotated with CoroutinesTimeout already use this ResourceLock, so there is no need to
// annotate them additionally.
//
// Note that while calls to test factories are verified to finish in the specified time, but the methods that they
// produce are not affected by this extension.
//
// Beware that registering the extension via CoroutinesTimeout annotation conflicts with manually registering it on
// the same tests via other methods (most notably, RegisterExtension) and is prohibited.
//
// Example of usage:
// ```
// class HangingTest {
//     @JvmField
//     @RegisterExtension
//     val timeout = CoroutinesTimeoutExtension.seconds(5)
//
//     @Test
//     fun testThatHangs() = runBlocking {
//          ...
//          delay(Long.MAX_VALUE) // somewhere deep in the stack
//          ...
//     }
// }
// ```
//
// @see CoroutinesTimeout

// NB: the constructor is not private so that JUnit is able to call it via reflection.
// TODO: internal class CoroutinesTimeoutExtension : InvocationInterceptor
class CoroutinesTimeoutExtension /* TODO: : InvocationInterceptor */ {
public:
    // TODO: internal constructor
    CoroutinesTimeoutExtension(
        bool enable_coroutine_creation_stack_traces = false,
        std::optional<long> timeout_ms = std::nullopt,
        std::optional<bool> cancel_on_timeout = std::nullopt
    )
        : enable_coroutine_creation_stack_traces_(enable_coroutine_creation_stack_traces)
        , timeout_ms_(timeout_ms)
        , cancel_on_timeout_(cancel_on_timeout)
        , debug_probes_ownership_passed_(false)
    {
        // We install the debug probes early so that the coroutines launched from the test constructor are captured as well.
        // However, this is not enough as the same extension instance may be reused several times, even cleaning up its
        // resources from the store.
        // TODO: DebugProbes.enableCreationStackTraces = enable_coroutine_creation_stack_traces_;
        // TODO: DebugProbes.install();
    }

    // Creates the CoroutinesTimeoutExtension extension with the given timeout in milliseconds.
    CoroutinesTimeoutExtension(
        long timeout_ms,
        bool cancel_on_timeout = false,
        bool enable_coroutine_creation_stack_traces = true
    )
        : CoroutinesTimeoutExtension(enable_coroutine_creation_stack_traces, timeout_ms, cancel_on_timeout)
    {
    }

    // TODO: companion object -> static methods

    // Creates the CoroutinesTimeoutExtension extension with the given timeout in seconds.
    // TODO: @JvmOverloads
    static CoroutinesTimeoutExtension seconds(
        int timeout,
        bool cancel_on_timeout = false,
        bool enable_coroutine_creation_stack_traces = true
    ) {
        return CoroutinesTimeoutExtension(
            enable_coroutine_creation_stack_traces,
            static_cast<long>(timeout) * 1000,
            cancel_on_timeout
        );
    }

private:
    bool enable_coroutine_creation_stack_traces_;
    std::optional<long> timeout_ms_;
    std::optional<bool> cancel_on_timeout_;

    // @see initialize
    std::atomic<bool> debug_probes_ownership_passed_;

    bool try_pass_debug_probes_ownership() {
        bool expected = false;
        return debug_probes_ownership_passed_.compare_exchange_strong(expected, true);
    }

public:
    // This is needed so that a class with no tests still successfully passes the ownership of DebugProbes to JUnit5.
    // TODO: override fun interceptTestClassConstructor
    template<typename T>
    T intercept_test_class_constructor(
        /* TODO: InvocationInterceptor.Invocation<T> invocation */
        /* TODO: ReflectiveInvocationContext<Constructor<T>> invocation_context */
        /* TODO: ExtensionContext extension_context */
    ) {
        // TODO: initialize(extension_context);
        // TODO: return invocation.proceed();
        throw std::runtime_error("Not implemented");
    }

    // Initialize this extension instance and/or the extension value store.
    //
    // It seems that the only way to reliably have JUnit5 clean up after its extensions is to put an instance of
    // ExtensionContext.Store.CloseableResource into the value store corresponding to the extension instance, which
    // means that DebugProbes.uninstall must be placed into the value store. debug_probes_ownership_passed_ is `true`
    // if the call to DebugProbes.install performed in the constructor of the extension instance was matched with a
    // placing of DebugProbes.uninstall into the value store. We call the process of placing the cleanup procedure
    // "passing the ownership", as now JUnit5 (and not our code) has to worry about uninstalling the debug probes.
    //
    // However, extension instances can be reused with different value stores, and value stores can be reused across
    // extension instances. This leads to a tricky scheme of performing DebugProbes.uninstall:
    //
    // - If neither the ownership of this instance's DebugProbes was yet passed nor there is any cleanup procedure
    //   stored, it means that we can just store our cleanup procedure, passing the ownership.
    // - If the ownership was not yet passed, but a cleanup procedure is already stored, we can't just replace it with
    //   another one, as this would lead to imbalance between DebugProbes.install and DebugProbes.uninstall.
    //   Instead, we know that this extension context will at least outlive this use of this instance, so some debug
    //   probes other than the ones from our constructor are already installed and won't be uninstalled during our
    //   operation. We simply uninstall the debug probes that were installed in our constructor.
    // - If the ownership was passed, but the store is empty, it means that this test instance is reused and, possibly,
    //   the debug probes installed in its constructor were already uninstalled. This means that we have to install them
    //   anew and store an uninstaller.
private:
    void initialize(/* TODO: ExtensionContext extension_context */) {
        // TODO: val store: ExtensionContext.Store = extension_context.getStore(
        //     ExtensionContext.Namespace.create(CoroutinesTimeoutExtension::class, extension_context.uniqueId))

        // It seems that the JUnit5 documentation does not specify the relationship between the extension instances and
        // the corresponding ExtensionContext (in which the value stores are managed), so it is unclear whether it's
        // theoretically possible for two extension instances that run concurrently to share an extension context. So,
        // just in case this risk exists, we synchronize here.

        // TODO: synchronized(store) {
        //     if (store["debugProbes"] == null) {
        //         if (!try_pass_debug_probes_ownership()) {
        //             // This means that the DebugProbes.install call from the constructor of this extensions has
        //             // already been matched with a corresponding cleanup procedure for JUnit5, but then JUnit5 cleaned
        //             // everything up and later reused the same extension instance for other tests. Therefore, we need to
        //             // install the DebugProbes anew.
        //             DebugProbes.enableCreationStackTraces = enable_coroutine_creation_stack_traces_;
        //             DebugProbes.install();
        //         }
        //         // put a fake resource into this extensions's store so that JUnit cleans it up, uninstalling the
        //         // DebugProbes after this extension instance is no longer needed.
        //         store.put("debugProbes", ExtensionContext.Store.CloseableResource { DebugProbes.uninstall() });
        //     } else if (!debug_probes_ownership_passed_.load()) {
        //         // This instance shares its store with other ones. Because of this, there was no need to install
        //         // DebugProbes, they are already installed, and this fact will outlive this use of this instance of
        //         // the extension.
        //         if (try_pass_debug_probes_ownership()) {
        //             // We successfully marked the ownership as passed and now may uninstall the extraneous debug probes.
        //             DebugProbes.uninstall();
        //         }
        //     }
        // }
    }

public:
    // TODO: All the interceptor methods below - JUnit5 InvocationInterceptor interface

    // TODO: override fun interceptTestMethod
    void intercept_test_method(
        /* TODO: InvocationInterceptor.Invocation<Void> invocation */
        /* TODO: ReflectiveInvocationContext<Method> invocation_context */
        /* TODO: ExtensionContext extension_context */
    ) {
        // TODO: intercept_normal_method(invocation, invocation_context, extension_context);
    }

    // TODO: override fun interceptAfterAllMethod
    void intercept_after_all_method(
        /* TODO: parameters */
    ) {
        // TODO: intercept_lifecycle_method(invocation, invocation_context, extension_context);
    }

    // TODO: override fun interceptAfterEachMethod
    void intercept_after_each_method(
        /* TODO: parameters */
    ) {
        // TODO: intercept_lifecycle_method(invocation, invocation_context, extension_context);
    }

    // TODO: override fun interceptBeforeAllMethod
    void intercept_before_all_method(
        /* TODO: parameters */
    ) {
        // TODO: intercept_lifecycle_method(invocation, invocation_context, extension_context);
    }

    // TODO: override fun interceptBeforeEachMethod
    void intercept_before_each_method(
        /* TODO: parameters */
    ) {
        // TODO: intercept_lifecycle_method(invocation, invocation_context, extension_context);
    }

    // TODO: override fun <T : Any?> interceptTestFactoryMethod
    template<typename T>
    T intercept_test_factory_method(
        /* TODO: parameters */
    ) {
        // TODO: return intercept_normal_method(invocation, invocation_context, extension_context);
        throw std::runtime_error("Not implemented");
    }

    // TODO: override fun interceptTestTemplateMethod
    void intercept_test_template_method(
        /* TODO: parameters */
    ) {
        // TODO: intercept_normal_method(invocation, invocation_context, extension_context);
    }

private:
    // TODO: Extension function on Class<T>
    template<typename T>
    std::optional<CoroutinesTimeout> coroutines_timeout_annotation(/* TODO: Class<T> */) {
        // TODO: return AnnotationSupport.findAnnotation(this, CoroutinesTimeout::class.java).let {
        //     when {
        //         it.isPresent -> it
        //         enclosingClass != null -> enclosingClass.coroutines_timeout_annotation()
        //         else -> Optional.empty()
        //     }
        // }
        return std::nullopt;
    }

    template<typename T>
    T intercept_method(
        bool use_class_annotation,
        /* TODO: InvocationInterceptor.Invocation<T> invocation */
        /* TODO: ReflectiveInvocationContext<Method> invocation_context */
        /* TODO: ExtensionContext extension_context */
    ) {
        // TODO: initialize(extension_context);

        // TODO: Full implementation with annotation finding and timeout interception
        // See original Kotlin code for complete logic

        throw std::runtime_error("Not implemented");
    }

    template<typename T>
    T intercept_normal_method(
        /* TODO: parameters */
    ) {
        // TODO: return intercept_method(true, invocation, invocation_context, extension_context);
        throw std::runtime_error("Not implemented");
    }

    void intercept_lifecycle_method(
        /* TODO: parameters */
    ) {
        // TODO: intercept_method(false, invocation, invocation_context, extension_context);
    }

    template<typename T>
    T intercept_invocation(
        /* TODO: InvocationInterceptor.Invocation<T> invocation */
        const std::string& method_name,
        long test_timeout_ms,
        bool cancel_on_timeout
    ) {
        // TODO: return run_with_timeout_dumping_coroutines(
        //     method_name, test_timeout_ms, cancel_on_timeout,
        //     []() { return CoroutinesTimeoutException(test_timeout_ms); },
        //     [&invocation]() { return invocation.proceed(); }
        // );
        throw std::runtime_error("Not implemented");
    }
};

} // namespace junit5
} // namespace debug
} // namespace coroutines
} // namespace kotlinx
