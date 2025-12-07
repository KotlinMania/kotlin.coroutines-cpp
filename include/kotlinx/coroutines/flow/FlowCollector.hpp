#pragma once
#include "kotlinx/coroutines/core_fwd.hpp"
#include <memory>

namespace kotlinx {
namespace coroutines {
namespace flow {

template <typename T>
struct FlowCollector {
    virtual ~FlowCollector() = default;
    virtual void emit(T value) = 0; // suspend function in Kotlin, might need strict continuation passing in C++ or coroutine magic
};

} // namespace flow
} // namespace coroutines
} // namespace kotlinx
