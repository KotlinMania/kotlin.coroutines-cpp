#pragma once
#include <exception>
#include <atomic>

namespace kotlinx {
namespace coroutines {

struct JobState {
    virtual ~JobState() = default;
};

class CompletedExceptionally : public JobState {
public:
    std::exception_ptr cause;
    std::atomic<bool> handled;

    CompletedExceptionally(std::exception_ptr cause, bool handled = false) 
        : cause(cause), handled(handled) {}
        
    CompletedExceptionally(const CompletedExceptionally& other) 
        : cause(other.cause), handled(other.handled.load()) {}

    bool make_handled() {
        bool expected = false;
        return handled.compare_exchange_strong(expected, true);
    }
};

} // namespace coroutines
} // namespace kotlinx
