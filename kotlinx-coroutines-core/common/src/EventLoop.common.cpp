/**
 * @file EventLoop.common.cpp
 * @brief Implementation of EventLoop.
 *
 * NOTE: The detailed API documentation, KDocs, and class definitions are located
 * in the companion header file: `include/kotlinx/coroutines/EventLoop.hpp`.
 */

#include "kotlinx/coroutines/EventLoop.hpp"

namespace kotlinx {
namespace coroutines {

// Static member definition
EventLoop* ThreadLocalEventLoop::event_loop = nullptr;

} // namespace coroutines
} // namespace kotlinx