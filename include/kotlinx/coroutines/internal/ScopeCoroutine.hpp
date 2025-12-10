#pragma once
#include "kotlinx/coroutines/AbstractCoroutine.hpp"
#include "kotlinx/coroutines/CoroutineScope.hpp"
#include <memory>

namespace kotlinx {
namespace coroutines {
namespace internal {

template <typename T>
class ScopeCoroutine : public AbstractCoroutine<T> {
public:
    std::shared_ptr<Continuation<T>> u_cont;

    ScopeCoroutine(CoroutineContext context, std::shared_ptr<Continuation<T>> uCont)
        : AbstractCoroutine<T>(context, true, true), u_cont(uCont) {}

    // bool is_scoped_coroutine() const override { return true; } // Not standard in AbstractCoroutine API
    
    // Resume delegate
    void after_completion(std::any state) override {
        // u_cont->resume_with(state);
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
