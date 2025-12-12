#pragma once
#include <exception>
#include <iostream>
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
        auto delegate_ptr = get_delegate();
        if (!delegate_ptr) return;
        
        if (is_cancellable_mode(resume_mode)) {
            auto context = delegate_ptr->get_context();
            bool is_active = true; // TODO: Check Job.isActive from context
             if (!is_active) {
                 // cancel_completed_result(taken_state, cause);
                 return;
             }
        }
        
        Result<T> result = take_state();
        try {
            delegate_ptr->resume_with(result);
        } catch (...) {
            // handle_fatal_exception(std::current_exception(), nullptr);
        }
    }
};

// Resume the delegate continuation directly with the task's state
template<typename T>
void resume(DispatchedTask<T>* task, bool undispatched) {
    std::cerr << "[dispatch] resume called" << std::endl;
    auto delegate = task->get_delegate();
    if (!delegate) {
        std::cerr << "[dispatch] delegate is null!" << std::endl;
        return;
    }
    std::cerr << "[dispatch] got delegate, calling take_state" << std::endl;

    Result<T> state = task->take_state();
    std::cerr << "[dispatch] take_state returned, is_success=" << state.is_success() << std::endl;
    // In Kotlin this goes through getExceptionalResult/getSuccessfulResult
    // For now, pass the result directly
    delegate->resume_with(state);
    std::cerr << "[dispatch] resume_with called on delegate" << std::endl;
    (void)undispatched; // TODO: handle undispatched mode via DispatchedContinuation
}

template<typename T>
void dispatch(DispatchedTask<T>* task, int mode) {
    // Simplified dispatch - in Kotlin this checks if dispatcher.isDispatchNeeded
    // and either dispatches through the dispatcher or resumes directly.
    // For now, we just resume directly (equivalent to undispatched/unconfined behavior)
    bool undispatched = (mode == MODE_UNDISPATCHED);
    resume(task, undispatched);
}

} // namespace coroutines
} // namespace kotlinx
