#pragma once
#include "kotlinx/coroutines/Job.hpp"
#include <exception>

namespace kotlinx {
namespace coroutines {

class CompletableJob : public virtual Job {
public:
    virtual bool complete() = 0;
    virtual bool complete_exceptionally(std::exception_ptr exception) = 0;
    virtual ~CompletableJob() = default;
};

} // namespace coroutines
} // namespace kotlinx
