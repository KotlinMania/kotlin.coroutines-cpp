#pragma once

#include "kotlinx/coroutines/Deferred.hpp"
#include <memory>

namespace kotlinx {
namespace coroutines {
namespace dsl {

    /**
     * await - DSL wrapper for awaiting a Deferred result.
     *
     * Usage within coroutine_yield:
     *   coroutine_yield(this, await(deferred, completion));
     *
     * @param deferred The Deferred to await
     * @param continuation The completion continuation
     * @return COROUTINE_SUSPENDED or result
     */
    template <typename T>
    void* await(Deferred<T>& deferred, std::shared_ptr<Continuation<void*>> continuation) {
        return deferred.await(continuation.get());
    }

} // namespace dsl

using dsl::await;

} // namespace coroutines
} // namespace kotlinx
