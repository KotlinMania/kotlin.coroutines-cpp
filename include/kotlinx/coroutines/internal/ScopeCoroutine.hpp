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

    bool is_scoped_coroutine() const override { return true; }
    
    // Additional overrides if needed
};

// Also ContextScope
class ContextScope : public CoroutineScope {
    CoroutineContext context_;
public:
    explicit ContextScope(CoroutineContext context) : context_(context) {}
    CoroutineContext get_coroutine_context() const override { return context_; }
};

} // namespace internal
} // namespace coroutines
} // namespace kotlinx
