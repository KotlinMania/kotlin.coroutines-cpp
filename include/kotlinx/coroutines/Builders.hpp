#pragma once
#include "kotlinx/coroutines/CoroutineScope.hpp"
#include "kotlinx/coroutines/CoroutineContext.hpp"
#include "kotlinx/coroutines/Job.hpp"
#include "kotlinx/coroutines/Deferred.hpp"
#include "kotlinx/coroutines/AbstractCoroutine.hpp"
#include <functional>

namespace kotlinx {
namespace coroutines {

enum class CoroutineStart {
    DEFAULT,
    LAZY,
    ATOMIC,
    UNDISPATCHED
};

// Forward decls
class StandaloneCoroutine;
template <typename T> class DeferredCoroutine;

/**
 * Launcher for a new coroutine.
 */
Job* launch(
    CoroutineScope* scope,
    CoroutineContext context = CoroutineContext(), // Empty
    CoroutineStart start = CoroutineStart::DEFAULT,
    std::function<void(CoroutineScope*)> block = nullptr
);

/**
 * Async builder.
 */
template<typename T>
Deferred<T>* async(
    CoroutineScope* scope,
    CoroutineContext context = CoroutineContext(),
    CoroutineStart start = CoroutineStart::DEFAULT,
    std::function<T(CoroutineScope*)> block = nullptr
) {
    // Stub implementation for now - refactored from Builders.common.cpp
    // Real implementation would instantiate DeferredCoroutine<T>
    return nullptr; 
}

template<typename T>
T with_context(
    CoroutineContext context,
    std::function<T(CoroutineScope*)> block
) {
    // Stub
    return T();
}

template<typename T>
T run_blocking(
    CoroutineContext context,
    std::function<T(CoroutineScope*)> block
) {
    // Stub implementation
    return T();
}

} // namespace coroutines
} // namespace kotlinx
