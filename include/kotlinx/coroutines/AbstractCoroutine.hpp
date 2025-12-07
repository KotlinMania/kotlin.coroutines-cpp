#pragma once
#include <string>
#include <memory>
#include <functional>
#include <any>
#include <typeinfo>
#include "kotlinx/coroutines/core_fwd.hpp"
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
             init_parent_job_internal(std::dynamic_pointer_cast<Job>(parent_context.get(Job::key)));
        }
    }

    virtual ~AbstractCoroutine() = default;

    CoroutineContext parent_context;
    CoroutineContext context;

    CoroutineContext get_coroutine_context() const override {
        return context;
    }

    bool is_active() const override {
        return JobSupport::is_active();
    }

    virtual void on_completed(T value) {}

    virtual void on_cancelled(std::exception_ptr cause, bool handled) {}

    std::string cancellation_exception_message() override {
        return "AbstractCoroutine was cancelled";
    }

    void resume_with(Result<T> result) override {
        std::any state = make_completing_once(result.to_state());
        // Simple check for COMPLETING_WAITING_CHILDREN if possible, or cast
        // if (state.type() == typeid(void*) && std::any_cast<void*>(state) == COMPLETING_WAITING_CHILDREN) return;
        after_resume(state);
    }

    virtual void after_resume(std::any state) {
        after_completion(state);
    }
    
    void handle_on_completion_exception(std::exception_ptr exception) override {
        handle_coroutine_exception(context, exception);
    }

    std::string name_string() override {
        return JobSupport::name_string();
    }

    template <typename R>
    void start(CoroutineStart start_strategy, R receiver, std::function<T(R)> block) {
        // start_strategy.invoke(block, receiver, this);
    }

protected:
    void on_completion_internal(std::any state) override {
        if (state.type() == typeid(CompletedExceptionally)) {
             auto ex = std::any_cast<CompletedExceptionally>(state);
             on_cancelled(ex.cause, ex.handled);
        } else {
            try {
                if (state.has_value()) {
                    on_completed(std::any_cast<T>(state));
                }
            } catch (const std::bad_any_cast&) {
                // Handle or ignore mismatch
            }
        }
    }
};

} // namespace coroutines
} // namespace kotlinx
