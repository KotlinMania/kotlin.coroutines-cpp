/**
 * Transliterated from: kotlinx-coroutines-core/common/src/Builders.common.kt
 */
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
#include "kotlinx/coroutines/internal/Scopes.hpp"
#include <functional>
#include <thread>
#include <memory>
#include "kotlinx/coroutines/dsl/Suspend.hpp"
#include "kotlinx/coroutines/ContinuationImpl.hpp"

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
        std::function<void*(CoroutineScope*, std::shared_ptr<Continuation<void*>>)> suspend_block_;

    public:
        LazyStandaloneCoroutine(
            std::shared_ptr<CoroutineContext> parent_context,
            std::function<void(CoroutineScope*)> block_param
        );
        // Transliterated from: kotlinx-coroutines-core/common/src/Builders.common.kt:199-208
        LazyStandaloneCoroutine(
            std::shared_ptr<CoroutineContext> parent_context,
            std::function<void*(CoroutineScope*, std::shared_ptr<Continuation<void*>>)> block_param
        );
        void on_start() override;
    };

    // Transliterated from: kotlinx-coroutines-core/common/src/Builders.common.kt:43-54
    std::shared_ptr<Job> launch(
        CoroutineScope* scope, std::shared_ptr<CoroutineContext> context,
        CoroutineStart start,
        std::function<void*(CoroutineScope*, std::shared_ptr<Continuation<void*>>)> block);

    // Transliterated from: kotlinx-coroutines-core/common/src/Builders.common.kt:43-54
    std::shared_ptr<Job> launch(
        CoroutineScope* scope,
        std::function<void*(CoroutineScope*, std::shared_ptr<Continuation<void*>>)> block);

    // Transliterated from: kotlinx-coroutines-core/common/src/Builders.common.kt:43-54
    std::shared_ptr<Job> launch(
        CoroutineScope* scope, std::shared_ptr<CoroutineContext> context,
        std::function<void*(CoroutineScope*, std::shared_ptr<Continuation<void*>>)> block);

    /**
     * DeferredCoroutine - internal implementation of Deferred
     * Transliterated from Builders.common.kt lines 94-101
     */
    template<typename T>
    class DeferredCoroutine : public AbstractCoroutine<T>, public Deferred<T> {
    private:
        std::shared_ptr<CoroutineContext> parent_context_ref;
        bool active_flag;
        std::unique_ptr<selects::SelectClause1Impl<T>> on_await_clause_;

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

        /** Returns an owned value box, or suspends until the value or failure is available. */
        // Transliterated from: kotlinx-coroutines-core/common/src/Builders.common.kt:94-101
        void* await(Continuation<void*>* continuation) override {
            auto frame = std::make_shared<AwaitValueFrame>(
                std::dynamic_pointer_cast<DeferredCoroutine<T>>(this->shared_from_this()),
                internal::retain_continuation(continuation));
            frame->retain();
            return frame->start(Result<void*>::success(nullptr));
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

        selects::SelectClause1<T>& on_await() override {
            if (!on_await_clause_) {
                on_await_clause_ = std::make_unique<selects::SelectClause1Impl<T>>(
                    this,
                    [](void* clause_object, void* select, void* param) {
                        static_cast<JobSupport*>(static_cast<DeferredCoroutine<T>*>(clause_object))->on_await_internal_reg_func(select, param);
                    },
                    [](void* clause_object, void* param, void* result) -> void* {
                        auto* state = static_cast<JobState*>(JobSupport::on_await_internal_process_res_func(clause_object, param, result));
                        auto* completed = dynamic_cast<CompletedValue<T>*>(state);
                        if (!completed) throw std::logic_error("Unexpected selected await state");
                        return new T(completed->value);
                    }
                );
            }
            return *on_await_clause_;
        }

    private:
        class AwaitValueFrame final : public ContinuationImpl {
        public:
            AwaitValueFrame(std::shared_ptr<DeferredCoroutine<T>> deferred,
                            std::shared_ptr<Continuation<void*>> completion)
                : ContinuationImpl(std::move(completion)), deferred_(std::move(deferred)) {}
            void retain() { self_ref_ = shared_from_this(); }
            void* invoke_suspend(Result<void*> result) override {
                try {
                    coroutine_begin(this)
                    coroutine_yield_value(this, result, deferred_->await_internal(this), state_);
                    self_ref_.reset();
                    auto* value = dynamic_cast<CompletedValue<T>*>(static_cast<JobState*>(state_));
                    if (!value) throw std::logic_error("Unexpected await state");
                    return new T(value->value);
                } catch (...) {
                    self_ref_.reset();
                    throw;
                }
            }
        private:
            void* _label = nullptr;
            void* state_ = nullptr; // Borrowed from the retained deferred's completion state.
            std::shared_ptr<DeferredCoroutine<T>> deferred_;
            std::shared_ptr<BaseContinuationImpl> self_ref_;
        };

    public:
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

    // Transliterated from: kotlinx-coroutines-core/common/src/Builders.common.kt:103-111
    template<typename T>
    class LazyDeferredCoroutine final : public DeferredCoroutine<T> {
        std::function<void*(CoroutineScope*, std::shared_ptr<Continuation<void*>>)> block_;
    public:
        LazyDeferredCoroutine(std::shared_ptr<CoroutineContext> context,
            std::function<void*(CoroutineScope*, std::shared_ptr<Continuation<void*>>)> block)
            : DeferredCoroutine<T>(std::move(context), false), block_(std::move(block)) {}

        void on_start() override {
            this->start(CoroutineStart::DEFAULT, static_cast<CoroutineScope*>(this), std::move(block_));
        }
    };

    // Transliterated from: kotlinx-coroutines-core/common/src/Builders.common.kt:78-89
    template<typename T>
    std::shared_ptr<Deferred<T>> async(
        CoroutineScope* scope, std::shared_ptr<CoroutineContext> context,
        CoroutineStart start,
        std::function<void*(CoroutineScope*, std::shared_ptr<Continuation<void*>>)> block) {
        if (!context) context = empty_context();
        auto new_context = new_coroutine_context(scope, std::move(context));
        std::shared_ptr<DeferredCoroutine<T>> coroutine;
        if (start == CoroutineStart::LAZY) {
            coroutine = std::make_shared<LazyDeferredCoroutine<T>>(new_context, block);
        } else {
            coroutine = std::make_shared<DeferredCoroutine<T>>(new_context, true);
        }
        coroutine->start(start, static_cast<CoroutineScope*>(coroutine.get()), std::move(block));
        return coroutine;
    }

    // Transliterated from: kotlinx-coroutines-core/common/src/Builders.common.kt:78-89
    template<typename T>
    std::shared_ptr<Deferred<T>> async(
        CoroutineScope* scope,
        std::function<void*(CoroutineScope*, std::shared_ptr<Continuation<void*>>)> block) {
        return async<T>(scope, nullptr, CoroutineStart::DEFAULT, std::move(block));
    }

    // Transliterated from: kotlinx-coroutines-core/common/src/Builders.common.kt:78-89
    template<typename T>
    std::shared_ptr<Deferred<T>> async(
        CoroutineScope* scope, std::shared_ptr<CoroutineContext> context,
        std::function<void*(CoroutineScope*, std::shared_ptr<Continuation<void*>>)> block) {
        return async<T>(scope, std::move(context), CoroutineStart::DEFAULT, std::move(block));
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

        // Upstream's `withContext` checks whether the new context introduces a new
        // ContinuationInterceptor and, if so, hops the block onto the new dispatcher
        // via DispatchedContinuation. The C++ port runs the block on the current thread
        // with the new context installed; the dispatcher hop is owned by the call site's
        // outer launch / dispatchedContinuation chain.

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
         
         // Upstream:
         //   public suspend fun <R> coroutineScope(block: suspend CoroutineScope.() -> R): R =
         //       suspendCoroutineUninterceptedOrReturn { uCont ->
         //           val coroutine = ScopeCoroutine(uCont.context, uCont)
         //           coroutine.startUndispatchedOrReturn(coroutine, block)
         //       }
         //
         // ScopeCoroutine waits for all children before completing. The C++ port uses a
         // plain SimpleScope wrapper because the AbstractCoroutine child-tracking is
         // owned by the launched coroutine itself — `coroutine_scope` callers route any
         // child waiting through their own join semantics on the AbstractCoroutine.

         if (!completion) throw std::invalid_argument("coroutine_scope requires non-null completion");
         
         internal::ContextScope scope(completion->get_context());
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
         
         if (!completion) throw std::invalid_argument("supervisor_scope requires non-null completion");
         
         internal::ContextScope scope(completion->get_context());
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
