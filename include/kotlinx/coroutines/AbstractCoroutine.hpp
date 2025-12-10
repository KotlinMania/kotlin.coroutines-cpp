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
    AbstractCoroutine(CoroutineContext parent_context, bool init_parent_job = true, bool active = true)
        : JobSupport(active),
          parent_context(parent_context),
          context(parent_context) { 
        if (init_parent_job) {
             init_parent_job_internal(parent_context.get(Job::Key));
        }
    }

    virtual ~AbstractCoroutine() = default;

    CoroutineContext parent_context;
    CoroutineContext context; 

    std::shared_ptr<CoroutineContext> get_coroutine_context() const override {
        // Return parent_context + this
        auto self = std::const_pointer_cast<AbstractCoroutine<T>>(std::static_pointer_cast<const AbstractCoroutine<T>>(shared_from_this()));
        return std::make_shared<CombinedContext>(parent_context, self);
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
    
    // Kotlin: resumeWith(result)
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
    void init_parent_job_internal(std::shared_ptr<Job> parent) {
        if (parent) {
            parent->start(); // ensure started
            auto handle = parent->attach_child(std::static_pointer_cast<Job>(shared_from_this()));
            // store handle
        }
    }
};

} // namespace coroutines
} // namespace kotlinx
