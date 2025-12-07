#pragma once
#include <exception>

namespace kotlinx {
namespace coroutines {

template <typename T>
struct Result {
    T value;
    std::exception_ptr exception;

    Result(T v) : value(v), exception(nullptr) {}
    Result(std::exception_ptr e) : exception(e) {}

    bool is_success() const { return !exception; }
    bool is_failure() const { return exception != nullptr; }

    void* to_state() const {
        if (exception) {
            // encapsulate exception?
            return exception; // Simplified
        }
        return value;
    }
    
    T get_or_throw() const {
        if (exception) std::rethrow_exception(exception);
        return value;
    }
};

} // namespace coroutines
} // namespace kotlinx
