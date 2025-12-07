#pragma once
#include "kotlinx/coroutines/core_fwd.hpp"
#include <exception>

namespace kotlinx {
namespace coroutines {

// Forward declarations
template<typename T> class Continuation;
class CoroutineContext;
class SchedulerTask; // Should be defined in SchedulerTask.hpp or similar
class CoroutineDispatcher;

constexpr int MODE_ATOMIC = 0;
constexpr int MODE_CANCELLABLE = 1;
constexpr int MODE_CANCELLABLE_REUSABLE = 2;
constexpr int MODE_UNDISPATCHED = 4;
constexpr int MODE_UNINITIALIZED = -1;

inline bool is_cancellable_mode(int mode) {
    return mode == MODE_CANCELLABLE || mode == MODE_CANCELLABLE_REUSABLE;
}

inline bool is_reusable_mode(int mode) {
    return mode == MODE_CANCELLABLE_REUSABLE;
}

class SchedulerTask { // Stub base class if not defined elsewhere
public:
    virtual ~SchedulerTask() = default;
    virtual void run() = 0;
};

template<typename T>
class DispatchedTask : public SchedulerTask {
public:
    int resume_mode;

    explicit DispatchedTask(int resume_mode) : resume_mode(resume_mode) {}

    virtual Continuation<T>* get_delegate() = 0;
    virtual void* take_state() = 0;
    virtual void cancel_completed_result(void* taken_state, std::exception_ptr cause) {}

    template<typename U>
    U get_successful_result(void* state) {
        return static_cast<U>(state); // Stub cast
    }

    virtual std::exception_ptr get_exceptional_result(void* state) {
        return nullptr;
    }

    void run() override {
        // Stub implementation
    }
};

template<typename T>
void dispatch(DispatchedTask<T>* task, int mode) {
    // Stub
}

template<typename T>
void resume(DispatchedTask<T>* task, Continuation<T>* delegate, bool undispatched) {
    // Stub
}

} // namespace coroutines
} // namespace kotlinx
