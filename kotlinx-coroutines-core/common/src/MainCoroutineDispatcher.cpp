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

std::shared_ptr<CoroutineDispatcher> MainCoroutineDispatcher::limited_parallelism(int parallelism, const std::string& name) {
     throw std::runtime_error("limited_parallelism not implemented");
}

std::string MainCoroutineDispatcher::to_string_internal_impl() const {
    return "MainCoroutineDispatcher";
}

} // namespace coroutines
} // namespace kotlinx
