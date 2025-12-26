#pragma once

#ifndef KOTLINX_ENABLE_STACKLESS
#error "Stackless coroutines are disabled. Set KOTLINX_ENABLE_STACKLESS=ON to enable."
#endif
/**
 * @file StacklessBuilders.hpp
 * @brief Stackless Coroutine Builders (Plugin Architecture)
 *
 * This file provides coroutine builders that execute on the StacklessScheduler.
 * They rely on the Clang suspend plugin to generate the state machines.
 */

#include "kotlinx/coroutines/internal/StacklessCoroutine.hpp"
#include "kotlinx/coroutines/Builders.hpp"
#include "kotlinx/coroutines/CoroutineScope.hpp"
#include "kotlinx/coroutines/Job.hpp"
#include "kotlinx/coroutines/Deferred.hpp"
#include "kotlinx/coroutines/dsl/Suspend.hpp"

namespace kotlinx {
namespace coroutines {
namespace stackless {

using internal::StacklessScheduler;
using internal::StacklessDispatcher;

// ============================================================================
// Runtime Initialization
// ============================================================================

inline void init() {
    // No specific init needed for simple scheduler, but kept for API compat
}

inline void shutdown() {
    StacklessScheduler::instance().shutdown();
}

// ============================================================================
// Builder Functions
// ============================================================================

/**
 * Launch a coroutine on the stackless scheduler.
 * 
 * @param body The suspend lambda to execute.
 * @return Job handle.
 */
template<typename Block>
inline std::shared_ptr<Job> launch(Block&& body) {
    // We cannot easily create a standalone scope with just a function.
    // We need a GlobalScope equivalent or create a throwaway scope.
    // For simplicity, we'll implement a standalone launch helper that creates a context.
    
    auto dispatcher = std::make_shared<StacklessDispatcher>();
    auto context = dispatcher; // Implicit cast to CoroutineContext
    
    // Create a temporary scope to use standard builders
    // Note: Standard builders require a Scope pointer usually.
    // But kotlinx::coroutines::launch takes (Scope, context, ...).
    // We can make a dummy scope.
    
    class StacklessGlobalScope : public CoroutineScope {
        std::shared_ptr<CoroutineContext> ctx_;
    public:
        StacklessGlobalScope() : ctx_(std::make_shared<StacklessDispatcher>()) {}
        std::shared_ptr<CoroutineContext> get_coroutine_context() const override { return ctx_; }
    };
    
    static StacklessGlobalScope globalScope; // Singleton scope
    
    return kotlinx::coroutines::launch(&globalScope, nullptr, CoroutineStart::DEFAULT, 
        [body = std::forward<Block>(body)](CoroutineScope* scope) {
            // We need to call the suspend body.
            // But standard launch expects a non-suspend lambda that calls suspend things?
            // No, standard launch expects a lambda that TAKES a scope and returns Unit.
            // AND that lambda IS the coroutine body, so it is treated as suspend by the plugin if it has [[suspend]]?
            // Wait, standard launch signature in Builders.hpp:
            // std::function<void(CoroutineScope*)> block
            // It doesn't take a suspend function directly in C++ signature?
            // Ah, the block passed to launch IS the entry point.
            // If the User passes a lambda, that lambda must be capable of suspending.
            // The plugin handles this if the lambda is marked [[suspend]].
            // BUT std::function type erasure might hide the [[suspend]] attribute?
            // The plugin transforms the lambda body.
            // The lambda itself must be callable with (CoroutineScope*).
            
            // If the user provided body is: [](auto scope) -> void { ... }
            // and it uses [[suspend]], it becomes a state machine.
            // The standard StandaloneCoroutine calls it.
            
            // Re-wrapping might be needed if Block signature differs.
            // Assuming Block is callable with (CoroutineScope*).
             body(scope);
        }
    );
}

// Overload for simply passing a lambda that takes no args?
// Kotlin launch block: suspend CoroutineScope.() -> Unit
// So it takes receiver.
// Our C++ launch takes (CoroutineScope*).

/**
 * Launch specialization for lambda taking explicit scope ptr.
 */
inline std::shared_ptr<Job> launch(std::function<void(CoroutineScope*)> body) {
   class StacklessGlobalScope : public CoroutineScope {
        std::shared_ptr<CoroutineContext> ctx_;
    public:
        StacklessGlobalScope() : ctx_(std::make_shared<StacklessDispatcher>()) {}
        std::shared_ptr<CoroutineContext> get_coroutine_context() const override { return ctx_; }
    };
    static StacklessGlobalScope globalScope;
    
    return kotlinx::coroutines::launch(&globalScope, nullptr, CoroutineStart::DEFAULT, body);
}


/**
 * Async builder.
 */
template<typename T, typename Block>
std::shared_ptr<Deferred<T>> async(Block&& body) {
   class StacklessGlobalScope : public CoroutineScope {
        std::shared_ptr<CoroutineContext> ctx_;
    public:
        StacklessGlobalScope() : ctx_(std::make_shared<StacklessDispatcher>()) {}
        std::shared_ptr<CoroutineContext> get_coroutine_context() const override { return ctx_; }
    };
    static StacklessGlobalScope globalScope;

    return kotlinx::coroutines::async<T>(&globalScope, nullptr, CoroutineStart::DEFAULT, 
        [body = std::forward<Block>(body)](CoroutineScope* scope) -> T {
            return body(scope);
        }
    );
}

inline void run() {
    StacklessScheduler::instance().run();
}

} // namespace stackless
} // namespace coroutines
} // namespace kotlinx
