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
        auto self = std::const_pointer_cast<AbstractCoroutine<T>>(std::static_pointer_cast<const AbstractCoroutine<T>>(shared_from_this()));
        
        // Cast self to Element
        auto self_element = std::static_pointer_cast<CoroutineContext::Element>(self);

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
    
    void resume_with(Result<T> result) override {
        void* state;
        if (result.is_success()) {
            if constexpr (std::is_void_v<T>) {
                state = nullptr; // Unit
            } else {
                state = new T(result.get_or_throw()); 
            }
        } else {
            state = new CompletedExceptionally(result.exception_or_null());
        }
        
        void* final_state = make_completing_once(state);
        
        if (final_state == (void*)COMPLETING_WAITING_CHILDREN) return;
        
        after_resume(final_state);
    }
    
    void* make_completing_once(void* proposed_update) {
        if (make_completing(proposed_update)) {
            return nullptr;
        }
        return (void*)COMPLETING_ALREADY;
    }

    virtual void after_resume(std::any state) {
        // after_completion(state); as per kotlin
    }
    
    virtual void after_completion(void* state) override {
        if (auto* ex = dynamic_cast<CompletedExceptionally*>(static_cast<CompletedExceptionally*>(state))) { 
             // if (ex) on_cancelled...
        }
    }
    
    std::string name_string() {
        return "AbstractCoroutine";
    }

    template <typename R>
    void start(CoroutineStart start_strategy, R receiver, std::function<T(R)> block) {
        invoke(start_strategy, block, receiver, this->shared_from_this());
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
                auto handle = parent_job->attach_child(std::static_pointer_cast<ChildJob>(shared_from_this()));
             }
        }
    }
};

} // namespace coroutines
} // namespace kotlinx
