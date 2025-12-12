#pragma once
#include <stdexcept>
#include <exception>
#include <string>

namespace kotlinx {
namespace coroutines {
namespace flow {
namespace internal {

class AbortFlowException : public std::runtime_error {
public:
    void* owner;
    
    explicit AbortFlowException(void* owner_) 
        : std::runtime_error("Flow was aborted, this exception should not be seen")
        , owner(owner_) {}

    void check_ownership(void* other) {
        if (owner != other) {
            throw *this;
        }
    }
};

class ChildCancelledException : public std::runtime_error {
public:
    ChildCancelledException() : std::runtime_error("Child flow cancelled") {}
};

inline int check_index_overflow(int index) {
    if (index < 0) {
        throw std::overflow_error("Index overflow has happened");
    }
    return index;
}

} // namespace internal
} // namespace flow
} // namespace coroutines
} // namespace kotlinx
