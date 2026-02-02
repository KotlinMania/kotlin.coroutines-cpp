#pragma once
// port-lint: source Builders.common.kt
#include "kotlinx/coroutines/CoroutineScope.hpp"
#include "kotlinx/coroutines/CoroutineContext.hpp"
#include "kotlinx/coroutines/context_impl.hpp"
#include "kotlinx/coroutines/Job.hpp"
#include "kotlinx/coroutines/Deferred.hpp"
#include "kotlinx/coroutines/AbstractCoroutine.hpp"
#include "kotlinx/coroutines/EventLoop.hpp"
#include "kotlinx/coroutines/CoroutineStart.hpp"
#include "kotlinx/coroutines/Unit.hpp"
#include <functional>
#include <thread>
#include <memory>
#include "kotlinx/coroutines/dsl/Suspend.hpp"

namespace kotlinx {
namespace coroutines {

// Forward decls
    // ------------------------------------------------------------------
    // Helper Classes (Internal)
    // ------------------------------------------------------------------

    // Forward declarations - implementations in Builders.common.cpp
    class StandaloneCoroutine : public AbstractCoroutine<Unit> {
    private:
        std::shared_ptr<CoroutineContext> parent_context_ref;

    public:
        StandaloneCoroutine(std::shared_ptr<CoroutineContext> parent_context, bool active);
        bool handle_job_exception(std::exception_ptr exception) override;
    };

    class LazyStandaloneCoroutine : public StandaloneCoroutine {
    private:
        std::function<void(CoroutineScope*)> block;

    public:
        LazyStandaloneCoroutine(
            std::shared_ptr<CoroutineContext> parent_context,
            std::function<void(CoroutineScope*)> block_param
        );
        void on_start() override;
    };

    /**
     * DeferredCoroutine - internal implementation of Deferred
     * Transliterated from Builders.common.kt lines 94-101
     */
    template<typename T>
    class DeferredCoroutine : public AbstractCoroutine<T>, public Deferred<T> {
    private:
        std::shared_ptr<CoroutineContext> parent_context_ref;
        bool active_flag;

    public:
        DeferredCoroutine(std::shared_ptr<CoroutineContext> parent_context, bool active)
            : AbstractCoroutine<T>(parent_context, true, active),
              parent_context_ref(std::move(parent_context)),
              active_flag(active) {}

        /**
         * Returns completed value or throws if completed exceptionally
         * Transliterated from: override fun getCompleted(): T = getCompletedInternal() as T
         */
        T get_completed() const override {
            auto* state = JobSupport::get_completed_internal();
            // Check for exception first
            if (auto* ex = dynamic_cast<CompletedExceptionally*>(state)) {
                std::rethrow_exception(ex->cause);
            }
            // Extract value from CompletedValue wrapper
            if (auto* completed = dynamic_cast<CompletedValue<T>*>(state)) {
                return completed->value;
            }
            throw std::logic_error("Unexpected completion state");
        }

        [[nodiscard]] std::exception_ptr get_completion_exception_or_null() const override {
            auto* state = JobSupport::get_state_for_await();
            if (auto* ex = dynamic_cast<CompletedExceptionally*>(state)) {
                return ex->cause;
            }
            return nullptr;
        }

        /**
         * Suspends until completion and returns result
         * Transliterated from: override suspend fun await(): T = awaitInternal() as T
         */
        void* await(Continuation<void*>* continuation) override {
            return AbstractCoroutine<T>::await_internal(continuation);
        }

        T await_blocking() override {
            auto* state = AbstractCoroutine<T>::await_internal_blocking();
            // Check for exception first
            if (auto* ex = dynamic_cast<CompletedExceptionally*>(state)) {
                std::rethrow_exception(ex->cause);
            }
            // Extract value from CompletedValue wrapper
            if (auto* completed = dynamic_cast<CompletedValue<T>*>(state)) {
                return completed->value;
            }
            throw std::logic_error("Unexpected await state");
        }

         // Bring template start method into scope (avoids shadowing)
         using AbstractCoroutine<T>::start;

         // Job overrides from Deferred
         bool is_active() const override { return AbstractCoroutine<T>::is_active(); }
         bool is_completed() const override { return AbstractCoroutine<T>::is_completed(); }
         bool is_cancelled() const override { return AbstractCoroutine<T>::is_cancelled(); }
         std::exception_ptr get_cancellation_exception() override { return AbstractCoroutine<T>::get_cancellation_exception(); }
         void cancel(std::exception_ptr cause = nullptr) override { AbstractCoroutine<T>::cancel(cause); }
         std::shared_ptr<struct Job> get_parent() const override { return AbstractCoroutine<T>::get_parent(); }
         std::vector<std::shared_ptr<struct Job>> get_children() const override { return AbstractCoroutine<T>::get_children(); }
         std::shared_ptr<ChildHandle> attach_child(std::shared_ptr<ChildJob> child) override { return AbstractCoroutine<T>::attach_child(child); }
         void* join(Continuation<void*>* continuation) override { return AbstractCoroutine<T>::join(continuation); }
         void join_blocking() override { AbstractCoroutine<T>::join_blocking(); }
         std::shared_ptr<DisposableHandle> invoke_on_completion(std::function<void(std::exception_ptr)> handler) override { return AbstractCoroutine<T>::invoke_on_completion(handler); }
         std::shared_ptr<DisposableHandle> invoke_on_completion(bool on_cancelling, bool invoke_immediately, std::function<void(std::exception_ptr)> handler) override { return AbstractCoroutine<T>::invoke_on_completion(on_cancelling, invoke_immediately, handler); }
         CoroutineContext::Key* key() const override { return AbstractCoroutine<T>::key(); }
    };

    // ------------------------------------------------------------------
    // Builder Implementations
    // ------------------------------------------------------------------

    // Stub for empty context
    // Use shared EmptyCoroutineContext
    inline std::shared_ptr<CoroutineContext> empty_context() {
        return EmptyCoroutineContext::instance();
    }

    // Full launch with all parameters
    inline std::shared_ptr<struct Job> launch(
        CoroutineScope* scope,
        std::shared_ptr<CoroutineContext> context,
        CoroutineStart start,
        std::function<void(CoroutineScope*)> block
    ) {
        if (!context) context = empty_context();
        auto new_context = scope->get_coroutine_context()->operator+(context); 

        std::shared_ptr<StandaloneCoroutine> coroutine;
        if (start == CoroutineStart::LAZY) {
             coroutine = std::make_shared<LazyStandaloneCoroutine>(new_context, block);
        } else {
             coroutine = std::make_shared<StandaloneCoroutine>(new_context, true);
        }
        
        // Wrap block to return Unit
        std::function<Unit(CoroutineScope*)> wrapped_block = [block](CoroutineScope* s) -> Unit {
            if (block) block(s);
            return Unit();
        };

        coroutine->start(start, static_cast<CoroutineScope*>(coroutine.get()), wrapped_block);
        return coroutine;
    }

    // Overload: launch(scope, block) - no context, default start
    inline std::shared_ptr<struct Job> launch(
        CoroutineScope* scope,
        std::function<void(CoroutineScope*)> block
    ) {
        return launch(scope, nullptr, CoroutineStart::DEFAULT, block);
    }

    // Overload: launch(scope, context, block) - no start parameter
    inline std::shared_ptr<struct Job> launch(
        CoroutineScope* scope,
        std::shared_ptr<CoroutineContext> context,
        std::function<void(CoroutineScope*)> block
    ) {
        return launch(scope, context, CoroutineStart::DEFAULT, block);
    }

    template<typename T>
    std::shared_ptr<Deferred<T>> async(
        CoroutineScope* scope,
        std::shared_ptr<CoroutineContext> context,
        CoroutineStart start,
        std::function<T(CoroutineScope*)> block
    ) {
        if (!context) context = empty_context();
        auto new_context = scope->get_coroutine_context()->operator+(context);

        std::shared_ptr<DeferredCoroutine<T>> coroutine;
        coroutine = std::make_shared<DeferredCoroutine<T>>(new_context, true);
        // Cast to CoroutineScope* to match the block signature
        coroutine->start(start, static_cast<CoroutineScope*>(coroutine.get()), block);
        return coroutine;
    }

    // Overload: async(scope, block) - no context, default start
    template<typename T>
    std::shared_ptr<Deferred<T>> async(
        CoroutineScope* scope,
        std::function<T(CoroutineScope*)> block
    ) {
        return async<T>(scope, nullptr, CoroutineStart::DEFAULT, block);
    }

    // Overload: async(scope, context, block) - no start
    template<typename T>
    std::shared_ptr<Deferred<T>> async(
        CoroutineScope* scope,
        std::shared_ptr<CoroutineContext> context,
        std::function<T(CoroutineScope*)> block
    ) {
        return async<T>(scope, context, CoroutineStart::DEFAULT, block);
    }

    // Transliterated from Builders.common.kt: suspend fun <T> withContext(context: CoroutineContext, block: suspend CoroutineScope.() -> T): T
    // C++: [[suspend]] void* with_context(context, block, completion)
    //
    // This is a suspend function. In C++, it must be called from within a suspend
    // state machine.
    //
    // Preferred Kotlin‑aligned DSL:
    //   [[suspend]] void* foo(std::shared_ptr<Continuation<void*>> completion) {
    //       auto r = suspend(with_context(ctx, block, completion));
    //       ...
    //   }
    // The Clang suspend plugin rewrites the DSL into a Kotlin‑Native‑shape state machine.
    //
    // Manual state machines may still call this directly and propagate
    // COROUTINE_SUSPENDED explicitly.
    //
    // Implementation pattern from NativeSuspendFunctionLowering.kt:
    // - State machine with label field tracks suspension points
    // - invoke_suspend() is the state machine method
    // - ContinuationImpl is the base class
    //
    // @param context The context to switch to
    // @param block The suspend block to execute
    // @param continuation The continuation to resume (passed by state machine)
    // @return Result or COROUTINE_SUSPENDED
    // Implemented as a suspend function that handles context switching logic.

    template<typename T, typename Block>
    void* with_context(
        std::shared_ptr<CoroutineContext> context,
        Block&& block,
        std::shared_ptr<Continuation<void*>> completion
    ) {
        using namespace kotlinx::coroutines::dsl;

        if (!context) {
            throw std::invalid_argument("with_context requires a non-null context");
        }
        if (!completion) {
            throw std::invalid_argument("with_context requires non-null completion");
        }

        // TODO: Implement full context switching logic (check dispatcher)
        // For now, we execute in the given context (assuming block handles it or just scope wrapping)
        // This simplistically matches the previous stub but with correct ABI.

        // Create a scope with the given context
        class WithContextScope : public CoroutineScope {
            std::shared_ptr<CoroutineContext> ctx_;
        public:
            explicit WithContextScope(std::shared_ptr<CoroutineContext> ctx) : ctx_(ctx) {}
            std::shared_ptr<CoroutineContext> get_coroutine_context() const override { return ctx_; }
        };
        WithContextScope scope(context);

        // Execute the block.
        // Block is expected to accept (CoroutineScope*, std::shared_ptr<Continuation<void*>>)
        // and return either a boxed result or COROUTINE_SUSPENDED.
        auto result = suspend(block(&scope, completion));
        return result;
    }

    /**
     * coroutine_scope builder.
     * Creates a CoroutineScope and calls the specified suspend block with this scope.
     * The provided scope inherits its coroutine_context from the outer scope, but overrides
     * the Job context element to ensure that it cancels all children when the scope is cancelled.
     * Use this function to manage the lifecycle of concurrent operations.
     */
    template<typename T, typename Block>
    void* coroutine_scope(
        Block&& block,
        std::shared_ptr<Continuation<void*>> completion
    ) {
         using namespace kotlinx::coroutines::dsl;
         
         // TODO: Use internal::ScopeCoroutine to properly wait for children.
         // For now, we reuse the caller's context and just create a scope wrapper.
         // This does NOT wait for children launched in this scope!
         
         if (!completion) throw std::invalid_argument("coroutine_scope requires non-null completion");
         
         class SimpleScope : public CoroutineScope {
             std::shared_ptr<CoroutineContext> ctx_;
         public:
             explicit SimpleScope(std::shared_ptr<CoroutineContext> ctx) : ctx_(ctx) {}
             std::shared_ptr<CoroutineContext> get_coroutine_context() const override { return ctx_; }
         };
         
         SimpleScope scope(completion->get_context());
         return suspend(block(&scope, completion));
    }

     /**
     * supervisor_scope builder.
     * Creates a CoroutineScope with SupervisorJob and calls the specified suspend block with this scope.
     * The children failure does not cause this scope to fail and does not affect other children.
     */
    template<typename T, typename Block>
    void* supervisor_scope(
        Block&& block,
        std::shared_ptr<Continuation<void*>> completion
    ) {
         using namespace kotlinx::coroutines::dsl;
         
         // TODO: Use SupervisorCoroutine.
         // Currently stubbed similarly to coroutine_scope.
         
         if (!completion) throw std::invalid_argument("supervisor_scope requires non-null completion");
         
         class SimpleScope : public CoroutineScope {
             std::shared_ptr<CoroutineContext> ctx_;
         public:
             explicit SimpleScope(std::shared_ptr<CoroutineContext> ctx) : ctx_(ctx) {}
             std::shared_ptr<CoroutineContext> get_coroutine_context() const override { return ctx_; }
         };
         
         // TODO: Add SupervisorJob to context
         SimpleScope scope(completion->get_context());
         return suspend(block(&scope, completion));
    }

    // BlockingCoroutine implementation
    // Transliterated from Builders.kt: private class BlockingCoroutine<T>
    template<typename T>
    class BlockingCoroutine : public AbstractCoroutine<T> {
        std::shared_ptr<EventLoop> event_loop_;
        T result_;
        std::exception_ptr exception_;
        bool completed_ = false;

    public:
        BlockingCoroutine(std::shared_ptr<CoroutineContext> parent_context, std::shared_ptr<EventLoop> event_loop)
            : AbstractCoroutine<T>(parent_context, true, true),
              event_loop_(event_loop) {}

        void on_completed(T value) override {
            result_ = std::move(value);
            completed_ = true;
            if (auto blocking_loop = std::dynamic_pointer_cast<BlockingEventLoop>(event_loop_)) {
                blocking_loop->shutdown();
            }
        }

        void on_cancelled(std::exception_ptr cause, bool handled) override {
            exception_ = cause;
            completed_ = true;
            if (auto blocking_loop = std::dynamic_pointer_cast<BlockingEventLoop>(event_loop_)) {
                blocking_loop->shutdown();
            }
        }

        /**
         * Blocks until completion and returns the result.
         * This is NOT an override of Job::join_blocking() - it's a separate method
         * that returns the result, matching Kotlin's BlockingCoroutine.joinBlocking(): T
         */
        T join_blocking_with_result() {
            if (auto blocking_loop = std::dynamic_pointer_cast<BlockingEventLoop>(event_loop_)) {
                blocking_loop->run();
            }
            if (exception_) {
                std::rethrow_exception(exception_);
            }
            return std::move(result_);
        }
    };

    template<typename T>
    T run_blocking(
        std::shared_ptr<CoroutineContext> context,
        std::function<T(CoroutineScope*)> block
    ) {
         if (!context) context = empty_context();

         auto event_loop = std::make_shared<BlockingEventLoop>(nullptr);
         auto old_loop = ThreadLocalEventLoop::current_or_null();
         ThreadLocalEventLoop::set_event_loop(event_loop);
         
         try {
             auto new_context = context->operator+(event_loop);
             auto coroutine = std::make_shared<BlockingCoroutine<T>>(new_context, event_loop);
             
             // Cast to CoroutineScope* to match block signature - R must be consistent
             coroutine->start(CoroutineStart::DEFAULT, static_cast<CoroutineScope*>(coroutine.get()), block);

             T result = coroutine->join_blocking_with_result();
             
             ThreadLocalEventLoop::set_event_loop(old_loop);
             return result;
         } catch (...) {
             ThreadLocalEventLoop::set_event_loop(old_loop);
             throw;
         }
    }


} // namespace coroutines
} // namespace kotlinx
