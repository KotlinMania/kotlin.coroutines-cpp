#pragma once
#include "kotlinx/coroutines/core_fwd.hpp"
#include <vector>
#include <unordered_set>
#include <limits>
#include <exception>

namespace kotlinx {
namespace coroutines {
namespace flow {

// Forward declarations
template<typename T> struct SharedFlow;
template<typename T> struct StateFlow;

// Only declarations of lint checks that are "valid" C++ or relevant
// Most of the lints in Kotlin were about preventing misuse of SharedFlow with flow operators that don't make sense (like conflate on StateFlow).
// In C++, we might just not provide those overloads, or mark them deleted.

template<typename T>
Flow<T> cancellable(SharedFlow<T> flow) = delete;

template<typename T>
Flow<T> flow_on(SharedFlow<T> flow, CoroutineContext context) = delete;

template<typename T>
Flow<T> conflate(StateFlow<T> flow) = delete;

template<typename T>
Flow<T> distinct_until_changed(StateFlow<T> flow) = delete;

} // namespace flow
} // namespace coroutines
} // namespace kotlinx
