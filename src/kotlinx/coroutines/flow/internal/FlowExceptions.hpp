#pragma once
#include "kotlinx/coroutines/Exceptions.hpp"
#include <stdexcept>
#include <exception>
#include <string>

namespace kotlinx {
namespace coroutines {
namespace flow {
namespace internal {

class AbortFlowException : public CancellationException {
public:
    void* owner;
    
    explicit AbortFlowException(void* owner_) 
        : CancellationException("Flow was aborted, this exception should not be seen")
        , owner(owner_) {
        (void)owner_;
    }

    void check_ownership(void* other) {
        if (owner != other) {
            throw *this;
        }
    }
};

class ChildCancelledException : public CancellationException {
public:
    ChildCancelledException();
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
