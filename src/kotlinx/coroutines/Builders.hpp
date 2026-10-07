/**
 * Transliterated from: kotlinx-coroutines-core/common/src/Builders.common.kt
 */
#pragma once
// port-lint: source kotlinx-coroutines-core/common/src/Builders.common.kt
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
#include "kotlinx/coroutines/UndispatchedCoroutine.hpp"
#include "kotlinx/coroutines/DispatchedCoroutine.hpp"
#include "kotlinx/coroutines/Supervisor.hpp"
#include "kotlinx/coroutines/common/CoroutineContextUtils.hpp"
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

    // Transliterated from: kotlinx-coroutines-core/common/src/Builders.common.kt:43-54
    // NOTE(port): Ordinary C++ blocks enter the same suspend builder and return erased Unit.
    inline std::shared_ptr<Job> launch(CoroutineScope* scope, std::shared_ptr<CoroutineContext> context,
        CoroutineStart start, std::function<void(CoroutineScope*)> block) {
        return launch(scope, std::move(context), start,
            std::function<void*(CoroutineScope*, std::shared_ptr<Continuation<void*>>)>(
                [block = std::move(block)](CoroutineScope* receiver, std::shared_ptr<Continuation<void*>>) -> void* {
                    block(receiver);
                    return nullptr;
                }));
    }

    // Overload: launch(scope, block) - no context, default start
    inline std::shared_ptr<struct Job> launch(
        CoroutineScope* scope,
        std::function<void(CoroutineScope*)> block
    ) {
        return launch(scope, EmptyCoroutineContext::instance(), CoroutineStart::DEFAULT, std::move(block));
    }

    // Overload: launch(scope, context, block) - no start parameter
    inline std::shared_ptr<struct Job> launch(
        CoroutineScope* scope,
        std::shared_ptr<CoroutineContext> context,
        std::function<void(CoroutineScope*)> block
    ) {
        return launch(scope, context, CoroutineStart::DEFAULT, block);
    }

    // Transliterated from: kotlinx-coroutines-core/common/src/Builders.common.kt:78-89
    template<typename T>
    std::shared_ptr<Deferred<T>> async(CoroutineScope* scope, std::shared_ptr<CoroutineContext> context,
        CoroutineStart start, std::function<void*(CoroutineScope*, std::shared_ptr<Continuation<void*>>)> block);

    // Transliterated from: kotlinx-coroutines-core/common/src/Builders.common.kt:78-89
    // NOTE(port): An ordinary C++ result is boxed for the existing suspend entry.
    template<typename T>
    std::shared_ptr<Deferred<T>> async(CoroutineScope* scope, std::shared_ptr<CoroutineContext> context,
        CoroutineStart start, std::function<T(CoroutineScope*)> block) {
        return async<T>(scope, std::move(context), start,
            std::function<void*(CoroutineScope*, std::shared_ptr<Continuation<void*>>)>(
                [block = std::move(block)](CoroutineScope* receiver, std::shared_ptr<Continuation<void*>>) -> void* {
                    if constexpr (std::is_same_v<T, Unit>) { block(receiver); return nullptr; }
                    else return new T(block(receiver));
                }));
    }

    // Overload: async(scope, block) - no context, default start
    template<typename T>
    std::shared_ptr<Deferred<T>> async(
        CoroutineScope* scope,
        std::function<T(CoroutineScope*)> block
    ) {
        return async<T>(scope, EmptyCoroutineContext::instance(), CoroutineStart::DEFAULT, std::move(block));
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
        return async<T>(scope, EmptyCoroutineContext::instance(), CoroutineStart::DEFAULT, std::move(block));
    }

    // Transliterated from: kotlinx-coroutines-core/common/src/Builders.common.kt:78-89
    template<typename T>
    std::shared_ptr<Deferred<T>> async(
        CoroutineScope* scope, std::shared_ptr<CoroutineContext> context,
        std::function<void*(CoroutineScope*, std::shared_ptr<Continuation<void*>>)> block) {
        return async<T>(scope, std::move(context), CoroutineStart::DEFAULT, std::move(block));
    }

    /**
     * Calls the specified suspending block with a given coroutine context, suspends until it completes, and returns
     * the result.
     *
     * The resulting context for the [block] is derived by merging the current [coroutineContext] with the
     * specified [context] using `coroutineContext + context` (see [CoroutineContext.plus]).
     * This suspending function is cancellable. It immediately checks for cancellation of
     * the resulting context and throws [CancellationException] if it is not [active][CoroutineContext.isActive].
     *
     * Calls to [withContext] whose [context] argument provides a [CoroutineDispatcher] that is
     * different from the current one, by necessity, perform additional dispatches: the [block]
     * can not be executed immediately and needs to be dispatched for execution on
     * the passed [CoroutineDispatcher], and then when the [block] completes, the execution
     * has to shift back to the original dispatcher.
     *
     * Note that the result of `withContext` invocation is dispatched into the original context in a cancellable way
     * with a **prompt cancellation guarantee**, which means that if the original [coroutineContext]
     * in which `withContext` was invoked is cancelled by the time its dispatcher starts to execute the code,
     * it discards the result of `withContext` and throws [CancellationException].
     *
     * The cancellation behaviour described above is enabled if and only if the dispatcher is being changed.
     * For example, when using `withContext(NonCancellable) { ... }` there is no change in dispatcher and
     * this call will not be cancelled neither on entry to the block inside `withContext` nor on exit from it.
     */
    // Transliterated from: kotlinx-coroutines-core/common/src/Builders.common.kt:140-173
    template<typename T, typename Block>
    void* with_context(std::shared_ptr<CoroutineContext> context, Block&& block,
                       std::shared_ptr<Continuation<void*>> completion) {
        using Value = std::conditional_t<std::is_void_v<T>, Unit, T>;
        auto old_context = completion->get_context();
        auto new_context = new_coroutine_context(old_context, std::move(context));
        context_ensure_active(*new_context);
        auto typed_completion = internal::result_box_completion<Value>(completion);
        auto start_block = [&block](auto coroutine) -> void* {
            return coroutine->start_undispatched_or_return(
                [coroutine, block = std::forward<Block>(block)](std::shared_ptr<Continuation<void*>> continuation) mutable -> void* {
                    return block(static_cast<CoroutineScope*>(coroutine.get()), std::move(continuation));
                });
        };
        // FAST PATH #1: The new context is the same as the old one.
        if (new_context == old_context)
            return start_block(std::make_shared<internal::ScopeCoroutine<Value>>(new_context, typed_completion));
        auto new_interceptor = new_context->get(ContinuationInterceptor::type_key);
        auto old_interceptor = old_context->get(ContinuationInterceptor::type_key);
        // Equality is used by design for dispatcher wrappers.
        if (new_interceptor ? new_interceptor->equals(old_interceptor.get()) : !old_interceptor) {
            auto coroutine = std::make_shared<UndispatchedCoroutine<Value>>(new_context, typed_completion);
            return with_coroutine_context<void*>(coroutine->get_context(), nullptr,
                [&]() -> void* { return start_block(coroutine); });
        }
        // SLOW PATH: Use the new dispatcher and switch back cancellably.
        auto coroutine = std::make_shared<DispatchedCoroutine<Value>>(new_context, typed_completion);
        coroutine->start(CoroutineStart::DEFAULT, static_cast<CoroutineScope*>(coroutine.get()),
            std::function<void*(CoroutineScope*, std::shared_ptr<Continuation<void*>>)>(std::forward<Block>(block)));
        return coroutine->get_result();
    }

    /**
     * Calls the specified suspending block with the given CoroutineDispatcher,
     * suspends until it completes, and returns the result. This calls with_context.
     */
    // Transliterated from: kotlinx-coroutines-core/common/src/Builders.common.kt:181-183
    template <typename T, typename Block>
    void* invoke(std::shared_ptr<CoroutineDispatcher> dispatcher, Block&& block,
                 std::shared_ptr<Continuation<void*>> completion) {
        return with_context<T>(std::move(dispatcher), std::forward<Block>(block), std::move(completion));
    }

    /**
     * Creates a [CoroutineScope] and calls the specified suspend block with this scope.
     * The provided scope inherits its [coroutineContext][CoroutineScope.coroutineContext] from the outer scope, using the
     * [Job] from that context as the parent for a new [Job].
     *
     * This function is designed for _concurrent decomposition_ of work. When any child coroutine in this scope fails,
     * this scope fails, cancelling all the other children (for a different behavior, see [supervisorScope]).
     * This function returns as soon as the given block and all its child coroutines are completed.
     * A usage of a scope looks like this:
     *
     * ```kotlin
     * suspend fun showSomeData() = coroutineScope {
     *     val data = async(Dispatchers.IO) { // <- extension on current scope
     *      ... load some UI data for the Main thread ...
     *     }
     *
     *     withContext(Dispatchers.Main) {
     *         doSomeWork()
     *         val result = data.await()
     *         display(result)
     *     }
     * }
     * ```
     *
     * The scope in this example has the following semantics:
     * 1) `showSomeData` returns as soon as the data is loaded and displayed in the UI.
     * 2) If `doSomeWork` throws an exception, then the `async` task is cancelled and `showSomeData` rethrows that exception.
     * 3) If the outer scope of `showSomeData` is cancelled, both started `async` and `withContext` blocks are cancelled.
     * 4) If the `async` block fails, `withContext` will be cancelled.
     *
     * The method may throw a [CancellationException] if the current job was cancelled externally,
     * rethrow the exception thrown by [block], or throw an unhandled [Throwable] if there is one
     * (for example, from a crashed coroutine that was started with [launch][CoroutineScope.launch] in this scope).
     */
    // Transliterated from: kotlinx-coroutines-core/common/src/CoroutineScope.kt:279-288
    template<typename T, typename Block>
    void* coroutine_scope(Block&& block, std::shared_ptr<Continuation<void*>> completion) {
        using Value = std::conditional_t<std::is_void_v<T>, Unit, T>;
        auto coroutine = std::make_shared<internal::ScopeCoroutine<Value>>(
            completion->get_context(), internal::result_box_completion<Value>(completion));
        return coroutine->start_undispatched_or_return(
            [coroutine, block = std::forward<Block>(block)](std::shared_ptr<Continuation<void*>> continuation) mutable -> void* {
                return block(static_cast<CoroutineScope*>(coroutine.get()), std::move(continuation));
            });
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
