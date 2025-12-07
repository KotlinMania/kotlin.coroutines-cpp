#pragma once
#include "kotlinx/coroutines/core_fwd.hpp"
#include "kotlinx/coroutines/CoroutineContext.hpp"
#include "kotlinx/coroutines/Result.hpp"

namespace kotlinx {
namespace coroutines {

class ContinuationBase {
public:
    virtual ~ContinuationBase() = default;
};

template <typename T>
class Continuation : public ContinuationBase {
public:
    virtual CoroutineContext get_context() const = 0;
    virtual void resume_with(Result<T> result) = 0;
};

} // namespace coroutines
} // namespace kotlinx
