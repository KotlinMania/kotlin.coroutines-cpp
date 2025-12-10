#pragma once
#include "kotlinx/coroutines/flow/SharedFlow.hpp"

namespace kotlinx {
namespace coroutines {
namespace flow {

/**
 * A SharedFlow that represents a read-only state with a single up-to-date value.
 */
template<typename T>
struct StateFlow : public SharedFlow<T> {
    virtual ~StateFlow() = default;

    /**
     * The current value of this state flow.
     */
    virtual T value() const = 0;
};

} // namespace flow
} // namespace coroutines
} // namespace kotlinx
