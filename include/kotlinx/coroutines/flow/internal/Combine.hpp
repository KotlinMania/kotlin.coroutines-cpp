#pragma once
#include "kotlinx/coroutines/core_fwd.hpp"
#include <functional>

namespace kotlinx {
namespace coroutines {
namespace flow {
namespace internal {

// Stub declarations for combine and zip internals
// In C++, variadic templates or fixed overloads are usually used.

template <typename R, typename T>
void combine_internal(
    // Stub
) {}

template <typename T1, typename T2, typename R>
Flow<R>* zip_impl(Flow<T1>* flow, Flow<T2>* flow2, std::function<R(T1, T2)> transform) {
    return nullptr;
}

} // namespace internal
} // namespace flow
} // namespace coroutines
} // namespace kotlinx
