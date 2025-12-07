#pragma once
#include "kotlinx/coroutines/flow/FlowCollector.hpp"

namespace kotlinx {
namespace coroutines {
namespace flow {
namespace internal {

template <typename T>
struct NopCollector : public FlowCollector<T> {
    void emit(T value) override {
        // does nothing
    }
};

} // namespace internal
} // namespace flow
} // namespace coroutines
} // namespace kotlinx
