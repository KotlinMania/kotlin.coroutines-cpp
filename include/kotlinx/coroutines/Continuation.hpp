#pragma once
#include "kotlinx/coroutines/CoroutineContext.hpp"
#include "kotlinx/coroutines/Result.hpp"
#include <memory>

namespace kotlinx {
namespace coroutines {

class ContinuationBase {
public:
    virtual ~ContinuationBase() = default;
};

template <typename T>
class Continuation : public ContinuationBase {
public:
    // Return shared_ptr to avoid slicing abstract base class
    virtual std::shared_ptr<CoroutineContext> get_context() const = 0;
    virtual void resume_with(Result<T> result) = 0;
};

} // namespace coroutines
} // namespace kotlinx
