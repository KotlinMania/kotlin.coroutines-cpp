#pragma once
#include "kotlinx/coroutines/flow/Flow.hpp"
#include <vector>

namespace kotlinx {
namespace coroutines {
namespace flow {

/**
 * A hot flow that shares emitted values among all its collectors in a broadcast fashion,
 * so that all collectors get all emitted values.
 */
template<typename T>
struct SharedFlow : public Flow<T> {
    virtual ~SharedFlow() = default;

    /**
     * A snapshot of the replay cache.
     */
    virtual std::vector<T> replay_cache() const = 0;
};

} // namespace flow
} // namespace coroutines
} // namespace kotlinx
