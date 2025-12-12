#pragma once
#include "kotlinx/coroutines/CoroutineScope.hpp"
#include "kotlinx/coroutines/CoroutineContext.hpp"
#include "kotlinx/coroutines/context_impl.hpp"
#include "kotlinx/coroutines/Job.hpp"
#include "kotlinx/coroutines/Deferred.hpp"
#include "kotlinx/coroutines/AbstractCoroutine.hpp"
#include "kotlinx/coroutines/EventLoop.hpp"
#include "kotlinx/coroutines/CoroutineStart.hpp"
#include "kotlinx/coroutines/Unit.hpp"
#include "kotlinx/coroutines/CancellableContinuationImpl.hpp"
#include "kotlinx/coroutines/DispatchedContinuation.hpp"
#include "kotlinx/coroutines/internal/ScopeCoroutine.hpp"
#include "kotlinx/coroutines/internal/DispatchedTask.hpp"
#include "kotlinx/coroutines/intrinsics/Intrinsics.hpp"
#include "kotlinx/coroutines/selects/Select.hpp"
#include <functional>
#include <thread>
#include <memory>

namespace kotlinx {
namespace coroutines {

// Forward decls
    // ------------------------------------------------------------------
    // Helper Classes (Internal)
    // ------------------------------------------------------------------

    class StandaloneCoroutine : public AbstractCoroutine<Unit> {
    private:
        std::shared_ptr<CoroutineContext> parent_context_ref;
        bool active_flag;

    public:
        StandaloneCoroutine(std::shared_ptr<CoroutineContext> parentContext, bool active)
            : AbstractCoroutine<Unit>(parentContext, true, active),
              parent_context_ref(parentContext),
              active_flag(active) {}

        bool handle_job_exception(std::exception_ptr exception) override {
            /*
             * TODO: STUB - Exception handling not implemented
             *
             * Kotlin source: StandaloneCoroutine.handleJobException() in Builders.common.kt
             *
             * What's missing:
             * - Should call handleCoroutineException(context, exception) to propagate
             *   the exception to the CoroutineExceptionHandler in the context
             * - handleCoroutineException() needs to be implemented first
             *
             * Current behavior: Silently swallows exception and returns true (handled)
             * Correct behavior: Propagate to CoroutineExceptionHandler, then return true
             */
            (void)exception;
            return true;
        }
    };

    class LazyStandaloneCoroutine : public StandaloneCoroutine {
    private:
        std::function<void(CoroutineScope*)> block;

    public:
        LazyStandaloneCoroutine(
            std::shared_ptr<CoroutineContext> parentContext,
            std::function<void(CoroutineScope*)> block_param
        ) : StandaloneCoroutine(parentContext, false),
            block(block_param) {}

        void on_start() override {
            /*
             * TODO: STUB - Lazy coroutine startup not implemented
             *
             * Kotlin source: LazyStandaloneCoroutine.onStart() in Builders.common.kt
             *
             * What's missing:
             * - Should call: continuation.startCoroutineCancellable(this)
             * - startCoroutineCancellable() wraps the block in a CancellableContinuation
             *   and dispatches it through the coroutine's dispatcher
             * - Requires: CancellableContinuationImpl, DispatchedContinuation integration
             *
             * Current behavior: Does nothing - lazy coroutine never actually starts
             * Correct behavior: Start the coroutine block with cancellation support
             *
             * Workaround: Use CoroutineStart::DEFAULT instead of LAZY for now
             */
        }
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
        DeferredCoroutine(std::shared_ptr<CoroutineContext> parentContext, bool active)
            : AbstractCoroutine<T>(parentContext, true, active),
              parent_context_ref(parentContext),
              active_flag(active) {}

        /**
         * Returns completed value or throws if completed exceptionally
         * Transliterated from: override fun getCompleted(): T = getCompletedInternal() as T
         */
        T get_completed() const override {
            return this->get_completed_internal();
        }
        
        std::exception_ptr get_completion_exception_or_null() const override {
            auto state = this->state;
            if (auto* ex = dynamic_cast<CompletedExceptionally*>(state.get())) {
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
            return *reinterpret_cast<T*>(state);
        }

         // Job overrides from Deferred
         bool is_active() const override { return AbstractCoroutine<T>::is_active(); }
         bool is_completed() const override { return AbstractCoroutine<T>::is_completed(); }
         bool is_cancelled() const override { return AbstractCoroutine<T>::is_cancelled(); }
         std::exception_ptr get_cancellation_exception() override { return AbstractCoroutine<T>::get_cancellation_exception(); }
         bool start() override { return AbstractCoroutine<T>::start(); }
         void cancel(std::exception_ptr cause = nullptr) override { AbstractCoroutine<T>::cancel(cause); }
         std::shared_ptr<struct Job> get_parent() const override { return AbstractCoroutine<T>::get_parent(); }
         std::vector<std::shared_ptr<struct Job>> get_children() const override { return AbstractCoroutine<T>::get_children(); }
         std::shared_ptr<DisposableHandle> attach_child(std::shared_ptr<ChildJob> child) override { return AbstractCoroutine<T>::attach_child(child); }
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

    inline std::shared_ptr<struct Job> launch(
        CoroutineScope* scope,
        std::shared_ptr<CoroutineContext> context = nullptr,
        CoroutineStart start = CoroutineStart::DEFAULT,
        std::function<void(CoroutineScope*)> block = nullptr
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

    template<typename T>
    std::shared_ptr<Deferred<T>> async(
        CoroutineScope* scope,
        std::shared_ptr<CoroutineContext> context = nullptr,
        CoroutineStart start = CoroutineStart::DEFAULT,
        std::function<T(CoroutineScope*)> block = nullptr
    ) {
        if (!context) context = empty_context();
        auto new_context = scope->get_coroutine_context()->operator+(context);

        std::shared_ptr<DeferredCoroutine<T>> coroutine;
        // implementation ...
        coroutine = std::make_shared<DeferredCoroutine<T>>(new_context, true);
        coroutine->start(start, coroutine.get(), block);
        return coroutine;
    }

    // Transliterated from Builders.common.kt: suspend fun <T> withContext(context: CoroutineContext, block: suspend CoroutineScope.() -> T): T
    //
    // This is a suspend function. In C++, it must be called from within a suspend
    // state machine.
    //
    // Preferred Kotlin‑aligned DSL:
    //   [[suspend]] void* foo(Continuation<void*>* c) {
    //       auto r = suspend(with_context(ctx, block, c));
    //       ...
    //   }
    // The Clang suspend plugin rewrites the DSL into a Kotlin‑Native‑shape state machine.
    //
    // Manual state machines may still call this directly and propagate
    // COROUTINE_SUSPENDED explicitly.
    //
    // Implementation pattern from NativeSuspendFunctionLowering.kt:
    // - State machine with label field tracks suspension points
    // - invokeSuspend() is the state machine method
    // - ContinuationImpl is the base class
    //
    // @param context The context to switch to
    // @param block The suspend block to execute
    // @param continuation The continuation to resume (passed by state machine)
    // @return Result or COROUTINE_SUSPENDED
    template<typename T>
    void* with_context(
        std::shared_ptr<CoroutineContext> context,
        std::function<void*(CoroutineScope*, Continuation<void*>*)> block,
        Continuation<void*>* continuation
    ) {
        if (!context) {
            throw std::invalid_argument("withContext requires a non-null context");
        }

        // Create a scope with the given context
        class WithContextScope : public CoroutineScope {
            std::shared_ptr<CoroutineContext> ctx_;
        public:
            explicit WithContextScope(std::shared_ptr<CoroutineContext> ctx) : ctx_(ctx) {}
            std::shared_ptr<CoroutineContext> get_coroutine_context() const override { return ctx_; }
        };

        WithContextScope scope(context);

        // Execute the block, passing the continuation for proper suspend handling
        // The block itself may suspend, returning COROUTINE_SUSPENDED
        return block(&scope, continuation);
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

        T join_blocking() {
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

         auto eventLoop = std::make_shared<BlockingEventLoop>(nullptr);
         auto oldLoop = ThreadLocalEventLoop::current_or_null();
         ThreadLocalEventLoop::set_event_loop(eventLoop);
         
         try {
             auto newContext = context->operator+(eventLoop); 
             auto coroutine = std::make_shared<BlockingCoroutine<T>>(newContext, eventLoop);
             
             coroutine->start(CoroutineStart::DEFAULT, coroutine.get(), block);
             
             T result = coroutine->join_blocking();
             
             ThreadLocalEventLoop::set_event_loop(oldLoop);
             return result;
         } catch (...) {
             ThreadLocalEventLoop::set_event_loop(oldLoop);
             throw;
         }
    }


} // namespace coroutines
} // namespace kotlinx
