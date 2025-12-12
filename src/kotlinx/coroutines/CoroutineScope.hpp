#pragma once
#include "kotlinx/coroutines/CoroutineContext.hpp"
#include <memory>

namespace kotlinx {
namespace coroutines {

/**
 * Defines a scope for new coroutines. Every **coroutine builder** (like [launch], [async], etc.)
 * is an extension on [CoroutineScope] and inherits its [coroutineContext]
 * to automatically propagate all its elements and cancellation.
 *
 * ### Convention for structured concurrency
 *
 * Manual implementation of this interface is not recommended, implementation by delegation should be preferred.
 * By convention, the scope context should contain an instance of a [Job] to enforce the discipline
 * of **structured concurrency** with propagation of cancellation.
 *
 * Every coroutine builder and scoping function provides _its own_ scope with its own [Job] instance
 * into the inner block of code it runs. By convention, they all wait for all the coroutines inside
 * their block to complete before completing themselves.
 *
 * ### Custom usage
 *
 * `CoroutineScope` should be declared as a property on entities with a well-defined lifecycle.
 * **The key part of custom usage is cancelling it at the end of the lifecycle.**
 *
 * ```cpp
 * class MyClass {
 *     CoroutineScope scope = MainScope(); 
 *     void destroy() { scope.cancel(); }
 * };
 * ```
 */
struct CoroutineScope {
    virtual ~CoroutineScope() = default;

    /**
     * The context of this scope.
     * Context is encapsulated by the scope and used for implementation of coroutine builders that are extensions on the scope.
     */
    virtual std::shared_ptr<CoroutineContext> get_coroutine_context() const = 0;
};

// GlobalScope definition 
// Singleton usually
class GlobalScope : public CoroutineScope {
public:
    static GlobalScope* instance();
    std::shared_ptr<CoroutineContext> get_coroutine_context() const override;
};

} // namespace coroutines
} // namespace kotlinx
