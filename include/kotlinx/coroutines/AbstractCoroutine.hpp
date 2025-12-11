#pragma once
#include <string>
#include <memory>
#include <functional>
#include <any>
#include <typeinfo>
#include "kotlinx/coroutines/JobSupport.hpp"
#include "kotlinx/coroutines/CoroutineScope.hpp"
#include "kotlinx/coroutines/CompletedExceptionally.hpp"
#include "kotlinx/coroutines/CoroutineContext.hpp"
#include "kotlinx/coroutines/context_impl.hpp"
#include "kotlinx/coroutines/Result.hpp"
#include "kotlinx/coroutines/CoroutineStart.hpp"
#include "kotlinx/coroutines/CoroutineExceptionHandler.hpp"

namespace kotlinx {
namespace coroutines {

/**
 * Abstract base class for implementation of coroutines in coroutine builders.
 */
template <typename T>
class AbstractCoroutine : public JobSupport, public Continuation<T>, public CoroutineScope {
public:
    using JobSupport::start;
    AbstractCoroutine(std::shared_ptr<CoroutineContext> parent_context, bool init_parent_job = true, bool active = true)
        : JobSupport(active),
          parent_context(parent_context),
          context(parent_context) { // Initial assignment, will be overwritten or used correctly
        
        // context = parent_context + this
        // "this" is Job, Job implements Element.
        // We cannot use shared_from_this() in constructor.
        // So we delay context creation? no, Kotlin does it in constructor "val context = parentContext + this"
        // In C++, we can't get shared_from_this in constructor.
        // Logic: if we need fully constructed context in constructor, we have a problem.
        // Usually context is used later.
        // We can just store parent_context and construct CombinedContext on demand or in start()?
        
        if (init_parent_job) {
             init_parent_job_internal(parent_context->get(Job::type_key));
        }
    }

    virtual ~AbstractCoroutine() = default;

    std::shared_ptr<CoroutineContext> parent_context;
    std::shared_ptr<CoroutineContext> context; 

    std::shared_ptr<CoroutineContext> get_coroutine_context() const override {
        // Return fully constructed context (parent + this)
        // This is safe to call after construction
        // Note: shared_from_this() returns shared_ptr<JobSupport>, we need to cast
        auto self_job = const_cast<AbstractCoroutine<T>*>(this)->JobSupport::shared_from_this();

        // Cast self to Element
        auto self_element = std::static_pointer_cast<CoroutineContext::Element>(self_job);

        return std::make_shared<CombinedContext>(parent_context, self_element);
    }
    
    // Continuation impl
    std::shared_ptr<CoroutineContext> get_context() const override {
        return get_coroutine_context();
    }

    bool is_active() const override {
        return JobSupport::is_active();
    }

    virtual void on_completed(T value) {}

    virtual void on_cancelled(std::exception_ptr cause, bool handled) {}

    virtual std::string cancellation_exception_message() {
        return "AbstractCoroutine was cancelled";
    }
    
    // NOTE: T should be Unit for coroutines that don't return a value, NOT void.
    // Using void as T is not supported and will cause compilation errors.
    void resume_with(Result<T> result) override {
        JobState* state;
        if (result.is_success()) {
            state = reinterpret_cast<JobState*>(new T(result.get_or_throw()));
        } else {
            state = new CompletedExceptionally(result.exception_or_null());
        }

        auto completing_result = JobSupport::make_completing_once(state);

        if (completing_result == CompletingResult::COMPLETING) return;

        after_resume(state);
    }

    virtual void after_resume(JobState* state) {
        // In Kotlin: protected open fun afterResume(state: Any?): Unit = afterCompletion(state)
        // afterCompletion is from JobSupport and is a no-op by default
        // We just call on_completion_internal directly
        on_completion_internal(state);
    }

    // NOTE: T should be Unit for coroutines that don't return a value, NOT void.
    void on_completion_internal(JobState* state) override {
        if (state == nullptr) {
            // Should not happen - state should always be valid T* or CompletedExceptionally*
            return;
        }

        // Try to cast to CompletedExceptionally first
        auto* ex = dynamic_cast<CompletedExceptionally*>(state);
        if (ex && ex->cause) {
            on_cancelled(ex->cause, ex->handled);
        } else {
            // It's a successful completion with value
            // State was stored via reinterpret_cast<JobState*>(new T(...))
            on_completed(*reinterpret_cast<T*>(state));
        }
    }
    
    void handle_on_completion_exception(std::exception_ptr exception) override {
        handle_coroutine_exception(*get_context(), exception);
    }
    
    // ===== API COMPLETENESS AUDIT =====
    // Kotlin API from kotlinx-coroutines-core.api line 1-13:
    // ✓ <init>(CoroutineContext, Boolean, Boolean) → AbstractCoroutine(parent_context, init_parent_job, active)
    // ✓ protected fun afterResume(state: Any?) → after_resume(JobState* state)
    // ✓ protected fun cancellationExceptionMessage() → cancellation_exception_message()
    // ✓ public final fun getContext() → get_context()
    // ✓ public fun getCoroutineContext() → get_coroutine_context()
    // ✓ public fun isActive() → is_active()
    // ✓ protected fun onCancelled(cause: Throwable, handled: Boolean) → on_cancelled(cause, handled)
    // ✓ protected fun onCompleted(value: T) → on_completed(value)
    // ✓ protected final fun onCompletionInternal(state: Any?) → on_completion_internal(JobState* state)
    // ✓ public final fun resumeWith(result: Result<T>) → resume_with(result)
    // ✓ public final fun start(start: CoroutineStart, receiver: R, block: suspend R.() -> T)
    //       → template<typename R> void start(CoroutineStart, R, std::function<T(R)>)

    std::string name_string() {
        // TODO: Implement coroutine name extraction from context
        // Kotlin: val coroutineName = context.coroutineName ?: return super.nameString()
        //         return "\"$coroutineName\":${super.nameString()}"
        return "AbstractCoroutine";
    }

    template <typename R>
    void start(CoroutineStart start_strategy, R receiver, std::function<T(R)> block) {
        invoke(start_strategy, block, receiver, JobSupport::shared_from_this());
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
                parent_job->start();
                auto handle = parent_job->attach_child(std::dynamic_pointer_cast<ChildJob>(JobSupport::shared_from_this()));
             }
        }
    }
};

} // namespace coroutines
} // namespace kotlinx
