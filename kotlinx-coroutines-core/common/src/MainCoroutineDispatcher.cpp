/**
 * @file MainCoroutineDispatcher.cpp
 * @brief Implementation of MainCoroutineDispatcher.
 *
 * NOTE: The detailed API documentation, KDocs, and class definitions are located
 * in the companion header file: `include/kotlinx/coroutines/MainCoroutineDispatcher.hpp`.
 */

#include "kotlinx/coroutines/MainCoroutineDispatcher.hpp"

namespace kotlinx {
namespace coroutines {

std::string MainCoroutineDispatcher::to_string() const {
    return to_string_internal_impl();
}

CoroutineDispatcher& MainCoroutineDispatcher::limited_parallelism(int parallelism, const std::string* name) {
    // Stub implementation
    return *this; // Should imply limitations on the main thread which is already single threaded usually
}

std::string MainCoroutineDispatcher::to_string_internal_impl() const {
    return "MainCoroutineDispatcher";
}

} // namespace coroutines
} // namespace kotlinx
