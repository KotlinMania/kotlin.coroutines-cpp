/**
 * @file Semaphore.cpp
 * @brief Implementation of Semaphore.
 */

#include "kotlinx/coroutines/sync/Semaphore.hpp"
#include "kotlinx/coroutines/sync/SemaphoreImpl.hpp" // MOVED HERE: Include at global scope

#include <stdexcept>
#include <iostream>

// Removed using namespace std::chrono to avoid global pollution if any

namespace kotlinx {
namespace coroutines {
namespace sync {

Semaphore* create_semaphore(int permits, int acquired_permits) {
    return new SemaphoreImpl(permits, acquired_permits);
}

} // namespace sync
} // namespace coroutines
} // namespace kotlinx
