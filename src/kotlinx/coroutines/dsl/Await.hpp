#pragma once

#include "kotlinx/coroutines/Deferred.hpp"
#include "kotlinx/coroutines/dsl/Suspend.hpp"
#include <memory>

namespace kotlinx {
namespace coroutines {

// Extension-like syntax for C++ ?
// In C++, we often use free functions for this or members.
// If the goal is to support "await(deferred)", here it is.

namespace dsl {

    // Implementation adapter
    template <typename T>
    [[suspend]]
    void* await(Deferred<T>& deferred, std::shared_ptr<Continuation<void*>> continuation) {
        return suspend(deferred.await(continuation.get()));
    }

    // Frontend Stub
    // The plugin transforms `await(deferred)` into `await(deferred, cont)`
    template <typename T>
    [[suspend]]
    T await(Deferred<T>& deferred) {
        // This body is replaced by the plugin.
        // In a non-plugin build, this would just fail or return T() default?
        // Since we are validating structure, we leave it empty/dummy or throwing.
        // But for return type T, we must return something to compile.
        throw std::runtime_error("await stub called without plugin transformation");
    }

} // namespace dsl

using dsl::await; // Make it available in coroutines namespace? Or user uses calls dsl::await

} // namespace coroutines
} // namespace kotlinx
