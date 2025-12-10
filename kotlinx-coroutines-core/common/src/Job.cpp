/**
 * @file Job.cpp
 * @brief Implementation of Job.
 *
 * NOTE: The detailed API documentation, KDocs, and class definitions are located
 * in the companion header file: `include/kotlinx/coroutines/Job.hpp`.
 */

#include <kotlinx/coroutines/Job.hpp>
#include "kotlinx/coroutines/JobImpl.hpp"
#include <stdexcept>

namespace kotlinx {
namespace coroutines {

// Define the static key instance
// CoroutineContext::KeyTyped<Job> Job::key_instance; // Defined inline in header

std::shared_ptr<struct Job> Job(std::shared_ptr<struct Job> parent) {
    return JobImpl::create(parent);
}

void Job::cancel_and_join() {
    cancel();
    join();
}

void Job::ensure_active() {
     if (!is_active()) {
          std::exception_ptr ex = get_cancellation_exception();
          if (ex) std::rethrow_exception(ex);
          throw std::runtime_error("Job cancelled but no exception cause");
     }
}

} // namespace coroutines
} // namespace kotlinx
