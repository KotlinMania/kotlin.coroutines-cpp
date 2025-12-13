#pragma once
#include "kotlinx/coroutines/flow/FlowCollector.hpp"

namespace kotlinx {
namespace coroutines {
namespace flow {
namespace internal {

template <typename T>
struct NopCollector : public FlowCollector<T> {
    void* emit(T /*value*/, Continuation<void*>* /*continuation*/) override { return nullptr; }
};

} // namespace internal
} // namespace flow
} // namespace coroutines
} // namespace kotlinx
