#pragma once
#include "core_fwd.hpp"
#include "CoroutineContext.hpp"
#include "Result.hpp"

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
