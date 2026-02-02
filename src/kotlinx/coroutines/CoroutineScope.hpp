#pragma once
// port-lint: source CoroutineScope.kt
/**
 * @file CoroutineScope.hpp
 * @brief Scope interface for structured concurrency
 *
 * Transliterated from: kotlinx-coroutines-core/common/src/CoroutineScope.kt
 */

#include "kotlinx/coroutines/CoroutineContext.hpp"
#include "kotlinx/coroutines/Job.hpp"
#include <memory>

namespace kotlinx {
namespace coroutines {

/**
 * Defines a scope for new coroutines. Every **coroutine builder** (like [launch], [async], etc.)
 * is an extension on [CoroutineScope] and inherits its [coroutine_context][CoroutineScope::get_coroutine_context]
 * to automatically propagate all its elements and cancellation.
 *
 * The best ways to obtain a standalone instance of the scope are [create_coroutine_scope()] and [main_scope()]
 * factory functions, taking care to cancel these coroutine scopes when they are no longer needed (see the section
 * on custom usage below for explanation and example).
 *
 * Additional context elements can be appended to the scope using the [plus][CoroutineScope::plus] operator.
 *
 * ### Convention for structured concurrency
 *
 * Manual implementation of this interface is not recommended, implementation by delegation should be preferred instead.
 * By convention, the [context of a scope][CoroutineScope::get_coroutine_context] should contain an instance of a
 * [job][Job] to enforce the discipline of **structured concurrency** with propagation of cancellation.
 *
 * Every coroutine builder (like [launch], [async], and others)
 * and every scoping function (like [coroutine_scope] and [with_context]) provides _its own_ scope
 * with its own [Job] instance into the inner block of code it runs.
 * By convention, they all wait for all the coroutines inside their block to complete before completing themselves,
 * thus enforcing the structured concurrency. See [Job] documentation for more details.
 *
 * ### Custom usage
 *
 * `CoroutineScope` should be declared as a property on entities with a well-defined lifecycle that are
 * responsible for launching child coroutines. The corresponding instance of `CoroutineScope` shall be created
 * with either `create_coroutine_scope()` or `main_scope()`:
 *
 * - `create_coroutine_scope()` uses the [context][CoroutineContext] provided to it as a parameter for its coroutines
 *   and adds a [Job] if one is not provided as part of the context.
 * - `main_scope()` uses [Dispatchers::Main] for its coroutines and has a [SupervisorJob].
 *
 * **The key part of custom usage of `CoroutineScope` is cancelling it at the end of the lifecycle.**
 * The [CoroutineScope::cancel] extension function shall be used when the entity that was launching coroutines
 * is no longer needed. It cancels all the coroutines that might still be running on behalf of it.
 *
 * For example:
 *
 * ```cpp
 * class MyUIClass {
 *     std::shared_ptr<CoroutineScope> scope = main_scope(); // uses Dispatchers::Main
 *
 *     void destroy() { // destroys an instance of MyUIClass
 *         scope->cancel(); // cancels all coroutines launched in this scope
 *         // ... do the rest of cleanup here ...
 *     }
 *
 *     // Note: if this instance is destroyed or any of the launched coroutines
 *     // in this method throws an exception, then all nested coroutines are cancelled.
 *     void show_some_data() {
 *         launch(scope, [this]() { // launched in the main thread
 *             // ... here we can use suspending functions or coroutine builders with other dispatchers
 *             draw(data); // draw in the main thread
 *         });
 *     }
 * };
 * ```
 *
 * Transliterated from:
 * public interface CoroutineScope
 */
struct CoroutineScope {
    virtual ~CoroutineScope() = default;

    /**
     * The context of this scope.
     *
     * Context is encapsulated by the scope and used for implementation of coroutine builders that are extensions on the scope.
     * Accessing this property in general code is not recommended for any purposes except accessing the [Job] instance for advanced usages.
     *
     * By convention, should contain an instance of a [job][Job] to enforce structured concurrency.
     *
     * Transliterated from:
     * public val coroutineContext: CoroutineContext
     */
    virtual std::shared_ptr<CoroutineContext> get_coroutine_context() const = 0;

    /**
     * Returns the [Job] of this scope's context.
     * This is a shorthand for `get_coroutine_context()->get(Job::type_key)`.
     */
    std::shared_ptr<Job> get_job() const {
        return context_job(*get_coroutine_context());
    }
};

/**
 * A global [CoroutineScope] not bound to any job.
 * Global scope is used to launch top-level coroutines that operate
 * throughout the application's lifetime and are not canceled prematurely.
 *
 * Active coroutines launched in `GlobalScope` do not keep the process alive. They are like daemon threads.
 *
 * This is a **delicate** API. It is easy to accidentally create resource or memory leaks when
 * `GlobalScope` is used. A coroutine launched in `GlobalScope` is not subject to the principle of structured
 * concurrency, so if it hangs or gets delayed due to a problem (e.g., due to a slow network), it will stay working
 * and consuming resources. For example, consider the following code:
 *
 * ```cpp
 * void load_configuration() {
 *     launch(GlobalScope::instance(), []() {
 *         auto config = fetch_config_from_server(); // network request
 *         update_configuration(config);
 *     });
 * }
 * ```
 *
 * A call to `load_configuration` creates a coroutine in the `GlobalScope` that works in the background without any
 * provision to cancel it or to wait for its completion. If a network is slow, it keeps waiting in the background,
 * consuming resources. Repeated calls to `load_configuration` will consume more and more resources.
 *
 * ### Possible replacements
 *
 * In many circumstances, uses of 'GlobalScope' should be removed,
 * with the containing operation marked as 'suspend', for example:
 *
 * ```cpp
 * void* load_configuration(Continuation<void*>* cont) {
 *     auto config = fetch_config_from_server(cont); // network request
 *     update_configuration(config);
 *     return nullptr;
 * }
 * ```
 *
 * In cases when `GlobalScope::launch` was used to launch multiple concurrent operations, the corresponding
 * operations shall be grouped with [coroutine_scope] instead:
 *
 * ```cpp
 * // concurrently load configuration and data
 * void* load_configuration_and_data(Continuation<void*>* cont) {
 *     return coroutine_scope([](CoroutineScope& scope) {
 *         launch(scope, []() { load_configuration(); });
 *         launch(scope, []() { load_data(); });
 *     }, cont);
 * }
 * ```
 *
 * In top-level code, when launching a concurrent operation from a non-suspending context, an appropriately
 * confined instance of [CoroutineScope] shall be used instead of `GlobalScope`. See docs on [CoroutineScope] for
 * details.
 *
 * ### Legitimate use-cases
 *
 * There are limited circumstances under which `GlobalScope` can be legitimately and safely used, such as top-level background
 * processes that must stay active for the whole duration of the application's lifetime.
 *
 * @DelicateCoroutinesApi
 *
 * Transliterated from:
 * public object GlobalScope : CoroutineScope
 */
class GlobalScope : public CoroutineScope {
public:
    static GlobalScope* instance();

    /**
     * Returns [EmptyCoroutineContext].
     *
     * Transliterated from:
     * override val coroutineContext: CoroutineContext get() = EmptyCoroutineContext
     */
    std::shared_ptr<CoroutineContext> get_coroutine_context() const override;
};

// ============================================================================
// Line 99-100: CoroutineScope plus operator
// ============================================================================

/**
 * Adds the specified coroutine context to this scope, overriding existing elements
 * in the current scope's context with the corresponding keys.
 *
 * This is a shorthand for `create_coroutine_scope(this_scope.context + context)`.
 */
std::shared_ptr<CoroutineScope> operator+(
    const CoroutineScope& scope,
    const CoroutineContext& context
);

// ============================================================================
// Line 157-158: isActive property
// ============================================================================

/**
 * Returns `true` when the current [Job] is still active (has not completed and was not cancelled yet).
 *
 * This function returns `true` if there is no [job][Job] in the scope's context.
 *
 * Transliterated from:
 * public val CoroutineScope.isActive: Boolean get() = coroutineContext[Job]?.isActive ?: true
 */
inline bool is_active(const CoroutineScope& scope) {
    auto job = scope.get_job();
    return job ? job->is_active() : true;
}

// ============================================================================
// Line 307-310: cancel function
// ============================================================================

/**
 * Cancels this scope, including its job and all its children with an optional cancellation cause.
 *
 * A cause can be used to specify an error message or to provide other details on
 * a cancellation reason for debugging purposes.
 *
 * Throws std::logic_error if the scope does not have a job in it.
 *
 * Transliterated from:
 * public fun CoroutineScope.cancel(cause: CancellationException? = null)
 */
inline void cancel(CoroutineScope& scope, std::exception_ptr cause = nullptr) {
    auto job = scope.get_job();
    if (!job) {
        throw std::logic_error("Scope cannot be cancelled because it does not have a job");
    }
    job->cancel(cause);
}

/**
 * Cancels this scope with a specified diagnostic error message.
 *
 * Transliterated from:
 * public fun CoroutineScope.cancel(message: String, cause: Throwable? = null)
 */
inline void cancel(CoroutineScope& scope, const std::string& message) {
    cancel(scope, std::make_exception_ptr(std::runtime_error(message)));
}

// ============================================================================
// Line 352: ensureActive function
// ============================================================================

/**
 * Throws the CancellationException that was the scope's cancellation cause
 * if the scope is no longer active.
 *
 * Coroutine cancellation is cooperative and normally checked when a coroutine suspends.
 * ensureActive() is for code that wants to be cooperative without suspending.
 *
 * This function does nothing if there is no Job in the scope's context.
 *
 * Transliterated from:
 * public fun CoroutineScope.ensureActive(): Unit = coroutineContext.ensureActive()
 */
inline void ensure_active(const CoroutineScope& scope) {
    auto job = scope.get_job();
    if (job && !job->is_active()) {
        throw std::runtime_error("Coroutine was cancelled");  // Would be CancellationException
    }
}

// ============================================================================
// Line 298-299: CoroutineScope factory function
// ============================================================================

class ContextScope;  // Forward declaration

/**
 * Creates a [CoroutineScope] that wraps the given coroutine [context].
 *
 * If the given context does not contain a Job element, then a default Job() is created.
 * This way, failure of any child coroutine in this scope or cancellation of the scope itself
 * cancels all the scope's children, just like inside coroutine_scope block.
 *
 * Transliterated from:
 * public fun CoroutineScope(context: CoroutineContext): CoroutineScope
 */
std::shared_ptr<CoroutineScope> create_coroutine_scope(std::shared_ptr<CoroutineContext> context);

// ============================================================================
// Line 121-122: MainScope factory function
// ============================================================================

/**
 * Creates the main CoroutineScope for UI components.
 *
 * Example of use:
 * ```cpp
 * class MyAndroidActivity {
 *     std::shared_ptr<CoroutineScope> scope = main_scope();
 *
 *     void on_destroy() {
 *         cancel(*scope);
 *     }
 * };
 * ```
 *
 * The resulting scope has SupervisorJob and Dispatchers::Main context elements.
 *
 * Transliterated from:
 * public fun MainScope(): CoroutineScope = ContextScope(SupervisorJob() + Dispatchers.Main)
 */
std::shared_ptr<CoroutineScope> main_scope();

// ============================================================================
// ContextScope implementation
// ============================================================================

/**
 * Simple CoroutineScope implementation that wraps a context.
 *
 * Transliterated from: internal class ContextScope (Scopes.kt)
 */
class ContextScope : public CoroutineScope {
public:
    explicit ContextScope(std::shared_ptr<CoroutineContext> context)
        : context_(std::move(context))
    {}

    std::shared_ptr<CoroutineContext> get_coroutine_context() const override {
        return context_;
    }

private:
    std::shared_ptr<CoroutineContext> context_;
};

} // namespace coroutines
} // namespace kotlinx

