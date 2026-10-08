#pragma once
// port-lint: source kotlinx-coroutines-core/common/src/AbstractCoroutine.kt
/** Transliterated from: kotlinx-coroutines-core/common/src/AbstractCoroutine.kt */
#include <string>
#include <memory>
#include <mutex>
#include <functional>
#include <any>
#include <typeinfo>
#include <type_traits>
#include "kotlinx/coroutines/JobSupport.hpp"
#include "kotlinx/coroutines/CoroutineScope.hpp"
#include "kotlinx/coroutines/CompletedExceptionally.hpp"
#include "kotlinx/coroutines/CompletedValue.hpp"
#include "kotlinx/coroutines/CompletionState.hpp"
#include "kotlinx/coroutines/CoroutineContext.hpp"
#include "kotlinx/coroutines/Unit.hpp"
#include "kotlinx/coroutines/context_impl.hpp"
#include "kotlinx/coroutines/Result.hpp"
#include "kotlinx/coroutines/CoroutineStart.hpp"
#include "kotlinx/coroutines/CoroutineExceptionHandler.hpp"
#include "kotlinx/coroutines/intrinsics/Intrinsics.hpp"

namespace kotlinx::coroutines {

/**
 * Abstract base class for implementation of coroutines in coroutine builders.
 *
 * This class implements completion [Continuation], [Job], and [CoroutineScope] interfaces.
 * It stores the result of continuation in the state of the job.
 * This coroutine waits for children coroutines to finish before completing and
 * fails through an intermediate _failing_ state.
 *
 * The following methods are available for override:
 *
 * - [onStart] is invoked when the coroutine was created in non-active state and is being [started][Job.start].
 * - [onCancelling] is invoked as soon as the coroutine starts being cancelled for any reason (or completes).
 * - [onCompleted] is invoked when the coroutine completes with a value.
 * - [onCancelled] in invoked when the coroutine completes with an exception (cancelled).
 *
 * @param parentContext the context of the parent coroutine.
 * @param initParentJob specifies whether the parent-child relationship should be instantiated directly
 *               in `AbstractCoroutine` constructor. If set to `false`, it's the responsibility of the child class
 *               to invoke [initParentJob] manually.
 * @param active when `true` (by default), the coroutine is created in the _active_ state, otherwise it is created in the _new_ state.
 *               See [Job] for details.
 *
 * @suppress **This an internal API and should not be used from general code.**
 */
    // Transliterated from: kotlinx-coroutines-core/common/src/AbstractCoroutine.kt:34-39
    template <typename T>
    class AbstractCoroutine : public JobSupport, public Continuation<T>, public virtual CoroutineScope {
    public:
        using JobSupport::start;
        /**
     * @brief Constructs an AbstractCoroutine with the given context and configuration.
     *
     * @param parent_context The parent coroutine context
     * @param init_parent_job Whether to initialize parent-child relationship
     * @param active Whether to start in active state (true) or new state (false)
     *
     * @note The full context (parent_context + this_job) is constructed lazily
     *       because shared_from_this() cannot be used in constructors.
     */
        AbstractCoroutine(std::shared_ptr<CoroutineContext> parent_context, bool init_parent_job = true, bool active = true)
            : JobSupport(active),
              parent_context(parent_context),
              context(parent_context),
              init_parent_job_flag(init_parent_job) {
        }

        virtual ~AbstractCoroutine() = default;

        std::shared_ptr<CoroutineContext> parent_context;
        std::shared_ptr<CoroutineContext> context;
        bool init_parent_job_flag{true};
        bool init_parent_job_done{false};

        using JobSupport::init_parent_job;

        void init_parent_job_if_needed() {
            if (init_parent_job_flag && !init_parent_job_done) {
                init_parent_job_done = true;
                if (parent_context) {
                    init_parent_job_internal(parent_context->get(Job::type_key));
                }
            }
        }

        /**
     * The context of this scope which is the same as the [context] of this coroutine.
     */
        // Transliterated from: kotlinx-coroutines-core/common/src/AbstractCoroutine.kt:63-63
        std::shared_ptr<CoroutineContext> get_coroutine_context() const override {
            const_cast<AbstractCoroutine<T>*>(this)->init_parent_job_if_needed();
            std::lock_guard<std::mutex> lock(context_mutex_);
            if (auto cached = context_cache_.lock()) return cached;
            // Return fully constructed context (parent + this)
            // This is safe to call after construction
            // Note: shared_from_this() returns shared_ptr<JobSupport>, we need to cast
            auto self_job = const_cast<AbstractCoroutine<T>*>(this)->JobSupport::shared_from_this();

            // Cast self to Element
            auto self_element = std::static_pointer_cast<CoroutineContext::Element>(self_job);

            auto combined = parent_context->operator+(std::move(self_element));
            context_cache_ = combined;
            return combined;
        }

        /**
     * The context of this coroutine that includes this coroutine as a [Job].
     */
        // Transliterated from: kotlinx-coroutines-core/common/src/AbstractCoroutine.kt:56-56
        std::shared_ptr<CoroutineContext> get_context() const override {
            return get_coroutine_context();
        }

        bool is_active() const override {
            return JobSupport::is_active();
        }

        /**
     * This function is invoked once when the job was completed normally with the specified [value],
     * right before all the waiters for the coroutine's completion are notified.
     */
        // Transliterated from: kotlinx-coroutines-core/common/src/AbstractCoroutine.kt:70-70
        // NOTE(port): The source hook is empty; retain its unused parameter name as a comment.
        virtual void on_completed(T /* value */) {}

        /**
     * This function is invoked once when the job was cancelled with the specified [cause],
     * right before all the waiters for coroutine's completion are notified.
     *
     * **Note:** the state of the coroutine might not be final yet in this function and should not be queried.
     * You can use [completionCause] and [completionCauseHandled] to recover parameters that we passed
     * to this `onCancelled` invocation only when [isCompleted] returns `true`.
     *
     * @param cause The cancellation (failure) cause
     * @param handled `true` if the exception was handled by parent (always `true` when it is a [CancellationException])
     */
        // Transliterated from: kotlinx-coroutines-core/common/src/AbstractCoroutine.kt:83-83
        // NOTE(port): The source hook is empty.
        virtual void on_cancelled(std::exception_ptr /* cause */, bool /* handled */) {}

        /**
     * @brief Returns the message for cancellation exceptions.
     *
     * Override to provide custom cancellation messages.
     * Default returns "AbstractCoroutine was cancelled".
     *
     * @return The cancellation message string
     *
     * Transliterated from: protected open fun cancellationExceptionMessage(): String
     */
        std::string cancellation_exception_message() const override {
            return "AbstractCoroutine was cancelled";
        }

        /**
     * Completes execution of this with coroutine with the specified result.
     */
        // Transliterated from: kotlinx-coroutines-core/common/src/AbstractCoroutine.kt:98-102
        void resume_with(Result<T> result) override final {
            // NOTE(port): Retain the executing receiver while finalization removes its parent handle.
            auto owner = JobSupport::shared_from_this();
            auto completing_result = JobSupport::make_completing_once(to_state<T>(std::move(result)));

            if (completing_result == CompletingResult::COMPLETING) return;
            after_resume(get_state_for_await());
        }

        /**
     * Invoked when the corresponding `AbstractCoroutine` was **conceptually** resumed, but not mechanically.
     * Currently, this function only invokes `resume` on the underlying continuation for [ScopeCoroutine]
     * or does nothing otherwise.
     *
     * Examples of resumes:
     * - `afterCompletion` calls when the corresponding `Job` changed its state (i.e. got cancelled)
     * - [AbstractCoroutine.resumeWith] was invoked
     */
        // Transliterated from: kotlinx-coroutines-core/common/src/AbstractCoroutine.kt:113
        virtual void after_resume(JobState* state) {
            this->after_completion(state);
        }

        // NOTE: T should be Unit for coroutines that don't return a value, NOT void.
        // Transliterated from: kotlinx-coroutines-core/common/src/AbstractCoroutine.kt:88-93
        void on_completion_internal(JobState* state) override final {
            if (auto* failed = dynamic_cast<CompletedExceptionally*>(state)) {
                on_cancelled(failed->cause, failed->handled.load());
            } else {
                on_completed(dynamic_cast<CompletedValue<T>&>(*state).value);
            }
        }

        // Transliterated from: kotlinx-coroutines-core/common/src/AbstractCoroutine.kt:115-117
        void handle_on_completion_exception(std::exception_ptr exception) override final {
            handle_coroutine_exception(*get_context(), exception);
        }


        //       → template<typename R> void start(CoroutineStart, R, std::function<T(R)>)

        /**
     * @brief Returns a string representation of this coroutine for debugging.
     *
     * If the context contains a CoroutineName, returns: "\"name\":<base_name>"
     * Otherwise delegates to JobSupport::name_string()
     *
     * Transliterated from: internal override fun nameString(): String
     */
        std::string name_string() const override {
            auto name = coroutine_name(get_coroutine_context());
            if (!name) {
                return JobSupport::name_string();
            }
            return "\"" + *name + "\":" + JobSupport::name_string();
        }

        /**
     * Starts this coroutine with the given code [block] and [start] strategy.
     * This function shall be invoked at most once on this coroutine.
     * 
     * - [DEFAULT] uses [startCoroutineCancellable].
     * - [ATOMIC] uses [startCoroutine].
     * - [UNDISPATCHED] uses [startCoroutineUndispatched].
     * - [LAZY] does nothing.
     */
        // Transliterated from: kotlinx-coroutines-core/common/src/AbstractCoroutine.kt:133-135
        // NOTE(port): Retain the typed completion at the explicit Continuation ABI boundary.
        template <typename R>
        void start(CoroutineStart start_strategy, R receiver,
                   std::function<void*(R, Continuation<T>*)> block) {
            init_parent_job_if_needed();
            invoke(start_strategy, std::move(block), std::forward<R>(receiver),
                   std::dynamic_pointer_cast<Continuation<T>>(JobSupport::shared_from_this()));
        }

        // Transliterated from: kotlinx-coroutines-core/common/src/AbstractCoroutine.kt:133-135
        template <typename R>
        void start(CoroutineStart start_strategy, R receiver, std::function<T(R)> block) {
            init_parent_job_if_needed();
            invoke(start_strategy, std::move(block), std::forward<R>(receiver), std::dynamic_pointer_cast<Continuation<T>>(JobSupport::shared_from_this()));
        }

        // Transliterated from: kotlinx-coroutines-core/common/src/AbstractCoroutine.kt:133-135
        template <typename R>
        void start(CoroutineStart start_strategy, R receiver,
                   std::function<void*(R, std::shared_ptr<Continuation<void*>>)> block) {
            init_parent_job_if_needed();
            invoke(start_strategy, std::move(block), std::forward<R>(receiver),
                   std::dynamic_pointer_cast<Continuation<T>>(JobSupport::shared_from_this()));
        }

        // Helper for parent init
        void init_parent_job_internal(std::shared_ptr<CoroutineContext::Element> parent_element) {
            // We need to cast Element to Job
            if (parent_element) {
                // In C++ we can't easily dynamic_cast from Element to Job if they are related via multiple inheritance nicely
                // But Job inherits Element.
                // We need to check if Element Key is Job::Key?
                // Actually parentContext.get(Job::Key) returns Element which IS a Job.
                auto parent_job = std::dynamic_pointer_cast<Job>(parent_element);
                if (parent_job) {
                    init_parent_job(parent_job);
                }
            }
        }

    private:
        // NOTE(port): Kotlin's context property is constructed once. A weak
        // cache preserves observable context identity without a C++ self cycle.
        mutable std::mutex context_mutex_;
        mutable std::weak_ptr<CoroutineContext> context_cache_;
    };

}
