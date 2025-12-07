#pragma once
#include "kotlinx/coroutines/CoroutineDispatcher.hpp"
#include "kotlinx/coroutines/Continuation.hpp"
#include "kotlinx/coroutines/Runnable.hpp"
#include <atomic>

namespace kotlinx {
namespace coroutines {

class DispatchedContinuationBase : public Runnable {
public:
    virtual void release() = 0;
};

template <typename T>
class DispatchedContinuation : public Continuation<T>, public DispatchedContinuationBase {
public:
    std::shared_ptr<CoroutineDispatcher> dispatcher;
    std::shared_ptr<Continuation<T>> continuation;
    
    DispatchedContinuation(std::shared_ptr<CoroutineDispatcher> dispatcher, std::shared_ptr<Continuation<T>> continuation)
        : dispatcher(dispatcher), continuation(continuation) {}

    CoroutineContext get_context() const override {
        return continuation->get_context();
    }

    void resume_with(Result<T> result) override {
        // Dispatch logic
        // context = continuation.context
        // if dispatcher.isDispatchNeeded(context)
        //    dispatcher.dispatch(context, this_as_runnable_with_result)
        // else
        //    resume_undispatched
        
        // Simplified for C++ port: always dispatch for now or basic check
        if (dispatcher->is_dispatch_needed(get_context())) {
             // We need to pass 'this' as Runnable, but 'this' is templated.
             // Usually implies DispatchedContinuation implements Runnable (via Base).
             // And we need to store the result to pass it to the continuation later.
             this->result_ = result;
             dispatcher->dispatch(get_context(), std::shared_ptr<Runnable>(this, [](Runnable*){})); // weak ref or shared_from_this?
        } else {
            continuation->resume_with(result);
        }
    }
    
    void run() override {
        continuation->resume_with(result_);
    }
    
    void release() override {
        // cleanup if needed
    }

private:
    Result<T> result_;
};

} // namespace coroutines
} // namespace kotlinx
