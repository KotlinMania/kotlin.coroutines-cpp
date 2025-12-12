#pragma once
/**
 * @file Lint.hpp
 * @brief Flow lint checks - deleted overloads to prevent misuse
 *
 * Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Lint.kt
 *
 * These lint checks prevent calling certain operators on SharedFlow or StateFlow
 * that don't make sense for those types.
 */

#include "kotlinx/coroutines/flow/Flow.hpp"
#include "kotlinx/coroutines/CoroutineContext.hpp"
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
std::shared_ptr<Flow<T>> cancellable(std::shared_ptr<SharedFlow<T>> flow) = delete;

template<typename T>
std::shared_ptr<Flow<T>> flow_on(std::shared_ptr<SharedFlow<T>> flow, const CoroutineContext& context) = delete;

template<typename T>
std::shared_ptr<Flow<T>> conflate(std::shared_ptr<StateFlow<T>> flow) = delete;

template<typename T>
std::shared_ptr<Flow<T>> distinct_until_changed(std::shared_ptr<StateFlow<T>> flow) = delete;

} // namespace flow
} // namespace coroutines
} // namespace kotlinx
