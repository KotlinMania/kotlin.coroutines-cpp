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
          context(parent_context) { // Simplified ctx init
        if (init_parent_job) {
             // init_parent_job_internal(std::dynamic_pointer_cast<Job>(parent_context.get(Job::key)));
             // Need smart retrieval of Job from context which is shared_ptr now?
             // CoroutineContext and Element hierarchy need fix for correct shared_ptr usage
        }
    }

    virtual ~AbstractCoroutine() = default;

    CoroutineContext parent_context;
    CoroutineContext context; // Should be shared_ptr<CoroutineContext>

    std::shared_ptr<CoroutineContext> get_coroutine_context() const override {
        // return std::make_shared<CoroutineContext>(context); // copy?
        // AbstractCoroutine logic needs heavy strict pointer refactor
        return std::make_shared<CoroutineContext>(); // Stub for now
    }
    
    // Continuation impl
    std::shared_ptr<CoroutineContext> get_context() const override {
        return std::make_shared<CoroutineContext>();
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
        // std::any state = make_completing_once(result.to_state());
        // after_resume(state);
        // Stub:
        if (result.is_success()) on_completed(result.get_or_throw());
        else on_cancelled(result.exception_or_null(), false);
    }

    virtual void after_resume(std::any state) {
        // after_completion(state);
    }
    
    virtual void after_completion(std::any state) {}
    
    // handle_coroutine_exception delegation?
    
    std::string name_string() {
        return "AbstractCoroutine";
    }

    template <typename R>
    void start(CoroutineStart start_strategy, R receiver, std::function<T(R)> block) {
        // start_strategy.invoke(block, receiver, this);
    }

protected:
    virtual void on_completion_internal(std::any state) {
        // ...
    }
};

} // namespace coroutines
} // namespace kotlinx
