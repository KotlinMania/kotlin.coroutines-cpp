#pragma once
#include <string>
#include <memory>
#include "kotlinx/coroutines/core_fwd.hpp"
#include "kotlinx/coroutines/context_impl.hpp"
#include "kotlinx/coroutines/ContinuationInterceptor.hpp"

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
     
    // We can just use ContinuationInterceptor::key for the constructor
    
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

    // Intercept
     // Note: In C++ templates, we might need explicit implementation here or in .cpp if not templated on T
     // But intercept_continuation is generic <T>.
    // virtual std::shared_ptr<Continuation<T>> intercept_continuation(std::shared_ptr<Continuation<T>> continuation) override; 
    // Cannot override template method virtual? 
    // C++ doesn't support virtual template methods. 
    // The interface ContinuationInterceptor must have had a non-template generic or we cast.
    // For now, let's keep it as is in provided HPP but make sure it works.
    
    // Actually, ContinuationInterceptor.hpp I wrote has `template <typename T> intercept_continuation`.
    // This cannot be virtual.
    // The design in C++ for this usually involves type erasure or base classes.
    // For this task, I'll stick to the "likely intended" structure where it's non-virtual in the interface 
    // (if the interface is just a concept) OR we assume type erasure.
    // But wait, the user code has `override fun <T> interceptContinuation(...)`.
    // Kotlin generic methods in interfaces ARE virtual. C++ cannot do this.
    // We must assume `ContinuationInterceptor` uses a base class `ContinuationBase` for interception if we want runtime polymorphism,
    // OR we relies on `AbstractCoroutineContextElement::get` returning correct type.
    
    // Correct C++ mapping for `interceptContinuation`:
    // It's likely NOT virtual in the C++ `ContinuationInterceptor` base if it's templated. 
    // But `CoroutineDispatcher` is a specific type.
    // Intercept
    template <typename T>
    std::shared_ptr<Continuation<T>> intercept_continuation(std::shared_ptr<Continuation<T>> continuation);

    void release_intercepted_continuation(std::shared_ptr<ContinuationBase> continuation) override;

    /**
     * @suppress **Error**: Operator '+' on two CoroutineDispatcher objects is meaningless.
     * CoroutineDispatcher is a coroutine context element and `+` is a set-sum operator for coroutine contexts.
     * The dispatcher to the right of `+` just replaces the dispatcher to the left.
     */
    virtual std::shared_ptr<CoroutineDispatcher> plus(std::shared_ptr<CoroutineDispatcher> other) {
        return other;
    }

    // Limited Parallelism
    virtual std::shared_ptr<CoroutineDispatcher> limited_parallelism(int parallelism, const std::string& name = "");

    virtual std::string to_string() const;
};

} // namespace coroutines
} // namespace kotlinx

#include "kotlinx/coroutines/DispatchedContinuation.hpp"

namespace kotlinx {
namespace coroutines {

template <typename T>
std::shared_ptr<Continuation<T>> CoroutineDispatcher::intercept_continuation(std::shared_ptr<Continuation<T>> continuation) {
    return std::make_shared<DispatchedContinuation<T>>(shared_from_this(), continuation);
}

} // namespace coroutines
} // namespace kotlinx
