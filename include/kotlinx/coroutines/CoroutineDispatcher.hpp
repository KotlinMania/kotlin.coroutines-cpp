#pragma once
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
 * If `kotlinx-coroutines` is used, it is recommended to avoid [ContinuationInterceptor] instances that are not
 * [CoroutineDispatcher] implementations, as [CoroutineDispatcher] ensures that the
 * debugging facilities in the [newCoroutineContext] function work properly.
 *
 * ## Predefined dispatchers
 *
 * The following standard implementations are provided by `kotlinx.coroutines` as properties on
 * the [Dispatchers] object:
 *
 * - [Dispatchers::Default] is used by all standard builders if no dispatcher or any other [ContinuationInterceptor]
 *   is specified in their context.
 *   It uses a common pool of shared background threads.
 *   This is an appropriate choice for compute-intensive coroutines that consume CPU resources.
 * - `Dispatchers::IO` (available on the JVM and Native targets)
 *   uses a shared pool of on-demand created threads and is designed for offloading of IO-intensive _blocking_
 *   operations (like file I/O and blocking socket I/O).
 * - [Dispatchers::Main] represents the UI thread if one is available.
 * - [Dispatchers::Unconfined] starts coroutine execution in the current call-frame until the first suspension,
 *   at which point the coroutine builder function returns.
 *   When the coroutine is resumed, the thread from which it is resumed will run the coroutine code until the next
 *   suspension, and so on.
 *   **The `Unconfined` dispatcher should not normally be used in code**.
 * - Calling [limitedParallelism] on any dispatcher creates a view of the dispatcher that limits the parallelism
 *   to the given value.
 *   This allows creating private thread pools without spawning new threads.
 *   For example, `Dispatchers::IO.limitedParallelism(4)` creates a dispatcher that allows running at most
 *   4 tasks in parallel, reusing the existing IO dispatcher threads.
 */
class CoroutineDispatcher : public AbstractCoroutineContextElement, 
                            public ContinuationInterceptor,
                            public std::enable_shared_from_this<CoroutineDispatcher> {
public:
    static constexpr const char* keyStr = "ContinuationInterceptor"; // Dispatcher IS the interceptor
     // Re-use ContinuationInterceptor key or define its own? 
     // Kotlin: Key : AbstractCoroutineContextKey<ContinuationInterceptor, CoroutineDispatcher>(ContinuationInterceptor)
     // essentially it uses ContinuationInterceptor::key.
    
    // AbstractCoroutineContextElement constructor usually takes a Key*
    CoroutineDispatcher() : AbstractCoroutineContextElement(ContinuationInterceptor::typeKey) {}
    virtual ~CoroutineDispatcher() = default;

    virtual bool is_dispatch_needed(const CoroutineContext& context) const {
        return true;
    }

    virtual void dispatch(const CoroutineContext& context, std::shared_ptr<Runnable> block) const = 0;

    virtual void dispatch_yield(const CoroutineContext& context, std::shared_ptr<Runnable> block) const {
        dispatch(context, block);
    }
    
    // ContinuationInterceptor overrides
    CoroutineContext::Key* key() const override { return ContinuationInterceptor::typeKey; }

    template <typename T>
    std::shared_ptr<Continuation<T>> intercept_continuation(std::shared_ptr<Continuation<T>> continuation);

    void release_intercepted_continuation(std::shared_ptr<ContinuationBase> continuation) override;

    virtual std::shared_ptr<CoroutineDispatcher> plus(std::shared_ptr<CoroutineDispatcher> other) {
        return other;
    }

    // Limited Parallelism
    virtual std::shared_ptr<CoroutineDispatcher> limited_parallelism(int parallelism, const std::string& name = "");

    virtual std::string to_string() const;
};

} // namespace coroutines
} // namespace kotlinx

// Implementation of intercept_continuation moved to DispatchedContinuation.hpp to avoid circular dependency
