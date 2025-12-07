#pragma once
#include "kotlinx/coroutines/CancellableContinuation.hpp"
#include <atomic>
#include <mutex>

namespace kotlinx {
namespace coroutines {

// Stub base classes
template <typename T>
struct DispatchedTask : public Runnable {
    DispatchedTask(int mode) {}
    virtual void run() override {}
};

struct CoroutineStackFrame {};
struct Waiter {};

/**
 * Implementation of CancellableContinuation.
 *
 * == State Machine ==
 * 
 * UNDECIDED -> trySuspend -> SUSPENDED
 *     |
 *     v tryResume
 *  RESUMED
 *
 * Both tryResume and trySuspend can be invoked at most once; first invocation wins.
 * 
 * Internal states:
 * - ACTIVE: Active, no listeners.
 * - SINGLE_A: Active, one cancellation listener.
 * - CANCELLED: Cancelled (final state).
 * - COMPLETED: Produced result or exception (final state).
 */
template <typename T>
class CancellableContinuationImpl : public DispatchedTask<T>, public CancellableContinuation<T>, public CoroutineStackFrame, public Waiter {
public:
    CancellableContinuationImpl(std::shared_ptr<Continuation<T>> delegate, int resume_mode)
        : DispatchedTask<T>(resume_mode), delegate(delegate), resume_mode(resume_mode) {}

    // Continuation overrides
    CoroutineContext get_context() const override { return delegate->get_context(); }
    void resume_with(Result<T> result) override {
        // implementation stub
    }

    // CancellableContinuation overrides
    bool is_active() const override { return true; } // stub
    bool is_completed() const override { return false; } // stub
    bool is_cancelled() const override { return false; } // stub

    void invoke_on_cancellation(std::function<void(std::exception_ptr)> handler) override {
        // stub
    }
    
    bool try_resume(T value) override { return true; }
    bool try_resume_with_exception(std::exception_ptr exception) override { return true; }
    
    void complete_resume(std::any token) override {}
    
    void init_cancellability() override {}
    
    // Internal state
    std::shared_ptr<Continuation<T>> delegate;
    int resume_mode;
    
    std::string to_string() const { // Virtual?
        return "CancellableContinuationImpl";
    }

    /**
     * Resumes this continuation with the given [value] in the context of the specific [dispatcher].
     * Optimization: if the [dispatcher] matches the delegate's dispatcher, use MODE_UNDISPATCHED.
     */
    void resume_undispatched_impl(CoroutineDispatcher* dispatcher, T value) override {
        // TODO: dynamic_cast delegate to DispatchedContinuation to check dispatcher
        // auto* dc = dynamic_cast<DispatchedContinuation<T>*>(delegate.get());
        // int mode = (dc && dc->dispatcher == dispatcher) ? MODE_UNDISPATCHED : resume_mode;
        // resume_impl(value, mode);
    }

private:
   // State machine fields (atomic)
   std::atomic<int> decision;
};

} // namespace coroutines
} // namespace kotlinx
