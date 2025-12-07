/**
 * @file MultithreadedDispatchers.cpp
 * @brief Implementation of MultithreadedDispatchers.
 *
 * NOTE: The detailed API documentation, KDocs, and class definitions are located
 * in the companion header file: `include/kotlinx/coroutines/MultithreadedDispatchers.hpp`.
 */

#include "kotlinx/coroutines/MultithreadedDispatchers.hpp"

namespace kotlinx {
namespace coroutines {

CloseableCoroutineDispatcher* new_fixed_thread_pool_context(int n_threads, const std::string& name) {
    // Stub implementation to clean file, real impl would need thread pool logic
    return nullptr;
}

} // namespace coroutines
} // namespace kotlinx
