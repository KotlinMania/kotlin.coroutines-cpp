#pragma once
#include "kotlinx/coroutines/CoroutineDispatcher.hpp"
#include "kotlinx/coroutines/Continuation.hpp"
#include "kotlinx/coroutines/Runnable.hpp"
#include "kotlinx/coroutines/internal/DispatchedTask.hpp"
#include <atomic>
#include <memory>

namespace kotlinx {
namespace coroutines {

class DispatchedContinuationBase : public Runnable {
public:
    virtual void release() = 0;
};

template <typename T>
class DispatchedContinuation : public Continuation<T>, 
                               public DispatchedTask<T>,
                               public std::enable_shared_from_this<DispatchedContinuation<T>> {
public:
    std::shared_ptr<CoroutineDispatcher> dispatcher;
    std::shared_ptr<Continuation<T>> continuation;
    
    DispatchedContinuation(std::shared_ptr<CoroutineDispatcher> dispatcher, std::shared_ptr<Continuation<T>> continuation)
        : DispatchedTask<T>(MODE_CANCELLABLE), dispatcher(dispatcher), continuation(continuation) {}

    std::shared_ptr<CoroutineContext> get_context() const override {
        return continuation->get_context();
    }

    std::shared_ptr<Continuation<T>> get_delegate() override {
        return continuation;
    }
    
    Result<T> take_state() override {
        return result_;
    }

    void resume_with(Result<T> result) override {
        auto context = get_context();
        if (dispatcher->is_dispatch_needed(*context)) {
             this->result_ = result;
             // Dispatch self
             dispatcher->dispatch(*context, this->shared_from_this());
        } else {
             continuation->resume_with(result);
        }
    }
    
    void run() override {
        DispatchedTask<T>::run();
    }

private:
    Result<T> result_;
};


// Implement CoroutineDispatcher::intercept_continuation here
template <typename T>
std::shared_ptr<Continuation<T>> CoroutineDispatcher::intercept_continuation(std::shared_ptr<Continuation<T>> continuation) {
    return std::make_shared<DispatchedContinuation<T>>(std::dynamic_pointer_cast<CoroutineDispatcher>(shared_from_this()), continuation);
}

} // namespace coroutines
} // namespace kotlinx
