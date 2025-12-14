#pragma once
#include "kotlinx/coroutines/AbstractCoroutine.hpp"
#include "kotlinx/coroutines/CoroutineScope.hpp"
#include <memory>

namespace kotlinx {
namespace coroutines {
namespace internal {

/**
 * This is a coroutine instance that is created by [coroutineScope] builder.
 */
template <typename T>
class ScopeCoroutine : public AbstractCoroutine<T> {
public:
    std::shared_ptr<Continuation<T>> u_cont; // unintercepted continuation

    ScopeCoroutine(std::shared_ptr<CoroutineContext> context, std::shared_ptr<Continuation<T>> uCont)
        : AbstractCoroutine<T>(context, true, true), u_cont(uCont) {}

    bool get_is_scoped_coroutine() const override { return true; }

    void after_completion(JobState* state) override {
        // Resume in a cancellable way by default when resuming from another context
        // TODO(port): implement interception and result recovery
        // u_cont->intercepted()->resume_cancellable_with(recover_result(state, u_cont));
        if (u_cont) {
            u_cont->resume_with(Result<T>::success(T{})); // TODO(semantics): proper result recovery
        }
    }

    /**
     * Invoked when a scoped coroutine was completed in an undispatched manner directly
     * at the place of its start because it never suspended.
     */
    virtual void after_completion_undispatched() {
    }

    void after_resume(JobState* state) override {
        // Resume direct because scope is already in the correct context
        if (u_cont) {
            u_cont->resume_with(Result<T>::success(T{})); // TODO(semantics): proper result recovery
        }
    }
};

class ContextScope : public CoroutineScope {
    std::shared_ptr<CoroutineContext> context_;
public:
    explicit ContextScope(std::shared_ptr<CoroutineContext> context) : context_(context) {}
    std::shared_ptr<CoroutineContext> get_coroutine_context() const override { return context_; }
};

} // namespace internal
} // namespace coroutines
} // namespace kotlinx
