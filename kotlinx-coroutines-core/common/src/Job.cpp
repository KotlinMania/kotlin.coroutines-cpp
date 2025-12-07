/**
 * @file Job.cpp
 * @brief Implementation of Job.
 *
 * NOTE: The detailed API documentation, KDocs, and class definitions are located
 * in the companion header file: `include/kotlinx/coroutines/Job.hpp`.
 */

#include "kotlinx/coroutines/Job.hpp"

namespace kotlinx {
namespace coroutines {

// Define the static key instance
CoroutineContext::KeyTyped<Job> Job::key_instance;

} // namespace coroutines
} // namespace kotlinx
