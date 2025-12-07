#pragma once
#include "kotlinx/coroutines/core_fwd.hpp"
#include "kotlinx/coroutines/context_impl.hpp"
#include "kotlinx/coroutines/Job.hpp"
#include <functional>
#include <any>

namespace kotlinx {
namespace coroutines {

template <typename T>
struct CancellableContinuation : public Continuation<T> {
    virtual bool is_active() const = 0;
    virtual bool is_completed() const = 0;
    virtual bool is_cancelled() const = 0;

    virtual void invoke_on_cancellation(std::function<void(std::exception_ptr)> handler) = 0;
    
    virtual bool try_resume(T value) = 0;
    virtual bool try_resume_with_exception(std::exception_ptr exception) = 0;
    
    // Dispatcher interaction helper
    class DispatcherResumeAdapter {
        CancellableContinuation& cont;
        CoroutineDispatcher* dispatcher;
    public:
        DispatcherResumeAdapter(CancellableContinuation& c, CoroutineDispatcher* d) : cont(c), dispatcher(d) {}
        void resume_undispatched(T value) {
            cont.resume_undispatched_impl(dispatcher, value);
        }
    };

    DispatcherResumeAdapter dispatcher_adapter(CoroutineDispatcher* d) {
        return DispatcherResumeAdapter(*this, d);
    }

protected:
    // Implementation hook for resume_undispatched
    virtual void resume_undispatched_impl(CoroutineDispatcher* dispatcher, T value) = 0;

public:
    // Kotlin specific helpers
    virtual void init_cancellability() = 0;
};

} // namespace coroutines
} // namespace kotlinx
