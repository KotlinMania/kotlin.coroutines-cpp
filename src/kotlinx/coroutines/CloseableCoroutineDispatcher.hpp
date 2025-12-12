#pragma once
#include "kotlinx/coroutines/CoroutineDispatcher.hpp"

namespace kotlinx {
namespace coroutines {

/**
 * [CoroutineDispatcher] that provides a method to close it,
 * causing the rejection of any new tasks and cleanup of all underlying resources
 * associated with the current dispatcher.
 */
class CloseableCoroutineDispatcher : public CoroutineDispatcher {
public:
    virtual ~CloseableCoroutineDispatcher() = default;

    /**
     * Initiate the closing sequence of the coroutine dispatcher.
     */
    virtual void close() = 0;
};

} // namespace coroutines
} // namespace kotlinx
