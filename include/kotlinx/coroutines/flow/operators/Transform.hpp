#pragma once
#include "kotlinx/coroutines/core_fwd.hpp"
#include <functional>
#include <vector>

namespace kotlinx {
namespace coroutines {
namespace flow {

// Forward declarations for transform operators
// C++ usually handles these via pipeline operators or function composition.
// For now, we declare template functions that return Flow<T>*.
// Since they are templates, they must be in header.

template <typename T>
Flow<T>* filter(Flow<T>* flow, std::function<bool(T)> predicate) {
    return nullptr; // Stub
}

template <typename T, typename R>
Flow<R>* map(Flow<T>* flow, std::function<R(T)> transform) {
    return nullptr; // Stub
}

// ... other operators ...

} // namespace flow
} // namespace coroutines
} // namespace kotlinx
