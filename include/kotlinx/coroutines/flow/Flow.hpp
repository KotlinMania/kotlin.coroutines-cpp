#pragma once
#include "kotlinx/coroutines/flow/FlowCollector.hpp"

namespace kotlinx {
namespace coroutines {
namespace flow {

template <typename T>
struct Flow {
    virtual ~Flow() = default;
    virtual void collect(std::shared_ptr<FlowCollector<T>> collector) = 0; // suspend
};

} // namespace flow
} // namespace coroutines
} // namespace kotlinx
