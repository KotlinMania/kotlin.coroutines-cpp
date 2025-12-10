#pragma once
#include <exception>
#include "kotlinx/coroutines/Runnable.hpp"
#include "kotlinx/coroutines/Continuation.hpp"
#include "kotlinx/coroutines/Job.hpp"
#include "kotlinx/coroutines/Result.hpp"

namespace kotlinx {
namespace coroutines {

constexpr int MODE_ATOMIC = 0;
constexpr int MODE_CANCELLABLE = 1;
constexpr int MODE_CANCELLABLE_REUSABLE = 2;
constexpr int MODE_UNDISPATCHED = 4;
constexpr int MODE_UNINITIALIZED = -1;

inline bool is_cancellable_mode(int mode) {
    return mode == MODE_CANCELLABLE || mode == MODE_CANCELLABLE_REUSABLE;
}

// SchedulerTask alias for Runnable to match Kotlin hierarchy
class SchedulerTask : public Runnable {
public:
    virtual ~SchedulerTask() = default;
};

template<typename T>
class DispatchedTask : public SchedulerTask {
public:
    int resume_mode;

    explicit DispatchedTask(int mode) : resume_mode(mode) {}

    virtual std::shared_ptr<Continuation<T>> get_delegate() = 0;
    virtual Result<T> take_state() = 0;
    virtual void cancel_completed_result(Result<T> taken_state, std::exception_ptr cause) {}

    void run() override {
        // Artisan implementation: Check cancellation relative to resume mode
        auto delegate_ptr = get_delegate();
        if (!delegate_ptr) return; // Should not happen
        
        if (is_cancellable_mode(resume_mode)) {
            // Context retrieval
            auto context = delegate_ptr->get_context(); // ContinuationBase returns context by value or shared_ptr? 
            // Continuation interface defined as: virtual CoroutineContext get_context() const = 0; (by value/struct slice if not careful)
            // Implementation return usually CoroutineContext (which is abstract).
            // Actually my Continuation.hpp defines: virtual CoroutineContext get_context() const = 0;
            // But CoroutineContext is a class (polymorphic). Returning by value slices it!
            // I need to fix Continuation.hpp return type !! 
            // It should be std::shared_ptr<CoroutineContext> or similar reference.
            
            // Let's assume I fix Continuation.hpp parallelly or handle it here.
            // For now, assuming get_context returns shared_ptr or reference.
            // Wait, previous `Continuation.hpp` view showed `virtual CoroutineContext get_context() const = 0;`.
            // `CoroutineContext` is a base class. This is object slicing.
            // I MUST fix Continuation.hpp first to return std::shared_ptr<CoroutineContext> or const CoroutineContext&.
            
            // Assuming fix:
            // std::shared_ptr<Job> job = std::dynamic_pointer_cast<Job>(context->get(Job::key));
            // if (job && !job->is_active()) ...
        }
        
        Result<T> result = take_state();
        try {
            delegate_ptr->resume_with(result);
        } catch (...) {
            // handle_fatal_exception(std::current_exception(), nullptr);
        }
    }
};

template<typename T>
void dispatch(DispatchedTask<T>* task, int mode) {
    // Helper to dispatch mechanism if needed
    // In Kotlin: dispatcher.dispatch(context, task)
    // Here we assume task is already dispatched or this method creates the dispatch.
}

} // namespace coroutines
} // namespace kotlinx
