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
 * @brief Abstract base class for coroutine implementations in coroutine builders.
 *
 * AbstractCoroutine bridges the Job system with coroutine execution. It combines
 * JobSupport (for lifecycle management) with Continuation<T> (for result handling)
 * and CoroutineScope (for context access).
 *
 * === Core Responsibilities ===
 * - Manages coroutine lifecycle through Job inheritance
 * - Handles coroutine result completion (success or failure)
 * - Provides coroutine context with proper Job integration
 * - Supports coroutine builders like launch() and async()
 * - Manages parent-child relationships in coroutine hierarchies
 *
 * === Template Parameter ===
 * @tparam T The result type of the coroutine. Use Unit for coroutines that
 *           don't return a value. void is not supported.
 *
 * === Context Integration ===
 * The coroutine context is constructed as parent_context + this_job, ensuring
 * that the coroutine itself is part of its own context. This enables proper
 * structured concurrency and cancellation semantics.
 *
 * === Lifecycle Management ===
 * - Inherits full job state machine from JobSupport
 * - Transitions to completing state when resume_with() is called
 * - Handles both successful completion and exceptional completion
 * - Invokes appropriate hooks (on_completed, on_cancelled)
 *
 * === Exception Handling ===
 * - Distinguishes between normal cancellation and other exceptions
 * - Propagates child failures to parent according to structured concurrency rules
 * - Supports CoroutineExceptionHandler for custom exception processing
 *
 * === Memory Management ===
 * - Uses shared_ptr for reference counting
 * - Properly handles circular references in parent-child relationships
 * - Automatic cleanup on completion
 *
 * @note This class is the foundation for all coroutine builder implementations.
 *       Specific coroutine types (launch, async, etc.) extend this class
 *       to provide specialized behavior.
 */
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

        // Transliterated from: kotlinx-coroutines-core/common/src/AbstractCoroutine.kt:56-63
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

        // Continuation impl
        std::shared_ptr<CoroutineContext> get_context() const override {
            return get_coroutine_context();
        }

        bool is_active() const override {
            return JobSupport::is_active();
        }

        /**
     * @brief Called when the coroutine completes successfully with a value.
     *
     * Override this method to handle successful completion.
     * Default implementation does nothing.
     *
     * @param value The result value of the coroutine
     */
        virtual void on_completed(T value) {}

        /**
     * @brief Called when the coroutine is cancelled or fails with an exception.
     *
     * Override this method to handle cancellation or failure.
     * Default implementation does nothing.
     *
     * @param cause The exception that caused cancellation
     * @param handled Whether the exception was handled by exception handlers
     */
        virtual void on_cancelled(std::exception_ptr cause, bool handled) {}

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
     * @brief Resumes the coroutine with the given result.
     *
     * This is the core method that completes a coroutine. It transitions
     * the job through the state machine and handles both successful and
     * exceptional completion.
     *
     * @param result The result containing either a value or an exception
     *
     * @note T should be Unit for coroutines that don't return a value, NOT void.
     *       Using void as T is not supported and will cause compilation errors.
     */
        // Transliterated from: kotlinx-coroutines-core/common/src/AbstractCoroutine.kt:98-102
        void resume_with(Result<T> result) override {
            // NOTE(port): Retain the executing receiver while finalization removes its parent handle.
            auto owner = JobSupport::shared_from_this();
            auto completing_result = JobSupport::make_completing_once(to_state<T>(std::move(result)));

            if (completing_result == CompletingResult::COMPLETING) return;
            after_resume(get_state_for_await());
        }

        // Transliterated from: kotlinx-coroutines-core/common/src/AbstractCoroutine.kt:113
        virtual void after_resume(JobState* state) {
            this->after_completion(state);
        }

        // NOTE: T should be Unit for coroutines that don't return a value, NOT void.
        void on_completion_internal(JobState* state) override {
            if (auto* failed = dynamic_cast<CompletedExceptionally*>(state)) {
                on_cancelled(failed->cause, failed->handled.load());
            } else {
                on_completed(dynamic_cast<CompletedValue<T>&>(*state).value);
            }
        }

        void handle_on_completion_exception(std::exception_ptr exception) override {
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
