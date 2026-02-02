#pragma once
// port-lint: source CoroutineDispatcher.kt
/**
 * @file CoroutineDispatcher.hpp
 * @brief Base class for all coroutine dispatcher implementations
 *
 * Transliterated from: kotlinx-coroutines-core/common/src/CoroutineDispatcher.kt
 */

#include <string>
#include <memory>
#include "kotlinx/coroutines/context_impl.hpp"
#include "kotlinx/coroutines/ContinuationInterceptor.hpp"
#include "kotlinx/coroutines/Runnable.hpp"


namespace kotlinx {
namespace coroutines {

/**
 * Base class to be extended by all coroutine dispatcher implementations.
 *
 * If `kotlinx-coroutines` is used, it is recommended to avoid ContinuationInterceptor instances that are not
 * CoroutineDispatcher implementations, as CoroutineDispatcher ensures that the
 * debugging facilities in the new_coroutine_context function work properly.
 *
 * ## Predefined dispatchers
 *
 * The following standard implementations are provided by `kotlinx.coroutines` as properties on
 * the Dispatchers object:
 *
 * - Dispatchers::Default is used by all standard builders if no dispatcher or any other ContinuationInterceptor
 *   is specified in their context.
 *   It uses a common pool of shared background threads.
 *   This is an appropriate choice for compute-intensive coroutines that consume CPU resources.
 * - Dispatchers::IO (available on the JVM and Native targets)
 *   uses a shared pool of on-demand created threads and is designed for offloading of IO-intensive _blocking_
 *   operations (like file I/O and blocking socket I/O).
 * - Dispatchers::Main represents the UI thread if one is available.
 * - Dispatchers::Unconfined starts coroutine execution in the current call-frame until the first suspension,
 *   at which point the coroutine builder function returns.
 *   When the coroutine is resumed, the thread from which it is resumed will run the coroutine code until the next
 *   suspension, and so on.
 *   **The `Unconfined` dispatcher should not normally be used in code**.
 * - Calling limited_parallelism on any dispatcher creates a view of the dispatcher that limits the parallelism
 *   to the given value.
 *   This allows creating private thread pools without spawning new threads.
 *   For example, `Dispatchers::IO.limited_parallelism(4)` creates a dispatcher that allows running at most
 *   4 tasks in parallel, reusing the existing IO dispatcher threads.
 *
 * ## Dispatch procedure
 *
 * Typically, a dispatch procedure is performed as follows:
 *
 * - First, is_dispatch_needed is invoked to determine whether the coroutine should be dispatched
 *   or is already in the right context.
 * - If is_dispatch_needed returns `true`, the coroutine is dispatched using the dispatch method.
 *   It may take a while for the dispatcher to start the task,
 *   but the dispatch method itself may return immediately, before the task has even begun to execute.
 * - If no dispatch is needed (which is the case for Dispatchers::Main.immediate when already on the
 *   main thread and for Dispatchers::Unconfined), dispatch is typically not called,
 *   and the coroutine is resumed in the thread performing the dispatch procedure,
 *   forming an event loop to prevent stack overflows.
 *
 * Transliterated from:
 * public abstract class CoroutineDispatcher : AbstractCoroutineContextElement(ContinuationInterceptor), ContinuationInterceptor
 */
class CoroutineDispatcher : public AbstractCoroutineContextElement,
                            public ContinuationInterceptor {
public:
    static constexpr auto key_str = "ContinuationInterceptor"; // Dispatcher IS the interceptor

    CoroutineDispatcher();
    virtual ~CoroutineDispatcher() = default;

    /**
     * Returns `true` if the execution of the coroutine should be performed with dispatch method.
     * The default behavior for most dispatchers is to return `true`.
     *
     * If this method returns `false`, the coroutine is resumed immediately in the current thread,
     * potentially forming an event-loop to prevent stack overflows.
     * The event loop is an advanced topic and its implications can be found in Dispatchers::Unconfined documentation.
     *
     * A dispatcher can override this method to provide a performance optimization and avoid paying a cost of
     * an unnecessary dispatch. E.g. MainCoroutineDispatcher::immediate checks whether we are already in the
     * required UI thread in this method and avoids an additional dispatch when it is not required.
     *
     * This method should generally be exception-safe. An exception thrown from this method
     * may leave the coroutines that use this dispatcher in the inconsistent and hard to debug state.
     *
     * @param context The context of the coroutine being dispatched, or EmptyCoroutineContext if a
     *        non-coroutine-specific Runnable is dispatched instead.
     * @return true if dispatch is needed, false if coroutine can be resumed immediately
     *
     * Transliterated from:
     * public open fun isDispatchNeeded(context: CoroutineContext): Boolean = true
     */
    virtual bool is_dispatch_needed(const CoroutineContext& context) const;

    /**
     * Dispatches execution of a runnable block onto another thread in the given context.
     * This method should guarantee that the given block will be eventually invoked,
     * otherwise the system may reach a deadlock state and never leave it.
     *
     * This method should not throw exceptions. Coroutine builders like launch and async
     * catch all exceptions thrown by the coroutine block and handle them through the
     * exception handling mechanism.
     *
     * @param context The coroutine context for the dispatched block
     * @param block The runnable block to dispatch
     *
     * Transliterated from:
     * public abstract fun dispatch(context: CoroutineContext, block: Runnable)
     */
    virtual void dispatch(const CoroutineContext& context, std::shared_ptr<Runnable> block) const = 0;

    /**
     * Dispatches execution of a runnable block onto another thread in the given context
     * with a hint to the dispatcher that the current dispatch is triggered by a yield call,
     * so that the execution of this continuation may be delayed in favor of already dispatched coroutines.
     *
     * Though the Runnable contract does not provide any guarantees about the execution context,
     * this dispatch method should generally follow the same rules as dispatch.
     *
     * Transliterated from:
     * public open fun dispatchYield(context: CoroutineContext, block: Runnable): Unit = dispatch(context, block)
     */
    virtual void dispatch_yield(const CoroutineContext& context, std::shared_ptr<Runnable> block) const;
    
    // ContinuationInterceptor overrides
    CoroutineContext::Key* key() const override { return ContinuationInterceptor::type_key; }

    template <typename T>
    std::shared_ptr<Continuation<T>> intercept_continuation(std::shared_ptr<Continuation<T>> continuation);

    void release_intercepted_continuation(std::shared_ptr<ContinuationBase> continuation) override;

    virtual std::shared_ptr<CoroutineDispatcher> plus(std::shared_ptr<CoroutineDispatcher> other) {
        return other;
    }

    /**
     * Creates a view of the current dispatcher that limits the parallelism to the given value.
     * The resulting view uses the original dispatcher for execution but with the guarantee that
     * no more than `parallelism` coroutines are executed at the same time.
     *
     * This method does not impose restrictions on the number of views or the total sum of parallelism values,
     * each view controls its own parallelism independently with the guarantee that the effective parallelism
     * of all views cannot exceed the actual parallelism of the original dispatcher.
     *
     * The resulting dispatcher does not guarantee that the coroutines will always be dispatched on the same
     * subset of threads, it only guarantees that at most `parallelism` coroutines are executed at the same time,
     * and reuses threads from the original dispatchers.
     * It does not constitute a resource -- it is a _view_ of the underlying dispatcher that can be thrown away
     * and is not required to be closed.
     *
     * ### Example of usage
     * ```cpp
     * // Background dispatcher for the application
     * auto dispatcher = new_fixed_thread_pool_context(4, "App Background");
     * // At most 2 threads will be processing images as it is really slow and CPU-intensive
     * auto image_processing_dispatcher = dispatcher->limited_parallelism(2, "Image processor");
     * // At most 3 threads will be processing JSON to avoid image processing starvation
     * auto json_processing_dispatcher = dispatcher->limited_parallelism(3, "Json processor");
     * // At most 1 thread will be doing IO
     * auto file_writer_dispatcher = dispatcher->limited_parallelism(1, "File writer");
     * ```
     *
     * ### `limited_parallelism(1)` pattern
     *
     * One of the common patterns is confining the execution of specific tasks to sequential execution in background
     * with `limited_parallelism(1)` invocation. For that purpose, the implementation guarantees that tasks are
     * executed sequentially and that a happens-before relation is established between them.
     *
     * @param parallelism The maximum number of coroutines that can be executed in parallel
     * @param name Optional name for the dispatcher view (for debugging purposes)
     * @return A new dispatcher view with limited parallelism
     *
     * Transliterated from:
     * public open fun limitedParallelism(parallelism: Int, name: String? = null): CoroutineDispatcher
     */
    virtual std::shared_ptr<CoroutineDispatcher> limited_parallelism(int parallelism, const std::string& name = "");

    virtual std::string to_string() const;
};

} // namespace coroutines
} // namespace kotlinx

// Implementation of intercept_continuation moved to internal/DispatchedContinuation.hpp to avoid circular dependency
