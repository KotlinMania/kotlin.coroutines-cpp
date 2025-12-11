#pragma once
#include "kotlinx/coroutines/internal/ScopeCoroutine.hpp"
#include "kotlinx/coroutines/flow/Flow.hpp"

namespace kotlinx {
namespace coroutines {
namespace flow {
namespace internal {

template <typename T>
class FlowCoroutine : public kotlinx::coroutines::internal::ScopeCoroutine<T> {
public:
    FlowCoroutine(std::shared_ptr<CoroutineContext> context, std::shared_ptr<Continuation<T>> uCont)
        : kotlinx::coroutines::internal::ScopeCoroutine<T>(context, uCont) {}

    bool child_cancelled(std::exception_ptr cause) override {
        // if (cause is ChildCancelledException) return true; // TODO checking exception type
        return this->cancel_impl(cause);
    }
};

} // namespace internal
} // namespace flow
} // namespace coroutines
} // namespace kotlinx
